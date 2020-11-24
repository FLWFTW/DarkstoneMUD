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
 *		        Main structure manipulation module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "connection.h"
#include "connection_manager.h"
#include "World.h"

#include <list>
using namespace std;

// Forward declarations
void do_revert(Character * ch, const char* argument);
void do_return(Character * ch, const char* argument);
void do_say(Character * ch, const char* argument);

extern int		top_exit;
extern int		top_ed;
extern int		top_affect;
extern int		cur_qobjs;
extern int		cur_qchars;
extern CHAR_DATA *	gch_prev;
extern OBJ_DATA  *	gobj_prev;

CHAR_DATA	*cur_char;
ROOM_INDEX_DATA	*cur_room;
bool		 cur_char_died;
ch_ret		 global_retcode;

int		 cur_obj;
int		 cur_obj_serial;
bool		 cur_obj_extracted;
obj_ret		 global_objcode;

OBJ_DATA *group_object( OBJ_DATA *obj1, OBJ_DATA *obj2 );

extern char * const dir_name_ern [];

/*
 * Return how much exp a char has
 */
int get_exp( CHAR_DATA *ch )
{
    return ch->exp;
}

/* KSILYAN
 *	small function for experience information
 */

int get_exp_worth_for_level( CHAR_DATA * ch, int level)
{
	int exp;
	exp    = (get_exp_worth( ch )
		*  URANGE( 0, (ch->level - level) + 10, 13 )) / 10;
	
	return exp;
}
/*
 * Calculate roughly how much experience a character is worth
 */


int get_exp_worth( CHAR_DATA *ch )
{
    int exp;

    exp = ch->level * ch->level * ch->level * 5;
    exp += ch->max_hit;
    exp -= (ch->armor-50) * 2;
    exp += ( ch->barenumdie * ch->baresizedie + ch->getDamRoll() ) * 50;
    exp += ch->getHitRoll() * ch->level * 10;
    if ( IS_AFFECTED(ch, AFF_SANCTUARY) )
      exp += (int) (exp * 1.5);
    if ( IS_AFFECTED(ch, AFF_FIRESHIELD) )
      exp += (int) (exp * 1.2);
    if ( IS_AFFECTED(ch, AFF_SHOCKSHIELD) )
      exp += (int) (exp * 1.2);
	if ( IS_AFFECTED(ch, AFF_ICESHIELD) )
	  exp += (int) (exp * 1.2);
    exp = URANGE( MIN_EXP_WORTH, exp, MAX_EXP_WORTH );

    return exp;
}

sh_int get_exp_base( CHAR_DATA *ch )
{
    if ( IS_NPC(ch) )
      return 1000;
    return class_table[ch->Class]->exp_base;
}

/*								-Thoric
 * Return how much experience is required for ch to get to a certain level
 */
int exp_level( CHAR_DATA *ch, sh_int level )
{
   int lvl;

   lvl = UMAX(0, level - 1);
   return ( lvl * lvl * lvl * get_exp_base(ch) );
}

/*
 * Get what level ch is based on exp
 */
sh_int level_exp( CHAR_DATA *ch, int exp )
{
    int x, lastx, y, tmp;

    x = LEVEL_LOTD;
    lastx = x;
    y = 0;
    while ( !y )
    {
	tmp = exp_level(ch, x);
	lastx = x;
	if ( tmp > exp )
	  x /= 2;
	else
	if (lastx != x )
	  x += ( x / 2 );
	else
	  y = x;
    }
    if ( y < 1 )
      y = 1;
    if ( y > LEVEL_LOTD )
      y = LEVEL_LOTD;
    return y;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
sh_int get_trust( CHAR_DATA *ch )
{
	if ( !ch )
		return 0;
	if ( !ch->GetConnection() )
		return ch->level;

	ch = ch->GetConnection()->GetOriginalCharacter();

    if ( ch->trust != 0 )
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO_MAX )
	return LEVEL_HERO_MAX;

    if ( ch->level >= LEVEL_WANDERER && IS_RETIRED( ch ) )
      return LEVEL_WANDERER;

    return ch->level;
}


/*
 * Retrieve a character's age.
 */
sh_int get_age( CHAR_DATA *ch )
{
	return 17 + ( ch->played + (secCurrentTime - ch->secLogonTime) ) / 7200;
}


/*
 * Retrieve a character's carry capacity.
 * Vastly reduced (finally) due to containers		-Thoric
 */
int can_carry_n( CHAR_DATA *ch )
{
    int penalty = 0;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return get_trust(ch)*200;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    if ( get_eq_char(ch, WEAR_WIELD) )
      ++penalty;
    if ( get_eq_char(ch, WEAR_DUAL_WIELD) )
      ++penalty;
    if ( get_eq_char(ch, WEAR_HOLD) )
      ++penalty;
    if ( get_eq_char(ch, WEAR_SHIELD) )
      ++penalty;
    return URANGE(5, (ch->level+15)/5 + ch->getDex()-13 - penalty, 20);
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return str_app[ch->getStr()].carry;
}


/*
 * See if a player/mob can take a piece of prototype eq		-Thoric
 */
bool can_take_proto( CHAR_DATA *ch )
{
  if ( IS_IMMORTAL(ch) )
    return TRUE;
  else
  if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PROTOTYPE) )
    return TRUE;
  else
    return FALSE;
}


char* remove_name(const char* str, char* namelist )
{
    char newstr[MAX_STRING_LENGTH] = "\0";
    char word[MAX_STRING_LENGTH];

    while(*namelist != '\0') {
        namelist = one_argument(namelist, word);

        if ( str_cmp(word, str) ) {
            strcat(newstr, word);
            strcat(newstr, " ");
        }
    }

    return str_dup(newstr);
}

/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *str, const char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	if ( !namelist )
	{
		gTheWorld->LogBugString("Is_name:: NULL namelist!\n\r");
		return false;
	}

	for ( ; ; )
	{
		namelist = one_argument( namelist, name );
		if ( name[0] == '\0' )
			return FALSE;
		if ( !str_cmp( str, name ) )
			return TRUE;
	}
}

bool is_name_prefix( const char *str, const char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_prefix( str, name ) )
	    return TRUE;
    }
}

const char *strip_the( const char * name)
{
	const char * stripit[] =
	{ 
		"the", 
		"beheaded", "corpse", "of", "animated", "one", "drained"
	};

	const char *bufptr;
	const char *oldptr;
	char test[MAX_STRING_LENGTH];
	int loop;
	bool argok;

	/* loop through each argument of name and check against loop
	   if it fails, then return..... */
	argok = TRUE;
	oldptr = name;

	bufptr = one_argument( name, test);
	if ( test[0] == '\0' )
		return NULL; /* NULL string*/

	for ( loop = 0; loop < 7; loop++ )
	{
		if ( !strcmp (test, stripit[loop]) )
			argok = FALSE;
	}

	if (argok)
		return name; /* name didn't have ANY of the above prefix words */

	while (!argok)
	{
		argok = TRUE;
		oldptr = bufptr;
		bufptr = one_argument( bufptr, test);

		if ( test[0] == '\0' )
			return name; /* went through the name and it was all prefix words*/

		for ( loop = 0; loop < 6; loop++ )
		{
			if ( !strcmp (test, stripit[loop]) )
				argok = FALSE;
		}

	}

	return oldptr; /* test wasn't a prefix so return the pre-buf state*/
}
 

/* same as strip_the, but adds a and an to the list */
const char *strip_a( const char *name)
{
	const char * stripit[] =
	{ 
		"the", 
		"beheaded", "corpse", "of", "animated", "one", "a", "an",
		"drained"

	};

	const char *bufptr;
	const char *oldptr;
	char test[MAX_STRING_LENGTH];
	int loop;
	bool argok;


	/* loop through each argument of name and check against loop
	   if it fails, then return..... */

	argok = TRUE;
	oldptr = name;


	bufptr = one_argument( name, test);

	if ( test[0] == '\0' )
		return NULL; /* NULL string*/

	for ( loop = 0; loop < 9; loop++ )
	{
		if ( !strcmp (test, stripit[loop]) )
			argok = FALSE;
	}

	if (argok)
		return name; /* name didn't have ANY of the above prefix words */

	while (!argok)
	{
		argok = TRUE;
		oldptr = bufptr;
		bufptr = one_argument( bufptr, test);

		if ( test[0] == '\0' )
			return name; /* went through the name and it was all prefix words*/

		for ( loop = 0; loop < 8; loop++ )
		{
			if ( !strcmp (test, stripit[loop]) )
				argok = FALSE;
		}

	}

	return oldptr; /* test wasn't a prefix so return the pre-buf state*/

}




/*
 * See if a string is one of the names of an object.		-Thoric
 * Treats a dash as a word delimiter as well as a space
 */
bool is_name2( const char *str, const char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	for ( ; ; )
	{
		namelist = one_argument2( namelist, name );
		if ( name[0] == '\0' )
			return FALSE;
		if ( !str_cmp( str, name ) )
			return TRUE;
	}
}

bool is_name2_prefix( const char *str, const char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	for ( ; ; )
	{
		namelist = one_argument2( namelist, name );
		if ( name[0] == '\0' )
			return FALSE;
		if ( !str_prefix( str, name ) )
			return TRUE;
	}
}

/*								-Thoric
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name( const char *str, const char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	if ( !str || str[0] == '\0' )
		return FALSE;

	for ( ; ; )
	{
		str = one_argument2( str, name );
		if ( name[0] == '\0' )
			return TRUE;
		if ( !is_name2( name, namelist ) )
			return FALSE;
	}
}

bool nifty_is_name_prefix( const char *str, const char *namelist )
{
    char name[MAX_INPUT_LENGTH];
    
    if ( !str || str[0] == '\0' )
      return FALSE;
 
    for ( ; ; )
    {
	str = one_argument2( str, name );
	if ( name[0] == '\0' )
	    return TRUE;
	if ( !is_name2_prefix( name, namelist ) )
	    return FALSE;
    }
}

void AddSpellMemory(CHAR_DATA * caster, CHAR_DATA * target, AFFECT_DATA * effect)
{
    if (!caster || !target || !effect)
        return;

    SpellMemory * spell = new SpellMemory(caster, target, effect);
    caster->spellMemory_.push_back(spell);
    target->spellMemory_.push_back(spell);
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd, CHAR_DATA * caster )
{
	OBJ_DATA *wield;
	int mod;
	SkillType *skill;
	ch_ret retcode;

	/*    if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NOAFFECTS && fAdd ) ) {
		  return;
		  }
	 */  
	mod = paf->modifier;

	if ( fAdd )
	{
		SET_BIT( ch->affected_by, paf->bitvector );
		// If there was a caster, add the spell memory.
		// But, don't want to add it if the target is an NPC.
		if (caster && caster != ch && !IS_NPC(ch) )
			AddSpellMemory(caster, ch, paf);
	}
	else
	{
		REMOVE_BIT( ch->affected_by, paf->bitvector );
		/*
		 * might be an idea to have a duration removespell which returns
		 * the spell after the duration... but would have to store
		 * the removed spell's information somewhere...		-Thoric
		 */
		if ( (paf->location % REVERSE_APPLY) == APPLY_REMOVESPELL )
			return;
		switch( paf->location % REVERSE_APPLY )
		{
			case APPLY_AFFECT:        REMOVE_BIT( ch->affected_by, mod );	return;
			case APPLY_RESISTANT:     REMOVE_BIT( ch->resistant, mod );	return;
			case APPLY_IMMUNE:        REMOVE_BIT( ch->immune, mod );	return;
			case APPLY_SUSCEPTIBLE:   REMOVE_BIT( ch->susceptible, mod );	return;
									  //case APPLY_WEARSPELL:	    /* affect only on wear */		return;
			case APPLY_REMOVE:	    SET_BIT( ch->affected_by, mod );	return;
		}
		mod = 0 - mod;
	}

	switch ( paf->location % REVERSE_APPLY )
	{
		default:
			bug( "Affect_modify: unknown location %d.", paf->location );
			return;

		case APPLY_NONE:						break;
		case APPLY_STR:           ch->mod_str		+= mod;	break;
		case APPLY_DEX:           ch->mod_dex		+= mod;	break;
		case APPLY_INT:           ch->mod_int		+= mod;	break;
		case APPLY_WIS:           ch->mod_wis		+= mod;	break;
		case APPLY_CON:	      ch->mod_con		+= mod;	break;
		case APPLY_CHA:	      ch->mod_cha		+= mod; break;
		case APPLY_LCK:	      ch->mod_lck		+= mod; break;
		case APPLY_SEX:
							  ch->sex = (ch->sex+mod) % 3;
							  if ( ch->sex < 0 )
								  ch->sex += 2;
							  ch->sex = URANGE( 0, ch->sex, 2 );
							  break;
		case APPLY_CLASS:						break;
		case APPLY_LEVEL:						break;
		case APPLY_AGE:						break;
		case APPLY_HEIGHT:	      ch->height		+= mod;	break;
		case APPLY_WEIGHT:	      ch->weight		+= mod;	break;
		case APPLY_MANA:
								  ch->max_mana += mod;
								  ch->mana += mod;
								  // Ksilyan:
								  if (ch->mana > ch->max_mana)
									  ch->mana = ch->max_mana;
								  break;
		case APPLY_HIT:
								  ch->max_hit += mod;
								  ch->hit += mod;
								  // Ksilyan:
								  if ( ch->hit > ch->max_hit )
									  ch->hit = ch->max_hit;
								  if ( ch->hit < 0 )
									  ch->hit = 0;
								  break;
		case APPLY_MOVE:
								  ch->max_move += mod;
								  ch->move += mod;
								  if (ch->move > ch->max_move)
									  ch->move = ch->max_move;
								  break;
		case APPLY_GOLD:						break;
		case APPLY_EXP:						break;
		case APPLY_AC:            ch->armor			+= mod;	break;
		case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
		case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
		case APPLY_SAVING_POISON: ch->saving_poison_death	+= mod;	break;
		case APPLY_SAVING_ROD:    ch->saving_wand		+= mod;	break;
		case APPLY_SAVING_PARA:   ch->saving_para_petri	+= mod;	break;
		case APPLY_SAVING_BREATH: ch->saving_breath		+= mod;	break;
		case APPLY_SAVING_SPELL:  ch->saving_spell_staff	+= mod;	break;
		case APPLY_AFFECT:        SET_BIT( ch->affected_by, mod );	break;
		case APPLY_RESISTANT:     SET_BIT( ch->resistant, mod );	break;
		case APPLY_IMMUNE:        SET_BIT( ch->immune, mod );	break;
		case APPLY_SUSCEPTIBLE:   SET_BIT( ch->susceptible, mod );	break;
		case APPLY_WEAPONSPELL:	/* see fight.c */		break;
		case APPLY_REMOVE:	      REMOVE_BIT(ch->affected_by, mod);	break;

		case APPLY_FULL:
								  if ( !IS_NPC(ch) )
									  ch->pcdata->condition[COND_FULL] = URANGE( 0, ch->pcdata->condition[COND_FULL] + mod, 48 );
								  break;

		case APPLY_THIRST:
								  if ( !IS_NPC(ch) )
									  ch->pcdata->condition[COND_THIRST] = URANGE( 0, ch->pcdata->condition[COND_THIRST] + mod, 48 );
								  break;

		case APPLY_DRUNK:
								  if ( !IS_NPC(ch) )
									  ch->pcdata->condition[COND_DRUNK] = URANGE( 0, ch->pcdata->condition[COND_DRUNK] + mod, 48 );
								  break;

		case APPLY_BLOOD:
								  if ( !IS_NPC(ch) )
									  ch->pcdata->condition[COND_BLOODTHIRST] = URANGE( 0, ch->pcdata->condition[COND_BLOODTHIRST] + mod, ch->level+10+mod );
								  break;

		case APPLY_MENTALSTATE:
								  ch->mental_state	= URANGE(-100, ch->mental_state + mod, 100);
								  break;
		case APPLY_EMOTION:
								  ch->emotional_state	= URANGE(-100, ch->emotional_state + mod, 100);
								  break;

		case APPLY_STRIPSN:
								  if ( IS_VALID_SN(mod) )
									  affect_strip( ch, mod );
								  else
									  bug( "affect_modify: APPLY_STRIPSN invalid sn %d", mod );
								  break;

		case APPLY_CONTAGION:
								  if (mod>0)
								  {
									  if ( IS_VALID_SN(mod) )
									  {
										  /* nothing to do except let nature take its course */
									  }
									  else
										  bug( "affect_modify: APPLY_CONTAGION invalid sn %d", mod );
								  }
								  /* else wearoff is -sn */

								  break;

								  /* spell cast upon wear/removal of an object	-Thoric */
		case APPLY_WEARSPELL:
		case APPLY_REMOVESPELL:
								  if (
										  saving_char == ch		/* so save/quit doesn't trigger */
										  ||   loading_char == ch )	/* so loading doesn't trigger */
									  return;

								  // Affects can disappear even in a no-magic room, so we need
								  // to put that ifcheck AFTER this.
								  if (mod < 0)
								  {
									  /* KSILYAN:
									   * The only way to get a negative mod is
									   * if we're removing a wearspell affect.
									   * So, this _should_ always be ok.
									   */
									  affect_strip_one(ch, -mod);
									  return;
								  }

								  if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NO_MAGIC) || IS_SET(ch->immune, RIS_MAGIC) )
									  return;
								  //mod = abs(mod);

								  if ( IS_VALID_SN(mod)
										  &&  (skill=skill_table[mod]) != NULL
										  &&   skill->type == SKILL_SPELL )
								  {
									  target_name = ch->getName().c_str();
									  if ( (retcode=(*skill->spell_fun) ( mod, ch->level, ch, ch )) == rCHAR_DIED
											  ||   char_died(ch) )
									  {
										  return;
									  }
									  else if (retcode == rSPELL_ALREADY_AFFECTED)
									  {
										  /* Ksilyan:
										   * Well, in this case, we want to add a fake affect,
										   * in case we have two items that affect the same thing.
										   * We only want to add it to the list, not actually
										   * process it.
										   */
										  AFFECT_DATA * paf;
										  AFFECT_DATA * paf_new;
										  for (paf = ch->first_affect; paf; paf = paf->next)
										  {
											  if (paf->type == mod)
												  break;
										  }
										  if (paf == NULL)
										  {
											  bug("affect_modify: Spell already affected, but couldn't find affect!!", 0);
											  return;
										  }
										  CREATE( paf_new, AFFECT_DATA, 1 );
										  paf_new->next = ch->first_affect;
										  ch->first_affect->prev = paf_new;
										  ch->first_affect = paf_new;
										  paf_new->type	= mod;
										  paf_new->duration	= paf->duration;
									  }
								  }
								  break;


								  /* skill apply types	-Thoric */

		case APPLY_PALM:	/* not implemented yet */
								  break;

		case APPLY_TRACK:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_track] > 0  )
									  ch->pcdata->learned[gsn_track] =
										  URANGE( 0, ch->pcdata->learned[gsn_track] + mod, 101 );
								  break;
		case APPLY_HIDE:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_hide] > 0  )
									  ch->pcdata->learned[gsn_hide] = 
										  URANGE( 0, ch->pcdata->learned[gsn_hide] + mod, 101 );
								  break;
		case APPLY_STEAL:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_steal] > 0 )
									  ch->pcdata->learned[gsn_steal] =
										  URANGE( 0, ch->pcdata->learned[gsn_steal] + mod, 101 );
								  break;
		case APPLY_SNEAK:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_steal] > 0 )
									  ch->pcdata->learned[gsn_sneak] =
										  URANGE( 0, ch->pcdata->learned[gsn_sneak] + mod, 101 );
								  break;
		case APPLY_PICK:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_pick_lock] > 0 )
									  ch->pcdata->learned[gsn_pick_lock] =
										  URANGE( 0, ch->pcdata->learned[gsn_pick_lock] + mod, 101 );
								  break;
		case APPLY_BACKSTAB:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_backstab] > 0 )
									  ch->pcdata->learned[gsn_backstab] =
										  URANGE( 0, ch->pcdata->learned[gsn_backstab] + mod, 101 );
								  break;
		case APPLY_DETRAP:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_detrap] > 0 )
									  ch->pcdata->learned[gsn_detrap] =
										  URANGE( 0, ch->pcdata->learned[gsn_detrap] + mod, 101 );
								  break;
		case APPLY_DODGE:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_dodge] > 0 )
									  ch->pcdata->learned[gsn_dodge] =
										  URANGE( 0, ch->pcdata->learned[gsn_dodge] + mod, 101 );
								  break;
		case APPLY_PEEK:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_peek] > 0 )
									  ch->pcdata->learned[gsn_peek] =
										  URANGE( 0, ch->pcdata->learned[gsn_peek] + mod, 101 );
								  break;
		case APPLY_SCAN:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_scan] > 0 )
									  ch->pcdata->learned[gsn_scan] =
										  URANGE( 0, ch->pcdata->learned[gsn_scan] + mod, 101 );
								  break;
		case APPLY_GOUGE:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_gouge] > 0 )
									  ch->pcdata->learned[gsn_gouge] =
										  URANGE( 0, ch->pcdata->learned[gsn_gouge] + mod, 101 );
								  break;
		case APPLY_SEARCH:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_search] > 0 )
									  ch->pcdata->learned[gsn_search] =
										  URANGE( 0, ch->pcdata->learned[gsn_search] + mod, 101 );
								  break;
		case APPLY_DIG:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_dig] > 0 )
									  ch->pcdata->learned[gsn_dig] =
										  URANGE( 0, ch->pcdata->learned[gsn_dig] + mod, 101 );
								  break;
		case APPLY_MOUNT:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_mount] > 0 )
									  ch->pcdata->learned[gsn_mount] =
										  URANGE( 0, ch->pcdata->learned[gsn_mount] + mod, 101 );
								  break;
		case APPLY_DISARM:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_disarm] > 0 )
									  ch->pcdata->learned[gsn_disarm] =
										  URANGE( 0, ch->pcdata->learned[gsn_disarm] + mod, 101 );
								  break;
		case APPLY_KICK:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_kick] > 0 )
									  ch->pcdata->learned[gsn_kick] =
										  URANGE( 0, ch->pcdata->learned[gsn_kick] + mod, 101 );
								  break;
		case APPLY_PARRY:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_parry] > 0 )
									  ch->pcdata->learned[gsn_parry] =
										  URANGE( 0, ch->pcdata->learned[gsn_parry] + mod, 101 );
								  break;
		case APPLY_BASH:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_bash] > 0 )
									  ch->pcdata->learned[gsn_bash] =
										  URANGE( 0, ch->pcdata->learned[gsn_bash] + mod, 101 );
								  break;
		case APPLY_STUN:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_stun] > 0 )
									  ch->pcdata->learned[gsn_stun] =
										  URANGE( 0, ch->pcdata->learned[gsn_stun] + mod, 101 );
								  break;
		case APPLY_PUNCH:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_punch] > 0 )
									  ch->pcdata->learned[gsn_punch] =
										  URANGE( 0, ch->pcdata->learned[gsn_punch] + mod, 101 );
								  break;
		case APPLY_CLIMB:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_climb] > 0 )
									  ch->pcdata->learned[gsn_climb] =
										  URANGE( 0, ch->pcdata->learned[gsn_climb] + mod, 101 );
								  break;
		case APPLY_GRIP:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_grip] > 0 )
									  ch->pcdata->learned[gsn_grip] =
										  URANGE( 0, ch->pcdata->learned[gsn_grip] + mod, 101 );
								  break;
		case APPLY_SCRIBE:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_scribe] > 0 )
									  ch->pcdata->learned[gsn_scribe] =
										  URANGE( 0, ch->pcdata->learned[gsn_scribe] + mod, 101 );
								  break;
		case APPLY_BREW:
								  if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_brew] > 0 )
									  ch->pcdata->learned[gsn_brew] =
										  URANGE( 0, ch->pcdata->learned[gsn_brew] + mod, 101 );
								  break;
	}

	/*
	 * Check for weapon wielding.
	 * Guard against recursion (for weapons with affects).
	 */
	if ( !IS_NPC( ch )
			&&   saving_char != ch
			&& ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
			&&   get_obj_weight(wield) > str_app[ch->getStr()].wield )
	{
		static int depth;

		if ( depth == 0 )
		{
			depth++;
			act( AT_ACTION, "You are too weak to wield $p any longer.", 
					ch, wield, NULL, TO_CHAR );
			act( AT_ACTION, "$n stops wielding $p.", ch, wield, NULL, TO_ROOM );

			Object * tmpobj;

			if ( wield->wear_loc == WEAR_WIELD )
				tmpobj = get_eq_char(ch, WEAR_DUAL_WIELD);

			unequip_char( ch, wield );

			// if there was a dual-wield, move it to wield
			if ( tmpobj )
				tmpobj->wear_loc = WEAR_WIELD;

			depth--;
		}
	}

	return;
}




/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf, CHAR_DATA * caster )
{
    AFFECT_DATA *paf_new;

    if ( !ch )
    {
      bug( "Affect_to_char: NULL ch!", 0 );
      return;
    }

    if ( !paf )
    {
      bug( "Affect_to_char: NULL paf!", 0 );
      return;
    }

    CREATE( paf_new, AFFECT_DATA, 1 );
    LINK( paf_new, ch->first_affect, ch->last_affect, next, prev );
    paf_new->type	= paf->type;
    paf_new->duration	= paf->duration;
    paf_new->location	= paf->location;
    paf_new->modifier	= paf->modifier;
    paf_new->bitvector	= paf->bitvector;

    affect_modify( ch, paf_new, TRUE, caster );
	
    return;
}


/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	if ( !ch->first_affect )
	{
		bug( "Affect_remove: no affect.", 0 );
		return;
	}
	
	affect_modify( ch, paf, FALSE );
	
	UNLINK( paf, ch->first_affect, ch->last_affect, next, prev );
	DISPOSE( paf );
	return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->first_affect; paf; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}

/*
 * Ksilyan:
 * Strip one affect of a given sn.
 */
void affect_strip_one( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for ( paf = ch->first_affect; paf; paf = paf_next )
	{
		paf_next = paf->next;
		if ( paf->type == sn )
		{
			affect_remove( ch, paf );
			break; // we've found one, so stop!
		}
	}

	return;
}


/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;
	
	for ( paf = ch->first_affect; paf; paf = paf->next )
		if ( paf->type == sn )
			return TRUE;
		
		return FALSE;
}

bool is_contagious(CHAR_DATA *ch)
{
	AFFECT_DATA *paf;
	
	for( paf = ch->first_affect; paf; paf = paf->next )
		if (paf->location == APPLY_CONTAGION)
			return TRUE;
		return FALSE;
}


/*
 * Add or enhance an affect.
 * Limitations put in place by Thoric, they may be high... but at least
 * they're there :)
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;

    for ( paf_old = ch->first_affect; paf_old; paf_old = paf_old->next )
	if ( paf_old->type == paf->type )
	{
	    paf->duration = UMIN( 1000000, paf->duration + paf_old->duration );
	    if ( paf->modifier )
		paf->modifier = UMIN( 5000, paf->modifier + paf_old->modifier );
	    else
	        paf->modifier = paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}

    affect_to_char( ch, paf );
    return;
}


/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
	OBJ_DATA *obj;
	
	if ( !ch->GetInRoom() )
	{
		bug( "Char_from_room: NULL.", 0 );
		return;
	}
	
	if ( !IS_NPC(ch) )
		--ch->GetInRoom()->area->nplayer;
	
	if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
		&&	 obj->item_type == ITEM_LIGHT
		&&	 obj->value[2] != 0
		&&	 ch->GetInRoom()->light > 0 )
		--ch->GetInRoom()->light;

	// this needs to be done before the player is removed from the room
	// because act() needs a in_room for the player.
	if ( ch->GetSittingOn() )
	{
		act(AT_ACTION, "You fall off of $p.", ch, ch->GetSittingOn(), NULL, TO_CHAR);
		ch->SittingOnId = 0;
	}

	// Remove the player's barricade
	if ( ch->BarricadeDir != -1 )
	{
		string buf;
		buf = string("You stop barricading the ") + string( dir_name_ern[ch->BarricadeDir] ) + " exit.";
		act( AT_ACTION, buf.c_str(), ch, NULL, NULL, TO_CHAR );
		buf = "$n stops barricading the " + string( dir_name_ern[ch->BarricadeDir] ) + " exit.";
		act( AT_ACTION, buf.c_str(), ch, NULL, NULL, TO_ROOM );
		ch->BarricadeDir = -1;
	}
	
	UNLINK( ch, ch->GetInRoom()->first_person, ch->GetInRoom()->last_person,
		next_in_room, prev_in_room );
	ch->InRoomId 	 = 0;
	ch->next_in_room = NULL;
	ch->prev_in_room = NULL;
	
	if ( !IS_NPC(ch)
		&&	 get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
		remove_timer( ch, TIMER_SHOVEDRAG );
	
	return;
}


/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
	OBJ_DATA *obj;

	if ( !ch )
	{
		bug( "Char_to_room: NULL ch!", 0 );
		return;
	}
	if ( !pRoomIndex )
	{
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Char_to_room: %s -> NULL room!  Putting char in limbo (%s)",
				ch->getName().c_str(), vnum_to_dotted(ROOM_VNUM_LIMBO) );
		bug( buf, 0 );
		/* This used to just return, but there was a problem with crashing
		   and I saw no reason not to just put the char in limbo. -Narn */
		pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
	}


	ch->InRoomId 	= pRoomIndex->GetId();


	if ( !ch->GetInRoom() )
	{
		gTheWorld->LogBugString( string("Char_to_room: ") + ch->getName().str() +
				string(" in invalid room! Putting char in limbo.") );
		ch->InRoomId = ROOM_VNUM_LIMBO;
		pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
	}
	
	LINK( ch, pRoomIndex->first_person, pRoomIndex->last_person,
			next_in_room, prev_in_room );


	
	if ( !IS_NPC(ch) )
	{
		if ( ++ch->GetInRoom()->area->nplayer > ch->GetInRoom()->area->max_players )
			ch->GetInRoom()->area->max_players = ch->GetInRoom()->area->nplayer;
	}

	if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
			&&	 obj->item_type == ITEM_LIGHT
			&&	 obj->value[2] != 0 )
		++ch->GetInRoom()->light;

	if ( !IS_NPC(ch)
			&&	  IS_SET(ch->GetInRoom()->room_flags, ROOM_SAFE)
			&&	  get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
		add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );	/*-30 Seconds-*/

	/*
	 * Delayed Teleport rooms					-Thoric
	 * Should be the last thing checked in this function
	 */
	if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_TELEPORT )
			&&	 ch->GetInRoom()->tele_delay > 0 )
	{
		TELEPORT_DATA *tele;

		for ( tele = first_teleport; tele; tele = tele->next )
			if ( tele->room == pRoomIndex )
				return;

		CREATE( tele, TELEPORT_DATA, 1 );
		LINK( tele, first_teleport, last_teleport, next, prev );
		tele->room		= pRoomIndex;
		tele->timer 	= pRoomIndex->tele_delay;
	}
	return;
}

/*
 * Give an obj to a char.
 */
OBJ_DATA *obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *otmp;
    OBJ_DATA *oret = obj;
    bool skipgroup, grouped;
    int oweight = get_obj_weight( obj );
    int onum = get_obj_number( obj );
    int wear_loc = obj->wear_loc;
    int extra_flags = obj->extra_flags;
    
    skipgroup = FALSE;
    grouped = FALSE;

    if (IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
    {
	if (!IS_IMMORTAL( ch ) 
	&& (IS_NPC(ch) && !IS_SET(ch->act, ACT_PROTOTYPE)) )
	  return obj_to_room( obj, ch->GetInRoom() );
    }

    if ( loading_char == ch )
    {
	int x,y;
	for ( x = 0; x < MAX_WEAR; x++ )
	  for ( y = 0; y < MAX_LAYERS; y++ )
	    if ( save_equipment[x][y] == obj )
	    {
		skipgroup = TRUE;
		break;
	    }
    }

    if ( !skipgroup )
      for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
	if ( (oret=group_object( otmp, obj )) == otmp )
	{
	    grouped = TRUE;
	    break;
	}

    if ( !grouped )
    {
	LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
	obj->CarriedById = ch->GetId();
	obj->InRoomId = 0;
	obj->InObjId = 0;
    }
    if (wear_loc == WEAR_NONE)
    {
	ch->carry_number	+= onum;
	ch->carry_weight	+= oweight;
    }
    else
    if ( !IS_SET(extra_flags, ITEM_MAGIC) )
	ch->carry_weight	+= oweight;
    return (oret ? oret : obj);
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
	CHAR_DATA *ch;

	if ( ( ch = obj->GetCarriedBy() ) == NULL )
	{
		bug( "Obj_from_char: null ch.", 0 );
		return;
	}

	if ( obj->wear_loc != WEAR_NONE )
		unequip_char( ch, obj );

	/* obj may drop during unequip... */
	if ( !obj->GetCarriedBy() )
		return;

	UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

	if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
		empty_obj( obj, NULL, NULL );
	
	obj->InRoomId = 0;
	obj->CarriedById = 0;
	obj->InObjId = 0;
	ch->carry_number	-= get_obj_number( obj );
	ch->carry_weight	-= get_obj_weight( obj );
	return;
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:		return 3 * obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_HEAD:		return 2 * obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_LEGS:		return 2 * obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_FEET:		return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_HANDS:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_ARMS:		return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_SHIELD:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_FINGER_L:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_FINGER_R: return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_NECK_1:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_NECK_2:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_ABOUT:	return 2 * obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_WAIST:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_WRIST_L:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_WRIST_R:	return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_HOLD:		return     obj->value[OBJECT_ARMOR_AC]; break;
    case WEAR_CAPE:		return	   obj->value[OBJECT_ARMOR_AC]; break;
    }
	
    return 0;
}



/*
 * Find a piece of eq on a character.
 * Will pick the top layer if clothing is layered.		-Thoric
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj, *maxobj = NULL;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {
        if ( obj->wear_loc == iWear )
        {
            if ( !obj->pIndexData->layers ) {
                return obj;
            } else {
                if ( !maxobj || obj->pIndexData->layers > maxobj->pIndexData->layers ) {
                    maxobj = obj;
                }
            }
        }
    }

    return maxobj;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
	AFFECT_DATA *paf;
	OBJ_DATA	*otmp;

	if ( (otmp=get_eq_char( ch, iWear )) != NULL
		&&   (!otmp->pIndexData->layers || !obj->pIndexData->layers) )
	{
		bug( "Equip_char: %s: %s: already equipped (%d).", ch->getName().c_str(), otmp->name_.c_str(), iWear );
		return;
	}

	extern void SanityCheck(Character*ch);
	SanityCheck(ch);

	separate_obj(obj);	/* just in case */
	SanityCheck(ch);
	if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
		||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
		||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	{
		/*
		 * Thanks to Morgenes for the bug fix here!
		 */
		if ( loading_char != ch )
		{
			act( AT_MAGIC, "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
			act( AT_MAGIC, "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
		}

		if ( obj->GetCarriedBy() )
			obj_from_char( obj );

		SanityCheck(ch);

		if ( IS_OBJ_STAT(obj, ITEM_DEITY) )
		{
			act(AT_MAGIC, "As $p hits the floor, it disappears in a puff of smoke.", ch, obj, NULL, TO_CHAR);
			act(AT_MAGIC, "As $p hits the floor, it disappears in a puff of smoke.", ch, obj, NULL, TO_ROOM);
			extract_obj(obj, TRUE);
			SanityCheck(ch);
		}
		else
		{
			obj_to_room( obj, ch->GetInRoom() );
			SanityCheck(ch);
		}

		if ( !obj ) { return; }
    
		oprog_zap_trigger( ch, obj);
		if ( IS_SET(sysdata.save_flags, SV_ZAPDROP) && !char_died(ch) )
			save_char_obj( ch );
		return;
	}

	ch->armor      	-= apply_ac( obj, iWear );
	obj->wear_loc	 = iWear;

	ch->carry_number	-= get_obj_number( obj );
	if ( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
		ch->carry_weight  -= get_obj_weight( obj );

	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		affect_modify( ch, paf, TRUE );
	for ( paf = obj->first_affect; paf; paf = paf->next )
		affect_modify( ch, paf, TRUE );

	if ( obj->item_type == ITEM_LIGHT
		&&   obj->value[2] != 0
		&&   ch->GetInRoom() )
		++ch->GetInRoom()->light;

	return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
	AFFECT_DATA *paf;

	if ( obj->wear_loc == WEAR_NONE )
	{
		bug( "Unequip_char: already unequipped.", 0 );
		return;
	}

	ch->carry_number	+= get_obj_number( obj );
	
	if ( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
		ch->carry_weight  += get_obj_weight( obj );

	ch->armor		+= apply_ac( obj, obj->wear_loc );
	obj->wear_loc	 = -1;

	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		affect_modify( ch, paf, FALSE );
	
	if ( obj->GetCarriedBy() )
		for ( paf = obj->first_affect; paf; paf = paf->next )
			affect_modify( ch, paf, FALSE );

	if ( !obj->GetInObj() )
		return;

	if ( obj->item_type == ITEM_LIGHT
			&&   obj->value[2] != 0
			&&   ch->GetInRoom()
			&&   ch->GetInRoom()->light > 0 )
		--ch->GetInRoom()->light;

	return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *List )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = List; obj; obj = obj->next_content )
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void	write_corpses	 ( CHAR_DATA *ch, const char *name ) ;

int falling;

void obj_from_room( OBJ_DATA *obj )
{
	ROOM_INDEX_DATA *in_room;
	CHAR_DATA       *vch;

	if ( ( in_room = obj->GetInRoom() ) == NULL )
	{
		bug( "obj_from_room: NULL.", 0 );
		return;
	}

	UNLINK( obj, in_room->first_content, in_room->last_content,
			next_content, prev_content );

	if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
		empty_obj( obj, NULL, obj->GetInRoom() );

	if (obj->item_type == ITEM_FIRE)
		obj->GetInRoom()->light--;

	obj->CarriedById = 0;
	obj->InObjId = 0;
	obj->InRoomId = 0;
	if ( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling == 0 )
		write_corpses( NULL, obj->shortDesc_.c_str() +14 );


	for ( vch = in_room->first_person; vch; vch = vch->next_in_room ) {
		if ( vch->GetSittingOn() && vch->GetSittingOn() == obj ) {
			act(AT_ACTION, "$n falls off of $p.", vch, obj, NULL, TO_ROOM);
			act(AT_ACTION, "You fall off of $p.", vch, obj, NULL, TO_CHAR);
			vch->SittingOnId = 0;
		}
	}

	return;
}

/*
 * Move an obj into a room.
 */
OBJ_DATA *obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
	OBJ_DATA *otmp, *oret;
	sh_int item_type = obj->item_type;

	for ( otmp = pRoomIndex->first_content; otmp; otmp = otmp->next_content )
	{
		if ( (oret=group_object( otmp, obj )) == otmp )
		{
			if (item_type == ITEM_FIRE)
				pRoomIndex->light++;
			return oret;
		}
	}

	LINK( obj, pRoomIndex->first_content, pRoomIndex->last_content, next_content, prev_content );

	obj->InRoomId = pRoomIndex->GetId();
	obj->CarriedById = 0;
	obj->InObjId = 0;
	if (item_type == ITEM_FIRE)
		pRoomIndex->light++;

	falling++;
	obj_fall( obj, FALSE );
	falling--;
	
	if ( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling == 0 )
		write_corpses( NULL, obj->shortDesc_.c_str()+14 );

	if ( IS_SET(pRoomIndex->room_flags, ROOM_DONATION) )
		SET_BIT(obj->extra_flags, ITEM_DONATION);

	return obj;
}



/*
 * Move an object into an object.
 */
OBJ_DATA *obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    OBJ_DATA *otmp, *oret;

    if ( obj == obj_to )
    {
	bug( "Obj_to_obj: trying to put object inside itself: vnum %s", vnum_to_dotted(obj->pIndexData->vnum) );
	return obj;
    }
    /* Big carry_weight bug fix here by Thoric */
    /* updated to handle unlimited nesting -- matthew 7/29/0 */
    if ( get_obj_carried_by(obj) != get_obj_carried_by(obj_to) )
    {
        if ( get_obj_carried_by(obj) )
        {
            get_obj_carried_by(obj)->carry_weight -= get_obj_weight( obj );
        }
        if ( get_obj_carried_by(obj_to) )
        {
            get_obj_carried_by(obj_to)->carry_weight += get_obj_weight( obj );
        }
    }

    for ( otmp = obj_to->first_content; otmp; otmp = otmp->next_content )
	if ( (oret=group_object( otmp, obj )) == otmp )
	    return oret;

    LINK( obj, obj_to->first_content, obj_to->last_content,
    	       next_content, prev_content );
    obj->InObjId = obj_to->GetId();
    obj->InRoomId = 0;
    obj->CarriedById = 0;

    return obj;
}


/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->GetInObj() ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    UNLINK( obj, obj_from->first_content, obj_from->last_content,
    		 next_content, prev_content );

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
	empty_obj( obj, obj->GetInObj(), NULL );

    obj->InObjId = 0;
    obj->InRoomId = 0;
    obj->CarriedById = 0;

    for ( ; obj_from; obj_from = obj_from->GetInObj() )
	if ( obj_from->GetCarriedBy() )
	    obj_from->GetCarriedBy()->carry_weight -= get_obj_weight( obj );

    return;
}



/*
 * Extract an obj from the world.
 */

void extract_obj( OBJ_DATA *obj, bool bDecrement )
{
	OBJ_DATA *obj_content;

	if ( obj_extracted(obj) )
	{
		bug( "extract_obj: obj %s already extracted!", vnum_to_dotted(obj->pIndexData->vnum) );
		return;
	}

	if ( obj->item_type == ITEM_PORTAL )
		remove_portal( obj );

	if ( obj->GetCarriedBy() )
		obj_from_char( obj );
	else if ( obj->GetInRoom() )
		obj_from_room( obj );
	else if ( obj->GetInObj() )
		obj_from_obj( obj );


	if (obj->GetQuestNote() )
		extract_obj(obj->GetQuestNote(),bDecrement);
	obj->QuestNoteId = 0;

	while ( ( obj_content = obj->last_content ) != NULL )
	{
		extract_obj( obj_content, bDecrement );
	}

	{
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;

		for ( paf = obj->first_affect; paf; paf = paf_next )
		{
			paf_next	= paf->next;
			DISPOSE( paf );
		}
		obj->first_affect = obj->last_affect = NULL;
	}

	{
		ExtraDescData *ed;
		ExtraDescData *ed_next;

		/* matthew */
		if ( obj->pObjNPC ) {
			DISPOSE(obj->pObjNPC);
		}

		for ( ed = obj->first_extradesc; ed; ed = ed_next )
		{
			ed_next = ed->next;
			delete ed;
		}
		obj->first_extradesc = obj->last_extradesc = NULL;
	}

	if ( obj == gobj_prev )
		gobj_prev		= obj->prev;

	UNLINK( obj, first_object, last_object, next, prev );
	/* shove onto extraction queue */
	queue_extracted_obj( obj );

	obj->pIndexData->count -= obj->count;

	if ( bDecrement ) {
		obj->pIndexData->total_count -= obj->count;

		if ( obj->pIndexData->total_count < obj->pIndexData->count ) {
			fprintf(stderr, "************ THIS SHOULD NOT HAPPEN ************\n");
			fprintf(stderr, "Vnum: %s, Name: %s  Total Count (%d) Brought below game count (%d)!\n",
					vnum_to_dotted(obj->pIndexData->vnum), obj->pIndexData->name_.c_str(),
					obj->pIndexData->total_count, obj->pIndexData->count);
			fprintf(stderr, "In function: extract_obj, Subtracted: %d\n", obj->count);
		}
	}


	numobjsloaded -= obj->count;
	--physicalobjects;
	if ( obj->serial == cur_obj )
	{
		cur_obj_extracted = TRUE;
		if ( global_objcode == rNONE )
			global_objcode = rOBJ_EXTRACTED;
	}
	return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *location;
	
	if ( !ch )
	{
		bug( "Extract_char: NULL ch.", 0 );
		return; 	/* who removed this line? */
	}
	
	if ( !ch->GetInRoom() )
	{
		bug( "Extract_char: NULL room.", 0 );
		return;
	}
	
	if ( ch == supermob )
	{
		bug( "Extract_char: ch == supermob!", 0 );
		return;
	}
	
	if ( char_died(ch) )
	{
		sprintf( buf, "extract_char: %s already died!", ch->getName().c_str() );
		bug( buf, 0 );
		return;
	}
	
	if ( ch == cur_char )
		cur_char_died = TRUE;
	/* shove onto extraction queue */
	queue_extracted_char( ch, fPull );
	
	if ( gch_prev == ch )
		gch_prev = ch->prev;
	
	if ( fPull && !IS_SET(ch->act, ACT_POLYMORPHED))
		die_follower( ch );
	
	//stop_fighting( ch, TRUE );
	ch->StopAllFights();
	
	if ( ch->GetMount() )
	{
		REMOVE_BIT( ch->GetMount()->act, ACT_MOUNTED );
		ch->MountId = 0;
		ch->position = POS_STANDING;
	}
	
	// Only remove objects if the player is being completely taken out
	// This way items stay on the player if they didn't go into the
	// corpse     -Ksilyan
	if ( !in_arena(ch) && fPull )
	{
		while ( (obj = ch->last_carrying) != NULL )
		{
			/* when the char dies, we don't want to decrement total_count! */
			extract_obj( obj, FALSE );
		}
	}
	if(ch->GetQuestNote())
		extract_obj(ch->GetQuestNote(),TRUE); /* note dies if char dies */
	ch->QuestNoteId = 0;
	
	/* Ksilyan:
	 * Loop through the list of spells we have cast.
	 * If ch has one, eliminate it.
	 */
	
	list<SpellMemory*>::iterator itor;
	
	// Create a local copy of the list to loop through.
	list<SpellMemory*> localList = ch->spellMemory_;
	
	for (itor = localList.begin(); itor != localList.end(); itor++)
	{
		SpellMemory * spell = *itor;
		
		/*// If the spell is on me, but I'm not the caster,
		// then get rid of it.
		if ( spell->Target == ch && ch != spell->Caster )
		{
			log_string("Quitting character is target, but not caster.");
			if ( ch->IsAffected(spell->Spell) )
			{
				log_string("Quitting character does indeed have affect.");
				affect_remove( ch, spell->Spell );
			}
		}*/
		
		// the spell caster is quitting, so we need to remove
		// the spells he/she has cast
		// added: immortal spells never go away, sep-16-2004 -ksilyan
		if ( ch == spell->Caster && !IS_NPC(ch) && !IS_IMMORTAL(ch) )
		{
			act( AT_MAGIC, "You feel magical power slipping away.", spell->Target, NULL, NULL, TO_CHAR );
			
			if ( spell->Target->IsAffected(spell->Spell) )
				affect_remove( spell->Target, spell->Spell);
		}
		
		spell->Target->spellMemory_.remove(spell);
		spell->Caster->spellMemory_.remove(spell);
		
		delete spell;
	}
	
	char_from_room( ch );
	
	if ( !fPull )
	{
		location = NULL;
		
		if ( !IS_NPC(ch) && ch->pcdata->clan )
			location = get_room_index( ch->pcdata->clan->recall );
		
		if ( !IS_NPC(ch) && ch->pcdata->recall_room )
			location = get_room_index( ch->pcdata->recall_room );
		
		if ( !location )
			location = get_room_index( ROOM_VNUM_ALTAR );
		
		if ( !location )
			location = get_room_index( 1 );
		
		char_to_room( ch, location );
		/*
		 * Make things a little fancier				-Thoric
		 */
		if ( ( wch = get_char_room( ch, "healer" ) ) != NULL )
		{
			act( AT_MAGIC, "$n mutters a few incantations, waves $s hands and points $s finger.",
				wch, NULL, NULL, TO_ROOM );
			act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
			sprintf(buf, "Welcome back to the land of the living, %s",
				capitalize( ch->getName().c_str() ) );
			do_say( wch, buf );
		}
		else
			act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
		ch->position = POS_RESTING;
		return;
	}
	
	if ( IS_NPC(ch) )
	{
		--ch->pIndexData->count;
		--nummobsloaded;
	}
	
	if ( ch->GetConnection() && ch->GetConnection()->OriginalCharId != 0 && IS_SET(ch->act, ACT_POLYMORPHED))
		do_revert( ch, "" );
	
	if ( ch->GetConnection() && ch->GetConnection()->OriginalCharId != 0 )
		do_return( ch, "" );
	
	UNLINK( ch, first_char, last_char, next, prev );
	
	if ( ch->GetConnection() ) {
		if ( ch->GetConnection()->CurrentCharId != ch->GetId() ) {
			bug( "Extract_char: char's descriptor points to another char", 0 );
		} else {
			ch->GetConnection()->CurrentCharId = 0;
			gConnectionManager->RemoveSocket(ch->GetConnection(), false);
			ch->SetConnection(NULL);
		}
	}
	
	return;
}


/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, const char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number, count, vnum;

    number = number_argument( argument, arg );
    if ( !str_cmp( arg, "self" ) )
	return ch;

    if ( get_trust(ch) >= LEVEL_ENGINEER )
    	vnum = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
    else
        vnum = -1;

    if ( vnum > 0 ) {
        number = 1;
    }

    count  = 0;

    for ( rch = ch->GetInRoom()->first_person; rch; rch = rch->next_in_room ) {
        if ( can_see( ch, rch )
        &&  (nifty_is_name( arg, rch->getName().c_str() )
        ||  (IS_NPC(rch) && vnum == rch->pIndexData->vnum)) )
        {
            if ( number == 1 && !IS_NPC(rch) )
                return rch;
            else
                if ( ++count == number )
                    return rch;
        }
    }
    if ( vnum > 0 )
	return NULL;

    /* If we didn't find an exact match, run through the list of characters
     * again looking for prefix matching, ie gu == guard.
     * Added by Narn, Sept/96
     */
    count  = 0;
    for ( rch = ch->GetInRoom()->first_person; rch; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !nifty_is_name_prefix( arg, rch->getName().c_str() ) )
	    continue;       
	if ( number == 1 && !IS_NPC(rch) )
	    return rch;
	else
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}



/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, const char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	int number, count, vnum;
	
	
	// number_argument() returns 1 by default, not 0!! -- Warp
	number = number_argument(argument, arg);
	count  = 0;
	
	if (!str_cmp(arg, "self"))
		return ch;
	
	/*
	 * Allow reference by vnum for saints+			-Thoric
	 */
	if (get_trust(ch) >= LEVEL_ENGINEER )
	{
		vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);
	}
	else
		vnum = -1;
	
	if ( vnum > 0 )
		number = 1;
	
	// check the room for an exact match
	for (wch = ch->GetInRoom()->first_person; wch; wch = wch->next_in_room)
	{
		if (can_see(ch, wch)
			&&	(nifty_is_name(arg, wch->getName().c_str())
			||	(IS_NPC(wch) && vnum == wch->pIndexData->vnum)))
		{
			if (number == 1 && !IS_NPC(wch))
			{
				return wch;
			}
			else if (++count == number)
			{
				return wch;
			}
		}
	}
	count = 0;
	
	/* check the world for an exact match */
	for (wch = first_char; wch; wch = wch->next)
	{
		if (can_see(ch, wch)
			&&	(nifty_is_name(arg, wch->getName().c_str())
			||	(IS_NPC(wch) && vnum == wch->pIndexData->vnum)))
		{
			if (number == 1 && !IS_NPC(wch))
				return wch;
			else if (++count == number)
					return wch;
		}
	}
		
	
	/* bail out if looking for a vnum match */
	if (vnum > 0)
		return NULL;
	
		/*
		* If we didn't find an exact match, check the room for
		* for a prefix match, ie gu == guard.
		* Added by Narn, Sept/96
	*/
	
	count  = 0;
	for (wch = ch->GetInRoom()->first_person; wch; wch = wch->next_in_room) {
		if (!can_see(ch, wch) || !nifty_is_name_prefix(arg, wch->getName().c_str()))
			continue;
		if (number == 1 && !IS_NPC(wch))
			return wch;
		else
			if (++count == number)
				return wch;
	}
	
	/*
	* If we didn't find a prefix match in the room, run through the full list
	* of characters looking for prefix matching, ie gu == guard.
	* Added by Narn, Sept/96
	*/
	count  = 0;
	for (wch = first_char; wch; wch = wch->next) {
		if (!can_see(ch, wch) || !nifty_is_name_prefix(arg, wch->getName().c_str()) || IS_NPC(wch))
			continue;
		
		return wch;
	}
	
	for (wch = first_char; wch; wch = wch->next) {
		if (!can_see(ch, wch) || !nifty_is_name_prefix(arg, wch->getName().c_str()) || !IS_NPC(wch))
			continue;
		if (number == 1 || ++count == number)
			return wch;
	}
	
	return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P', 'T' and 'H' commands.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = last_object; obj; obj = obj->prev )
	if ( obj->pIndexData == pObjIndex )
	    return obj;

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, const char *argument, OBJ_DATA *List )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = List; obj; obj = obj->next_content )
	if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name_.c_str() ) )
	    if ( (count += obj->count) >= number )
		return obj;

    /* If we didn't find an exact match, run through the list of objects
       again looking for prefix matching, ie swo == sword.
       Added by Narn, Sept/96
    */
    count = 0;
    for ( obj = List; obj; obj = obj->next_content )
	if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name_.c_str() ) )
	    if ( (count += obj->count) >= number )
		return obj;

    return NULL;
}

/*
 * Find an obj in a list...going the other way			-Thoric
 */
OBJ_DATA *get_obj_list_rev( CHAR_DATA *ch, const char *argument, OBJ_DATA *List )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = List; obj; obj = obj->prev_content )
	if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name_.c_str() ) )
	    if ( (count += obj->count) >= number )
		return obj;

    /* If we didn't find an exact match, run through the list of objects
       again looking for prefix matching, ie swo == sword.
       Added by Narn, Sept/96
    */
    count = 0;
    for ( obj = List; obj; obj = obj->prev_content )
	if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name_.c_str() ) )
	    if ( (count += obj->count) >= number )
		return obj;

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, const char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, count, vnum;

    number = number_argument( argument, arg );

    if ( get_trust(ch) >= LEVEL_ENGINEER )
        vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);
    else
        vnum = -1;

    if ( vnum > 0 )
        number = 1;

    count  = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	if ( obj->wear_loc == WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&  (nifty_is_name( arg, obj->name_.c_str() ) || obj->pIndexData->vnum == vnum) )
	    if ( (count += obj->count) >= number )
		return obj;

    if ( vnum > 0 )
	return NULL;

    /* If we didn't find an exact match, run through the list of objects
       again looking for prefix matching, ie swo == sword.
       Added by Narn, Sept/96
    */
    count = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	if ( obj->wear_loc == WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   nifty_is_name_prefix( arg, obj->name_.c_str() ) )
	    if ( (count += obj->count) >= number )
		return obj;

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, const char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, count, vnum;

    number = number_argument( argument, arg );

    if ( get_trust(ch) >= LEVEL_ENGINEER )
        vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);
    else
        vnum = -1;

    if ( vnum > 0 ) 
        number = 1;
  
    count  = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&  (nifty_is_name( arg, obj->name_.c_str() ) || obj->pIndexData->vnum == vnum) )
	    if ( ++count == number )
		return obj;

    if ( vnum > 0 )
	return NULL;

    /* If we didn't find an exact match, run through the list of objects
       again looking for prefix matching, ie swo == sword.
       Added by Narn, Sept/96
    */
    count = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   nifty_is_name_prefix( arg, obj->name_.c_str() ) )
	    if ( ++count == number )
		return obj;

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, const char *argument )
{
	OBJ_DATA *obj;

	obj = get_obj_list_rev( ch, argument, ch->GetInRoom()->last_content );
	if ( obj )
		return obj;

	if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
		return obj;

	if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
		return obj;

	return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, const char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, count, vnum;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );

    /*
     * Allow reference by vnum for saints+			-Thoric
     */
    if ( get_trust(ch) >= LEVEL_ENGINEER && is_number( arg ) )
	vnum = dotted_to_vnum( ch->GetInRoom()->vnum, arg );
    else
	vnum = -1;

    if ( vnum > 0 )
        number = 1;

    count  = 0;
    for ( obj = first_object; obj; obj = obj->next )
	if ( can_see_obj( ch, obj ) && (nifty_is_name( arg, obj->name_.c_str() )
	||   vnum == obj->pIndexData->vnum) )
	    if ( (count += obj->count) >= number )
		return obj;

    /* bail out if looking for a vnum */
    if ( vnum > 0 )
	return NULL;

    /* If we didn't find an exact match, run through the list of objects
       again looking for prefix matching, ie swo == sword.
       Added by Narn, Sept/96
    */
    count  = 0;
    for ( obj = first_object; obj; obj = obj->next )
	if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name_.c_str() ) )
	    if ( (count += obj->count) >= number )
		return obj;

    return NULL;
}


/*
 * How mental state could affect finding an object		-Thoric
 * Used by get/drop/put/quaff/recite/etc
 * Increasingly freaky based on mental state and drunkeness
 */
bool ms_find_obj( CHAR_DATA *ch )
{
    int ms = ch->mental_state;
    int drunk = IS_NPC(ch) ? 0 : ch->pcdata->condition[COND_DRUNK];
    const char *t;

    /*
     * we're going to be nice and let nothing weird happen unless
     * you're a tad messed up
     */
    drunk = UMAX( 1, drunk );
    if ( abs(ms) + (drunk/3) < 30 )
	return FALSE;
    if ( (number_percent() + (ms < 0 ? 15 : 5))> abs(ms)/2 + drunk/4 )
	return FALSE;
    if ( ms > 15 )	/* range 1 to 20 */
	switch( number_range( UMAX(1, (ms/5-15)), (ms+4) / 5 ) )
	{
	    default:
	    case  1: t="As you reach for it, you forgot what it was...\n\r";					break;
	    case  2: t="As you reach for it, something inside stops you...\n\r";				break;
	    case  3: t="As you reach for it, it seems to move out of the way...\n\r";				break;
	    case  4: t="You grab frantically for it, but can't seem to get a hold of it...\n\r";		break;
	    case  5: t="It disappears as soon as you touch it!\n\r";						break;
	    case  6: t="You would if it would stay still!\n\r";							break;
	    case  7: t="Whoa!  It's covered in blood!  Ack!  Ick!\n\r";						break;
	    case  8: t="Wow... trails!\n\r";									break;
	    case  9: t="You reach for it, then notice the back of your hand is growing something!\n\r";		break;
	    case 10: t="As you grasp it, it shatters into tiny shards which bite into your flesh!\n\r";		break;
	    case 11: t="What about that huge dragon flying over your head?!?!?\n\r";				break;
	    case 12: t="You stratch yourself instead...\n\r";							break;
	    case 13: t="You hold the universe in the palm of your hand!\n\r";					break;
	    case 14: t="You're too scared.\n\r";								break;
	    case 15: t="Your mother smacks your hand... 'NO!'\n\r";						break;
	    case 16: t="Your hand grasps the worse pile of revoltingness than you could ever imagine!\n\r";	break;
	    case 17: t="You stop reaching for it as it screams out at you in pain!\n\r";			break;
	    case 18: t="What about the millions of burrow-maggots feasting on your arm?!?!\n\r";		break;
	    case 19: t="That doesn't matter anymore... you've found the true answer to everything!\n\r";	break;
	    case 20: t="A supreme entity has no need for that.\n\r";						break;
	}
    else
    {
	int sub = URANGE(1, abs(ms)/2 + drunk, 60);
	switch( number_range( 1, sub/10 ) )
	{
	    default:
	    case  1: t="In just a second...\n\r";				break;
	    case  2: t="You can't find that...\n\r";					break;
	    case  3: t="It's just beyond your grasp...\n\r";				break;
	    case  4: t="...but it's under a pile of other stuff...\n\r";		break;
	    case  5: t="You go to reach for it, but pick your nose instead.\n\r";	break;
	    case  6: t="Which one?!?  I see two... no three...\n\r";			break;
	}
    }
    send_to_char( t, ch );
    return TRUE;
}


/*
 * Generic get obj function that supports optional containers.	-Thoric
 * currently only used for "eat" and "quaff".
 */
OBJ_DATA *find_obj( CHAR_DATA *ch, const char *argument, bool carryonly )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "from" )
    &&   argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg2[0] == '\0' )
    {
	if ( carryonly && ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return NULL;
	}
	else
	if ( !carryonly && ( obj = get_obj_here( ch, arg1 ) ) == NULL )
	{
	    act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
	    return NULL;
	}
	return obj;
    }
    else
    {
	OBJ_DATA *container;
	
	if ( carryonly
	&& ( container = get_obj_carry( ch, arg2 ) ) == NULL
	&& ( container = get_obj_wear( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return NULL;
	}
	if ( !carryonly && ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return NULL;
	}
	
	if ( !IS_OBJ_STAT(container, ITEM_COVERING )
	&&    IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name_.c_str(), TO_CHAR );
	    return NULL;
	}

	obj = get_obj_list( ch, arg1, container->first_content );
	if ( !obj )
	    act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
		"I see nothing like that beneath $p." :
		"I see nothing like that in $p.",
		ch, container, NULL, TO_CHAR );
	return obj;
    }
    return NULL;
}

int get_obj_number( OBJ_DATA *obj )
{
    return obj->count;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;

    weight = obj->count * obj->weight;
    for ( obj = obj->first_content; obj; obj = obj->next_content )
	weight += get_obj_weight( obj );

    return weight;
}



/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( !pRoomIndex )
    {
	bug( "room_is_dark: NULL pRoomIndex", 0 );
	return TRUE;
    }

    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_ALWAYS_LIGHT ) )
       return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;
    int pccount;

    if ( !pRoomIndex )
    {
	bug( "room_is_private: NULL pRoomIndex", 0 );
	return FALSE;
    }

    count = 0;
    pccount = 0;
    for ( rch = pRoomIndex->first_person; rch; rch = rch->next_in_room )
    {
    	count++;
        if ( !IS_NPC(rch) ) {
            ++pccount;
        }
    }

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_ONEPCONLY) && pccount )
        return TRUE;
    
    return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( !victim )
		return FALSE;

	if ( !ch )
	{
		if ( IS_AFFECTED(victim, AFF_INVISIBLE)
				||	 IS_AFFECTED(victim, AFF_HIDE)
				||	 IS_SET(victim->act, PLR_WIZINVIS) ) 
			return FALSE;
		else
			return TRUE;
	}


	if ( IS_NPC(ch) && ch->pIndexData->vnum == 3 ) {
		return TRUE; /* supermob can see all */
	}

	if ( ch == victim )
		return TRUE;

	if ( !IS_NPC(victim)
			&&   IS_SET(victim->act, PLR_WIZINVIS)
			&&   get_trust( ch ) < victim->pcdata->wizinvis )
		return FALSE;

	/* SB */
	if ( IS_NPC(victim)   
			&&   IS_SET(victim->act, ACT_MOBINVIS)
			&&   get_trust( ch ) < victim->mobinvis )
		return FALSE;

	// Added by Ksilyan  2005-june-20
	if ( IS_NPC(victim)
			&& IS_SET(victim->act, ACT_INVISIBLE_TO_PLAYERS)
			&& IS_IMMORTAL(ch) == false )
		return false;

	if ( !IS_IMMORTAL(ch) && !IS_NPC(victim) && !victim->GetConnection() 
			&&    get_timer(victim, TIMER_RECENTFIGHT) > 0
			&&  (!victim->GetSwitchedChar() || !IS_AFFECTED(victim->GetSwitchedChar(), AFF_POSSESS)) )
		return FALSE;

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
		return TRUE;

	/* The miracle cure for blindness? -- Altrag */
	if ( !IS_AFFECTED(ch, AFF_TRUESIGHT) )
	{
		if ( IS_AFFECTED(ch, AFF_BLIND) )
			return FALSE;

		if ( room_is_dark( ch->GetInRoom() ) && !IS_AFFECTED(ch, AFF_INFRARED) )
			return FALSE;

		if ( IS_AFFECTED(victim, AFF_INVISIBLE)
				&&  !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
			return FALSE;

		if ( IS_AFFECTED(victim, AFF_HIDE)
				&&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
				&&   !victim->IsFighting()
				&&   ( IS_NPC(ch) ? !IS_NPC(victim) : IS_NPC(victim) ) )
			return FALSE;
	}

	/* Redone by Narn to let newbie council members see pre-auths. */
	if( NOT_AUTHED( victim ) )
	{
		if( NOT_AUTHED( ch ) || IS_IMMORTAL( ch ) || IS_NPC( ch ) )
			return TRUE;

		if( ch->pcdata->council && ch->pcdata->council->name_.ciEqual( "Newbie Council" ) )
			return TRUE;

		return FALSE;
	}  

	/* Commented out for who list purposes 
	   if (!NOT_AUTHED(victim) && NOT_AUTHED(ch) && !IS_IMMORTAL(victim) 
	   && !IS_NPC(victim))
	   return FALSE;*/
	return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if ( !obj )
		return false;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;
	
    if ( IS_OBJ_STAT( obj, ITEM_BURIED ) )
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
	return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_HIDDEN) )
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( room_is_dark( ch->GetInRoom() ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_OBJ_STAT(obj, ITEM_NODROP) && !IS_OBJ_STAT(obj, ITEM_DEITY) )
    	return TRUE;
    
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
    	return TRUE;

    if ( IS_NPC(ch) && ch->pIndexData->vnum == 3 )
    	return TRUE;

    return FALSE;
}


/*
 * Return ascii name of an item type.
 */

 /* KSILYAN-
 	This is now obsolete! See itemtype_number_to_name instead. (translate.c)
	
char *item_type_name( OBJ_DATA *obj )
{
    if ( obj->item_type < 1 || obj->item_type > MAX_ITEM_TYPE )
    {
	bug( "Item_type_name: unknown type %d.", obj->item_type );
	return "(unknown)";
    }

    return o_types[obj->item_type];
}
*/



/*
 * Return ascii name of an affect location.
 */
const char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_CHA:		return "charisma";
    case APPLY_LCK:		return "luck";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SAVING_POISON:	return "save vs poison";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PARA:	return "save vs paralysis";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_HEIGHT:		return "height";
    case APPLY_WEIGHT:		return "weight";
    case APPLY_AFFECT:		return "affected_by";
    case APPLY_RESISTANT:	return "resistant";
    case APPLY_IMMUNE:		return "immune";
    case APPLY_SUSCEPTIBLE:	return "susceptible";
    case APPLY_BACKSTAB:	return "backstab";
    case APPLY_PICK:		return "pick";
    case APPLY_TRACK:		return "track";
    case APPLY_STEAL:		return "steal";
    case APPLY_SNEAK:		return "sneak";
    case APPLY_HIDE:		return "hide";
    case APPLY_PALM:		return "palm";
    case APPLY_DETRAP:		return "detrap";
    case APPLY_DODGE:		return "dodge";
    case APPLY_PEEK:		return "peek";
    case APPLY_SCAN:		return "scan";
    case APPLY_GOUGE:		return "gouge";
    case APPLY_SEARCH:		return "search";
    case APPLY_MOUNT:		return "mount";
    case APPLY_DISARM:		return "disarm";
    case APPLY_KICK:		return "kick";
    case APPLY_PARRY:		return "parry";
    case APPLY_BASH:		return "bash";
    case APPLY_STUN:		return "stun";
    case APPLY_PUNCH:		return "punch";
    case APPLY_CLIMB:		return "climb";
    case APPLY_GRIP:		return "grip";
    case APPLY_SCRIBE:		return "scribe";
    case APPLY_BREW:		return "brew";
    case APPLY_WEAPONSPELL:	return "weapon spell";
    case APPLY_WEARSPELL:	return "wear spell";
    case APPLY_REMOVESPELL:	return "remove spell";
    case APPLY_MENTALSTATE:	return "mental state";
    case APPLY_EMOTION:		return "emotional state";
    case APPLY_STRIPSN:		return "dispel";
    case APPLY_REMOVE:		return "remove";
    case APPLY_DIG:		return "dig";
    case APPLY_FULL:		return "hunger";
    case APPLY_THIRST:		return "thirst";
    case APPLY_DRUNK:		return "drunk";
    case APPLY_BLOOD:		return "blood";
    case APPLY_CONTAGION:       return "contagion";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
const char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
    if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
    if ( vector & AFF_EARTHMELD     ) strcat( buf, " earthmeld"     );
    if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
    if ( vector & AFF_ROCKMELD      ) strcat( buf, " rockmeld"      );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_PROTECT       ) strcat( buf, " protect"       );
    if ( vector & AFF_PARALYSIS     ) strcat( buf, " paralysis"     );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_POSSESS       ) strcat( buf, " possess"       );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
    if ( vector & AFF_FLOATING      ) strcat( buf, " floating"      );
    if ( vector & AFF_TRUESIGHT     ) strcat( buf, " true_sight"    );
    if ( vector & AFF_DETECTTRAPS   ) strcat( buf, " detect_traps"  );
    if ( vector & AFF_SCRYING       ) strcat( buf, " scrying"       );
    if ( vector & AFF_FIRESHIELD    ) strcat( buf, " fireshield"    );
    if ( vector & AFF_SHOCKSHIELD   ) strcat( buf, " shockshield"   );
    if ( vector & AFF_ICESHIELD     ) strcat( buf, " iceshield"     );
    if ( vector & AFF_POSSESS       ) strcat( buf, " possess"       );
    if ( vector & AFF_BERSERK       ) strcat( buf, " berserk"       );
    if ( vector & AFF_AQUA_BREATH   ) strcat( buf, " aqua_breath"   );
    return ( buf[0] != '\0' ) ? buf+1 : (char *) "none";
}



/*
 * Return ascii name of extra flags vector.
 */
const char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOYAL        ) strcat( buf, " loyal"        );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " anti-good"    );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " anti-evil"    );
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " anti-neutral" );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_DEATHROT	 ) strcat( buf, " deathrot"	);
    if ( extra_flags & ITEM_ANTI_MAGE    ) strcat( buf, " anti-mage"    );
    if ( extra_flags & ITEM_ANTI_THIEF   ) strcat( buf, " anti-thief"   );
    if ( extra_flags & ITEM_ANTI_WARRIOR ) strcat( buf, " anti-warrior" );
    if ( extra_flags & ITEM_ANTI_CLERIC  ) strcat( buf, " anti-cleric"  );
    if ( extra_flags & ITEM_ANTI_DRUID   ) strcat( buf, " anti-druid"   );
    if ( extra_flags & ITEM_ANTI_VAMPIRE ) strcat( buf, " anti-vampire" );
    if ( extra_flags & ITEM_ORGANIC      ) strcat( buf, " organic"      );
    if ( extra_flags & ITEM_METAL        ) strcat( buf, " metal"        );
    if ( extra_flags & ITEM_DONATION     ) strcat( buf, " donation"     );
    if ( extra_flags & ITEM_CLANOBJECT   ) strcat( buf, " clan"         );
    if ( extra_flags & ITEM_CLANCORPSE   ) strcat( buf, " clanbody"     );
    if ( extra_flags & ITEM_PROTOTYPE    ) strcat( buf, " prototype"    );
    if ( extra_flags & ITEM_DEITY        ) strcat( buf, " deity"        );
    return ( buf[0] != '\0' ) ? buf+1 : (char *) "none";
}

/*
 * Return ascii name of magic flags vector. - Scryn
 */
const char *magic_bit_name( int magic_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( magic_flags & ITEM_RETURNING     ) strcat( buf, " returning"     );
    return ( buf[0] != '\0' ) ? buf+1 : (char *) "none";
}

/*
 * Set off a trap (obj) upon character (ch)			-Thoric
 */
ch_ret spring_trap( CHAR_DATA *ch, OBJ_DATA *obj )
{
      int dam;
      int typ;
      int lev;
      const char *txt;
      char buf[MAX_STRING_LENGTH];
      ch_ret retcode;

      typ = obj->value[1];
      lev = obj->value[2];

      retcode = rNONE;
 
      switch(typ)
      {
       default:
	 txt = "hit by a trap";					break;
       case TRAP_TYPE_POISON_GAS:
	 txt = "surrounded by a green cloud of gas";		break;
       case TRAP_TYPE_POISON_DART:
	 txt = "hit by a dart";					break;
       case TRAP_TYPE_POISON_NEEDLE:
	 txt = "pricked by a needle";				break;
       case TRAP_TYPE_POISON_DAGGER:
	 txt = "stabbed by a dagger";				break;
       case TRAP_TYPE_POISON_ARROW:
	 txt = "struck with an arrow";				break;
       case TRAP_TYPE_BLINDNESS_GAS:
	 txt = "surrounded by a red cloud of gas";		break;
       case TRAP_TYPE_SLEEPING_GAS:
	 txt = "surrounded by a yellow cloud of gas";		break;
       case TRAP_TYPE_FLAME:
	 txt = "struck by a burst of flame";			break;
       case TRAP_TYPE_EXPLOSION:
	 txt = "hit by an explosion";				break;
       case TRAP_TYPE_ACID_SPRAY:
	 txt = "covered by a spray of acid";			break;
       case TRAP_TYPE_ELECTRIC_SHOCK:
	 txt = "suddenly shocked";				break;
       case TRAP_TYPE_BLADE:
	 txt = "sliced by a razor sharp blade";			break;
       case TRAP_TYPE_SEX_CHANGE:
	 txt = "surrounded by a mysterious aura";		break;
      }

      dam = number_range( obj->value[2], obj->value[2] * 2);
      sprintf( buf, "You are %s!", txt );
      act( AT_HITME, buf, ch, NULL, NULL, TO_CHAR );
      sprintf( buf, "$n is %s.", txt );
      act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
      --obj->value[0];
      if ( obj->value[0] <= 0 )
	extract_obj( obj, TRUE );
      switch(typ)
      {
       default:
       case TRAP_TYPE_POISON_DART:
       case TRAP_TYPE_POISON_NEEDLE:
       case TRAP_TYPE_POISON_DAGGER:
       case TRAP_TYPE_POISON_ARROW:
	 /* hmm... why not use spell_poison() here? */
	 retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
	 if ( retcode == rNONE )
	   retcode = damage( ch, ch, dam, TYPE_UNDEFINED, 0 );
	 break;
       case TRAP_TYPE_POISON_GAS:
	 retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
	 break;
       case TRAP_TYPE_BLINDNESS_GAS:
	 retcode = obj_cast_spell( gsn_blindness, lev, ch, ch, NULL );
	 break;
       case TRAP_TYPE_SLEEPING_GAS:
	 retcode = obj_cast_spell( skill_lookup("sleep"), lev, ch, ch, NULL );
	 break;
       case TRAP_TYPE_ACID_SPRAY:
	 retcode = obj_cast_spell( skill_lookup("acid blast"), lev, ch, ch, NULL );
	 break;
       case TRAP_TYPE_SEX_CHANGE:
	 retcode = obj_cast_spell( skill_lookup("change sex"), lev, ch, ch, NULL );
	 break;
       case TRAP_TYPE_FLAME:
       case TRAP_TYPE_EXPLOSION:
	 retcode = obj_cast_spell( gsn_fireball, lev, ch, ch, NULL );
	 break;
       case TRAP_TYPE_ELECTRIC_SHOCK:
       case TRAP_TYPE_BLADE:
	 retcode = damage( ch, ch, dam, TYPE_UNDEFINED, 0 );
      }
      return retcode;
}

/*
 * Check an object for a trap					-Thoric
 */
ch_ret check_for_trap( CHAR_DATA *ch, OBJ_DATA *obj, int flag )
{
  OBJ_DATA *check;
  ch_ret    retcode;

  if ( !obj->first_content )
    return rNONE;

  retcode = rNONE;

  for ( check = obj->first_content; check; check = check->next_content )
    if ( check->item_type == ITEM_TRAP
    &&   IS_SET(check->value[3], flag) )
    {
      retcode = spring_trap( ch, check );
      if ( retcode != rNONE )
	return retcode;
    }
  return retcode;
}

/*
 * Check the room for a trap					-Thoric
 */
ch_ret check_room_for_traps( CHAR_DATA *ch, int flag )
{
    OBJ_DATA *check;
    ch_ret    retcode;
  
    retcode = rNONE;

    if ( !ch )
      return rERROR;
    if ( !ch->GetInRoom() || !ch->GetInRoom()->first_content )
      return rNONE;

    for ( check = ch->GetInRoom()->first_content; check; check = check->next_content )
    {
	if ( check->item_type == ITEM_TRAP
	&&   IS_SET(check->value[3], flag) )
	{
	   retcode = spring_trap( ch, check );
	   if ( retcode != rNONE )
	     return retcode;
	}
    }
    return retcode;
}

/*
 * return TRUE if an object contains a trap			-Thoric
 */
bool is_trapped( OBJ_DATA *obj )
{
    OBJ_DATA *check;

    if ( !obj->first_content )
      return FALSE;

    for ( check = obj->first_content; check; check = check->next_content )
      if ( check->item_type == ITEM_TRAP )
	return TRUE;

    return FALSE;
}

/*
 * If an object contains a trap, return the pointer to the trap	-Thoric
 */
OBJ_DATA *get_trap( OBJ_DATA *obj )
{
    OBJ_DATA *check;

    if ( !obj->first_content )
      return NULL;

    for ( check = obj->first_content; check; check = check->next_content )
      if ( check->item_type == ITEM_TRAP )
	return check;

    return NULL;
}

/*
 * Remove an exit from a room					-Thoric
 */
void extract_exit( ROOM_INDEX_DATA *room, ExitData *pexit )
{
    UNLINK( pexit, room->first_exit, room->last_exit, next, prev );
    if ( pexit->rexit )
      pexit->rexit->rexit = NULL;
	delete pexit;
}

/*
 * Remove a room
 */
void extract_room( ROOM_INDEX_DATA *room )
{
  bug( "extract_room: not implemented", 0 );
  /*
  (remove room from hash table)
  clean_room( room )
  DISPOSE( room );
   */
  return;
}

/*
 * clean out a room (leave list pointers intact )		-Thoric
 */
void clean_room( ROOM_INDEX_DATA *room )
{
   ExtraDescData	*ed, *ed_next;
   ExitData		*pexit, *pexit_next;

   for ( ed = room->first_extradesc; ed; ed = ed_next )
   {
	ed_next = ed->next;
	delete ed;
	top_ed--;
   }
   room->first_extradesc	= NULL;
   room->last_extradesc		= NULL;
   for ( pexit = room->first_exit; pexit; pexit = pexit_next )
   {
	pexit_next = pexit->next;
	delete pexit;
	top_exit--;
   }
   room->first_exit = NULL;
   room->last_exit = NULL;
   room->room_flags = 0;
   room->sector_type = 0;
   room->light = 0;
}

/*
 * clean out an object (index) (leave list pointers intact )	-Thoric
 */
void clean_obj( OBJ_INDEX_DATA *obj )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	ExtraDescData *ed;
	ExtraDescData *ed_next;

	obj->item_type		= 0;
	obj->extra_flags	= 0;
	obj->wear_flags		= 0;
	obj->count		= 0;
	obj->weight		= 0;
	obj->cost		= 0;
	obj->value[0]		= 0;
	obj->value[1]		= 0;
	obj->value[2]		= 0;
	obj->value[3]		= 0;
	for ( paf = obj->first_affect; paf; paf = paf_next )
	{
	    paf_next    = paf->next;
	    DISPOSE( paf );
	    top_affect--;
	}
	obj->first_affect	= NULL;
	obj->last_affect	= NULL;
	for ( ed = obj->first_extradesc; ed; ed = ed_next )
	{
	    ed_next		= ed->next;
	    delete ed;
	    top_ed--;
	}
	obj->first_extradesc	= NULL;
	obj->last_extradesc	= NULL;
}

/*
 * clean out a mobile (index) (leave list pointers intact )	-Thoric
 */
void clean_mob( MOB_INDEX_DATA *mob )
{
	MPROG_DATA *mprog, *mprog_next;

	mob->spec_fun	= NULL;
	mob->pShop	= NULL;
	mob->rShop	= NULL;
	mob->progtypes	= 0;
	
	for ( mprog = mob->mudprogs; mprog; mprog = mprog_next )
	{
	    mprog_next = mprog->next;
	    STRFREE( mprog->arglist );
	    STRFREE( mprog->comlist );
	    DISPOSE( mprog );
	}
	mob->count	 = 0;	   mob->killed		= 0;
	mob->sex	 = 0;	   mob->level		= 0;
	mob->act	 = 0;	   mob->affected_by	= 0;
	mob->alignment	 = 0;	   mob->mobthac0	= 0;
	mob->ac		 = 0;	   mob->hitnodice	= 0;
	mob->hitsizedice = 0;	   mob->hitplus		= 0;
	mob->damnodice	 = 0;	   mob->damsizedice	= 0;
	mob->damplus	 = 0;	   mob->gold		= 0;
	mob->exp	 = 0;	   mob->position	= 0;
	mob->defposition = 0;	   mob->height		= 0;
	mob->weight	 = 0;	/* mob->vnum		= 0;	*/
}

extern int top_reset;

/*
 * Remove all resets from an area				-Thoric
 */
void clean_resets( AREA_DATA *tarea )
{
    RESET_DATA *pReset, *pReset_next;

    for ( pReset = tarea->first_reset; pReset; pReset = pReset_next )
    {
	pReset_next = pReset->next;
	DISPOSE( pReset );
	--top_reset;
    }
    tarea->first_reset	= NULL;
    tarea->last_reset	= NULL;
}


/*
 * "Roll" players stats based on the character name		-Thoric
 */
static void fiddle(sh_int *ps)
{
   switch(number_bits(2))
   {
     case 3:
        if(*ps < 23)
	  (*ps)++;
        break;
     case 0:
        if(*ps > 5)
          (*ps)--;
        break;
   }
}

void name_stamp_stats( CHAR_DATA *ch )
{
#if 0
    int x, a, b, c;

    for ( x = 0; x < strlen(ch->name); x++ )
    {
	c = ch->name[x] + x;
	b = c % 14;
	a = (c % 1) + 1;
	switch (b)
	{
	   case  0:
	     ch->perm_str = UMIN( 18, ch->perm_str + a );
	     break;
	   case  1:
	     ch->perm_dex = UMIN( 18, ch->perm_dex + a );
	     break;
	   case  2:
	     ch->perm_wis = UMIN( 18, ch->perm_wis + a );
	     break;
	   case  3:
	     ch->perm_int = UMIN( 18, ch->perm_int + a );
	     break;
	   case  4:
	     ch->perm_con = UMIN( 18, ch->perm_con + a );
	     break;
	   case  5:
	     ch->perm_cha = UMIN( 18, ch->perm_cha + a );
	     break;
	   case  6:
	     ch->perm_lck = UMIN( 18, ch->perm_lck + a );
	     break;
	   case  7:
	     ch->perm_str = UMAX(  9, ch->perm_str - a );
	     break;
	   case  8:
	     ch->perm_dex = UMAX(  9, ch->perm_dex - a );
	     break;
	   case  9:
	     ch->perm_wis = UMAX(  9, ch->perm_wis - a );
	     break;
	   case 10:
	     ch->perm_int = UMAX(  9, ch->perm_int - a );
	     break;
	   case 11:
	     ch->perm_con = UMAX(  9, ch->perm_con - a );
	     break;
	   case 12: 
	     ch->perm_cha = UMAX(  9, ch->perm_cha - a );
	     break;
	   case 13:
	     ch->perm_lck = UMAX(  9, ch->perm_lck - a );
	     break;
	}
    }
#else
   fiddle(&ch->perm_str);
   fiddle(&ch->perm_int);
   fiddle(&ch->perm_wis);
   fiddle(&ch->perm_dex);
   fiddle(&ch->perm_con);
   fiddle(&ch->perm_cha);
   fiddle(&ch->perm_lck);
#endif
}

void fix_affected_by(CHAR_DATA *ch)
{
  ch->affected_by = race_table[ch->race].affected;
  if(ch->Class == CLASS_VAMPIRE)
    ch->affected_by |= AFF_INFRARED;
}

/*
 * Updated fix_char to reflect new changes n' stuff.
 *       -Ksilyan
 */

void new_fix_char( Character * doer, Character * victim )
{
	// Make sure no equipment is worn...
	Object * obj;
	for ( obj = victim->first_carrying; obj; obj = obj->next_content )
	{
		if ( obj->wear_loc != -1 )
		{
			if ( !doer )
				gTheWorld->LogString( victim->getName().str() + " is still holding/wearing objects.\n\r" );
			else if ( doer == victim )
				doer->sendText( "You are still holding/wearing objects.\n\r" );
			else
				doer->sendText( victim->getName().str() + " is still holding/wearing objects.\n\r" );
			return;
		}
	}

	if ( victim->first_affect != NULL )
	{
		if ( !doer )
			gTheWorld->LogString( victim->getName().str() + " still has spells/effects.\n\r" );
		else if ( doer == victim )
			doer->sendText( "You still have spells/effects.\n\r" );
		else
			doer->sendText( victim->getName().str() + " still has spells/effects.\n\r" );
		return;
	}

	fix_affected_by(victim);
	victim->mod_str = victim->mod_dex = victim->mod_wis = victim->mod_int =
		victim->mod_con = victim->mod_lck = victim->mod_cha = 0;

	victim->max_hit = victim->BaseMaxHp;
	victim->max_mana = victim->BaseMaxMana;
	victim->max_move = victim->BaseMaxMove;

	victim->hit = victim->mana = victim->move = 1;

	victim->AddWaitSeconds(15);
}


/*
 * "Fix" a character's stats					-Thoric
 */
void fix_char( CHAR_DATA *ch )
{
    AFFECT_DATA *aff;
    OBJ_DATA *carry[MAX_LEVEL*200];
    OBJ_DATA *obj;
    int x, ncarry;

    de_equip_char( ch );

    ncarry = 0;
    while ( (obj=ch->first_carrying) != NULL )
    {
	carry[ncarry++]  = obj;
	obj_from_char( obj );
    }

    for ( aff = ch->first_affect; aff; aff = aff->next )
	affect_modify( ch, aff, FALSE );

    fix_affected_by( ch );
    ch->mental_state	= -10;
    ch->hit		= UMAX( 1, ch->hit  );
    ch->mana		= UMAX( 1, ch->mana );
    ch->move		= UMAX( 1, ch->move );
    ch->armor		= 100;
    ch->mod_str		= 0;
    ch->mod_dex		= 0;
    ch->mod_wis		= 0;
    ch->mod_int		= 0;
    ch->mod_con		= 0;
    ch->mod_cha		= 0;
    ch->mod_lck		= 0;
    ch->damroll		= 0;
    ch->hitroll		= 0;
    ch->alignment	= URANGE( -1000, ch->alignment, 1000 );
    ch->saving_breath	= 0;
    ch->saving_wand	= 0;
    ch->saving_para_petri = 0;
    ch->saving_spell_staff = 0;
    ch->saving_poison_death = 0;

    ch->carry_weight	= 0;
    ch->carry_number	= 0;

    for ( aff = ch->first_affect; aff; aff = aff->next )
	affect_modify( ch, aff, TRUE );

    for ( x = 0; x < ncarry; x++ )
	obj_to_char( carry[x], ch );

    re_equip_char( ch );
}


/*
 * Show an affect verbosely to a character			-Thoric
 */
void showaffect( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	char buf[MAX_STRING_LENGTH];
	int x;

	if ( !paf )
	{
	    bug( "showaffect: NULL paf", 0 );
	    return;
	}
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    switch( paf->location )
	    {
	      default:
		sprintf( buf, "Affects %s by %d.\n\r",
		  affect_loc_name( paf->location ), paf->modifier );
		break;
	      case APPLY_AFFECT:
		sprintf( buf, "Affects %s by",
		  affect_loc_name( paf->location ) );
		for ( x = 0; x < 32 ; x++ )
		if ( IS_SET( paf->modifier, 1 << x ) )
		{
		  strcat( buf, " " );
		  strcat( buf, a_flags[x] );
		}
		strcat( buf, "\n\r" );
		break;
	      case APPLY_WEAPONSPELL:
	      case APPLY_WEARSPELL:
	      case APPLY_REMOVESPELL:
		sprintf( buf, "Casts spell '%s'\n\r",
			IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name_.c_str()
						   : "unknown" );
		break;
	      case APPLY_RESISTANT:
	      case APPLY_IMMUNE:
	      case APPLY_SUSCEPTIBLE:
		sprintf( buf, "Affects %s by",
		  affect_loc_name( paf->location ) );
		for ( x = 0; x < 32 ; x++ )
		if ( IS_SET( paf->modifier, 1 << x ) )
		{
		  strcat( buf, " " );
		  strcat( buf, ris_flags[x] );
		}
		strcat( buf, "\n\r" );
		break;
	    }
	    send_to_char( buf, ch );
	}
}

/*
 * Set the current global object to obj				-Thoric
 */
void set_cur_obj( OBJ_DATA *obj )
{
    cur_obj = obj->serial;
    cur_obj_extracted = FALSE;
    global_objcode = rNONE;
}

/*
 * Check the recently extracted object queue for obj		-Thoric
 */
bool obj_extracted( OBJ_DATA *obj )
{
    OBJ_DATA *cod;

    if ( obj->serial == cur_obj
    &&   cur_obj_extracted )
	return TRUE;

    for (cod = extracted_obj_queue; cod; cod = cod->next )
	if ( obj == cod )
	     return TRUE;
    return FALSE;
}

/*
 * Stick obj onto extraction queue
 */
void queue_extracted_obj( OBJ_DATA *obj )
{
    
    ++cur_qobjs;
    obj->next = extracted_obj_queue;
    extracted_obj_queue = obj;
}

/*
 * Clean out the extracted object queue
 */
void clean_obj_queue()
{
	Object *obj;

	while ( extracted_obj_queue )
	{
		obj = extracted_obj_queue;
		extracted_obj_queue = extracted_obj_queue->next;
		delete obj;
		--cur_qobjs;
	}
}

/*
 * Set the current global character to ch			-Thoric
 */
void set_cur_char( CHAR_DATA *ch )
{
    cur_char	   = ch;
    cur_char_died  = FALSE;
    cur_room	   = ch->GetInRoom();
    global_retcode = rNONE;
}

/*
 * Check to see if ch died recently				-Thoric
 */
bool char_died( CHAR_DATA *ch )
{
    EXTRACT_CHAR_DATA *ccd;

    if ( ch == cur_char && cur_char_died )
	return TRUE;

    for (ccd = extracted_char_queue; ccd; ccd = ccd->next )
	if ( ccd->ch == ch )
	     return TRUE;
    return FALSE;
}

/*
 * Add ch to the queue of recently extracted characters		-Thoric
 */
void queue_extracted_char( CHAR_DATA *ch, bool extract )
{
	EXTRACT_CHAR_DATA *ccd;
	
	if ( !ch )
	{
		bug( "queue_extracted char: ch = NULL", 0 );
		return;
	}
	CREATE( ccd, EXTRACT_CHAR_DATA, 1 );
	ccd->ch 		= ch;
	ccd->room			= ch->GetInRoom();
	ccd->extract		= extract;
	if ( ch == cur_char )
		ccd->retcode		= global_retcode;
	else
		ccd->retcode		= rCHAR_DIED;
	ccd->next			= extracted_char_queue;
	extracted_char_queue	= ccd;
	cur_qchars++;
}

/*
 * clean out the extracted character queue
 */
void clean_char_queue()
{
	EXTRACT_CHAR_DATA *ccd;

	for ( ccd = extracted_char_queue; ccd; ccd = extracted_char_queue )
	{
		extracted_char_queue = ccd->next;
		if ( ccd->extract )
			free_char( ccd->ch );
		DISPOSE( ccd );
		--cur_qchars;
	}
}

/*
 * Add a timer to ch						-Thoric
 * Support for "call back" time delayed commands
 */
void add_timer( CHAR_DATA *ch, sh_int type, sh_int count, DO_FUN *fun, int value )
{
    TIMER *timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
	if ( timer->type == type )
	{
	   timer->count  = count;
	   timer->do_fun = fun;
	   timer->value	 = value;
	   break;
	}	
    if ( !timer )
    {
	CREATE( timer, TIMER, 1 );
	timer->count	= count;
	timer->type	= type;
	timer->do_fun	= fun;
	timer->value	= value;
	LINK( timer, ch->first_timer, ch->last_timer, next, prev );
    }
}

TIMER *get_timerptr( CHAR_DATA *ch, sh_int type )
{
    TIMER *timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
      if ( timer->type == type )
        return timer;
    return NULL;
}

sh_int get_timer( CHAR_DATA *ch, sh_int type )
{
    TIMER *timer;

    if ( (timer = get_timerptr( ch, type )) != NULL )
      return timer->count;
    else
      return 0;
}

void extract_timer( CHAR_DATA *ch, TIMER *timer )
{
	if ( !ch )
	{
		bug ("extract_timer: NULL ch!!", 0 );
		return;
	}

	if ( !timer )
	{
		bug( "extract_timer: NULL timer", 0 );
		return;
	}

	UNLINK( timer, ch->first_timer, ch->last_timer, next, prev );
	DISPOSE( timer );
	return;
}

void remove_timer( CHAR_DATA *ch, sh_int type )
{
    TIMER *timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
       if ( timer->type == type )
         break;

    if ( timer )
      extract_timer( ch, timer );
}

bool in_soft_range( CHAR_DATA *ch, AREA_DATA *tarea )
{
  if ( IS_IMMORTAL(ch) )
    return TRUE;
  else
  if ( IS_NPC(ch) )
    return TRUE;
  else
  if ( ch->level >= tarea->low_soft_range || ch->level <= tarea->hi_soft_range )
    return TRUE;
  else
    return FALSE;
}

bool in_hard_range( CHAR_DATA *ch, AREA_DATA *tarea )
{
  if ( IS_IMMORTAL(ch) )
    return TRUE;
  else
  if ( IS_NPC(ch) )
    return TRUE;
  else
  if ( ch->level >= tarea->low_hard_range && ch->level <= tarea->hi_hard_range )
    return TRUE;
  else
    return FALSE;
}


/*
 * Make a simple clone of an object (no extras...yet)		-Thoric
 */
OBJ_DATA *clone_object( OBJ_DATA *obj )
{
    OBJ_DATA *clone;

	clone = new Object(); 
    clone->pIndexData	= obj->pIndexData;
    clone->name_        = obj->name_;
    clone->shortDesc_   = obj->shortDesc_;
	clone->longDesc_    = obj->longDesc_;
    clone->actionDesc_	= obj->actionDesc_;
    clone->item_type	= obj->item_type;
    clone->extra_flags	= obj->extra_flags;
    clone->magic_flags	= obj->magic_flags;
    clone->wear_flags	= obj->wear_flags;
    clone->wear_loc	= obj->wear_loc;
    clone->weight	= obj->weight;
    clone->cost		= obj->cost;
#ifdef USE_OBJECT_LEVELS
   clone->level	= obj->level;
#endif   
    clone->timer	= obj->timer;
    clone->value[0]	= obj->value[0];
    clone->value[1]	= obj->value[1];
    clone->value[2]	= obj->value[2];
    clone->value[3]	= obj->value[3];
    clone->value[4]	= obj->value[4];
    clone->value[5]	= obj->value[5];
	clone->condition = obj->condition;
	clone->max_condition = obj->max_condition;
    clone->count	= 1;
    ++obj->pIndexData->count;
    ++obj->pIndexData->total_count;
    ++numobjsloaded;
    ++physicalobjects;
    cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
    clone->serial = clone->pIndexData->serial = cur_obj_serial;
    LINK( clone, first_object, last_object, next, prev );
    return clone;
}

/*
 * If possible group obj2 into obj1				-Thoric
 * This code, along with clone_object, obj->count, and special support
 * for it implemented throughout handler.c and save.c should show improved
 * performance on MUDs with players that hoard tons of potions and scrolls
 * as this will allow them to be grouped together both in memory, and in
 * the player files.
 */
OBJ_DATA *group_object( OBJ_DATA *obj1, OBJ_DATA *obj2 )
{
	if ( !obj1 || !obj2 )
		return NULL;
	if ( obj1 == obj2 )
		return obj1;

	if ( obj1->pIndexData == obj2->pIndexData
			/*
			   &&	!obj1->pIndexData->mudprogs
			   &&	!obj2->pIndexData->mudprogs
			 */
			&&	 obj1->name_       == obj2->name_
			&&	 obj1->shortDesc_  == obj2->shortDesc_
			&&	 obj1->longDesc_   == obj2->longDesc_
			&&	 obj1->actionDesc_ == obj2->actionDesc_
			&&	 obj1->item_type   == obj2->item_type
			&&	 obj1->extra_flags == obj2->extra_flags
			&&	 obj1->magic_flags == obj2->magic_flags
			&&	 obj1->wear_flags  == obj2->wear_flags
			&&	 obj1->wear_loc    == obj2->wear_loc
			&&	 obj1->weight      == obj2->weight
			&&	 obj1->cost 	== obj2->cost
#ifdef USE_OBJECT_LEVELS
			&&	 obj1->level		== obj2->level
#endif	
			&&	 obj1->timer		== obj2->timer
			&&	 obj1->value[0] 	== obj2->value[0]
			&&	 obj1->value[1] 	== obj2->value[1]
			&&	 obj1->value[2] 	== obj2->value[2]
			&&	 obj1->value[3] 	== obj2->value[3]
			&&	 obj1->value[4] 	== obj2->value[4]
			&&	 obj1->value[5] 	== obj2->value[5]
			&&	 obj1->condition	== obj2->condition
			&&	 obj1->max_condition	== obj2->max_condition
			&&	!obj1->first_extradesc	&& !obj2->first_extradesc
			&&	!obj1->first_affect && !obj2->first_affect
			&&	!obj1->first_content	&& !obj2->first_content 
			)
			{
				if (obj1->item_type == ITEM_PROJECTILE)
				{
					if (obj1->max_condition >= 60)
					{
						/* Arbitrary limit on number of projectiles per stack. */
						return obj2;
					}
					/* Quick fix for projectiles! */
					obj1->max_condition += obj2->max_condition;
					obj1->condition += obj2->condition;
				}
				else
				{
					obj1->count += obj2->count;
					obj1->pIndexData->count += obj2->count; /* to be decremented in */
					obj1->pIndexData->total_count += obj2->count;
					numobjsloaded += obj2->count;		/* extract_obj */
				}
				extract_obj( obj2, TRUE );
				return obj1;
			}
	return obj2;
}

/*
 * Split off a grouped object					-Thoric
 * decreased obj's count to num, and creates a new object containing the rest
 */
void split_obj( OBJ_DATA *obj, int num )
{
	int count = obj->count;
	OBJ_DATA *rest;
	
	if ( count <= num || num == 0 )
		return;
	
	rest = clone_object(obj);
	--obj->pIndexData->count;	/* since clone_object() ups this value */
	--obj->pIndexData->total_count;
	
	if ( obj->pIndexData->total_count < obj->pIndexData->count ) {
		fprintf(stderr, "************ THIS SHOULD NOT HAPPEN ************\n");
		fprintf(stderr, "Vnum: %s, Name: %s  Total Count (%d) Brought below game count (%d)!\n",
			vnum_to_dotted(obj->pIndexData->vnum), obj->pIndexData->name_.c_str(),
			obj->pIndexData->total_count, obj->pIndexData->count);
		fprintf(stderr, "In function: extract_obj, Subtracted: %d\n", obj->count);
	}
	
	--numobjsloaded;
	rest->count = obj->count - num;
	obj->count = num;
	
	if ( obj->GetCarriedBy() )
	{
		LINK( rest, obj->GetCarriedBy()->first_carrying,
			obj->GetCarriedBy()->last_carrying,
			next_content, prev_content );
		rest->CarriedById = obj->GetCarriedBy()->GetId();
		rest->InRoomId = 0;
		rest->InObjId = 0;
	}
	else if ( obj->GetInRoom() )
	{
		LINK( rest, obj->GetInRoom()->first_content, obj->GetInRoom()->last_content,
			next_content, prev_content );
		rest->CarriedById = 0;
		rest->InRoomId = obj->GetInRoom()->GetId();
		rest->InObjId = 0;
	}
	else if ( obj->GetInObj() )
	{
		LINK( rest, obj->GetInObj()->first_content, obj->GetInObj()->last_content,
			next_content, prev_content );
		rest->InObjId = obj->GetInObj()->GetId();
		rest->InRoomId = 0;
		rest->CarriedById = 0;
	}
}

void separate_obj( OBJ_DATA *obj )
{
    split_obj( obj, 1 );
}

/*
 * Empty an obj's contents... optionally into another obj, or a room
 */
bool empty_obj( OBJ_DATA *obj, OBJ_DATA *destobj, ROOM_INDEX_DATA *destroom )
{
	OBJ_DATA *otmp, *otmp_next;
	CHAR_DATA *ch = obj->GetCarriedBy();
	bool movedsome = FALSE;
	
	if ( !obj )
	{
		bug( "empty_obj: NULL obj", 0 );
		return FALSE;
	}
	if ( destobj || (!destroom && !ch && (destobj = obj->GetInObj()) != NULL) )
	{
		for ( otmp = obj->first_content; otmp; otmp = otmp_next )
		{
			otmp_next = otmp->next_content;
			
			if ( destobj->item_type == ITEM_QUIVER && otmp->item_type != ITEM_PROJECTILE )
				continue;
			
			if ( (destobj->item_type == ITEM_CONTAINER || destobj->item_type == ITEM_QUIVER)
				&&	 get_obj_weight( otmp ) + get_obj_weight( destobj )
				> destobj->value[0] )
				continue;
			obj_from_obj( otmp );
			obj_to_obj( otmp, destobj );
			movedsome = TRUE;
		}
		return movedsome;
	}
	if ( destroom || (!ch && (destroom = obj->GetInRoom()) != NULL) )
	{
		for ( otmp = obj->first_content; otmp; otmp = otmp_next )
		{
			otmp_next = otmp->next_content;
			if ( ch && (otmp->pIndexData->progtypes & DROP_PROG) && otmp->count > 1 ) 
			{
				separate_obj( otmp );
				obj_from_obj( otmp );
				if ( !otmp_next )
					otmp_next = obj->first_content;
			}
			else
				obj_from_obj( otmp );
			otmp = obj_to_room( otmp, destroom );
			if ( ch )
			{
				oprog_drop_trigger( ch, otmp ); 	/* mudprogs */
				if ( char_died(ch) )
					ch = NULL;
			}
			movedsome = TRUE;
		}
		return movedsome;
	}
	if ( ch )
	{
		for ( otmp = obj->first_content; otmp; otmp = otmp_next )
		{
			otmp_next = otmp->next_content;
			obj_from_obj( otmp );
			obj_to_char( otmp, ch );
			movedsome = TRUE;
		}
		return movedsome;
	}
	bug( "empty_obj: could not determine a destination for vnum %s",
		vnum_to_dotted(obj->pIndexData->vnum) );
	return FALSE;
}

/*
 * Improve mental state						-Thoric
 */
void better_mental_state( CHAR_DATA *ch, int mod )
{
    int c = URANGE( 0, abs(mod), 20 );
    int con = ch->getCon();

    c += number_percent() < con ? 1 : 0;

    if ( ch->mental_state < 0 )
	ch->mental_state = URANGE( -100, ch->mental_state + c, 0 );
    else
    if ( ch->mental_state > 0 )
	ch->mental_state = URANGE( 0, ch->mental_state - c, 100 );
}

/*
 * Deteriorate mental state					-Thoric
 */
void worsen_mental_state( CHAR_DATA *ch, int mod )
{
    int c   = URANGE( 0, abs(mod), 20 );
    int con = ch->getCon();


    c -= number_percent() < con ? 1 : 0;
    if ( c < 1 )
	return;

    if ( ch->mental_state < 0 )
	ch->mental_state = URANGE( -100, ch->mental_state - c, 100 );
    else
    if ( ch->mental_state > 0 )
	ch->mental_state = URANGE( -100, ch->mental_state + c, 100 );
    else
	ch->mental_state -= c;
}


/*
 * Add gold to an area's economy				-Thoric
 */
void boost_economy( AREA_DATA *tarea, int gold )
{
    while ( gold >= 1000000000 )
    {
	++tarea->high_economy;
	gold -= 1000000000;
    }
    tarea->low_economy += gold;
    while ( tarea->low_economy >= 1000000000 )
    {
	++tarea->high_economy;
	tarea->low_economy -= 1000000000;
    }
}

/*
 * Take gold from an area's economy				-Thoric
 */
void lower_economy( AREA_DATA *tarea, int gold )
{
    while ( gold >= 1000000000 )
    {
	--tarea->high_economy;
	gold -= 1000000000;
    }
    tarea->low_economy -= gold;
    while ( tarea->low_economy < 0 )
    {
	--tarea->high_economy;
	tarea->low_economy += 1000000000;
    }
}

/*
 * Check to see if economy has at least this much gold		   -Thoric
 */
bool economy_has( AREA_DATA *tarea, int gold )
{
    int hasgold = ((tarea->high_economy > 0) ? 1 : 0) * 1000000000
		+ tarea->low_economy;

    if ( hasgold >= gold )
	return TRUE;
    return FALSE;
}

/*
 * Used in db.c when resetting a mob into an area		    -Thoric
 * Makes sure mob doesn't get more than 10% of that area's gold,
 * and reduces area economy by the amount of gold given to the mob
 */
void economize_mobgold( CHAR_DATA *mob )
{
    int gold;
    AREA_DATA *tarea;

    /* make sure it isn't way too much */
    mob->gold = UMIN( mob->gold, mob->level * mob->level * 400 );
    if ( !mob->GetInRoom() )
	return;
    tarea = mob->GetInRoom()->area;

    gold = ((tarea->high_economy > 0) ? 1 : 0) * 1000000000 + tarea->low_economy;
    mob->gold = URANGE( 0, mob->gold, gold / 10 );
    if ( mob->gold )
	lower_economy( tarea, mob->gold );
}


/*
 * Add another notch on that there belt... ;)
 * Keep track of the last so many kills by vnum			-Thoric
 */
void add_kill( CHAR_DATA *ch, CHAR_DATA *mob )
{
    int x;
    int vnum;
    sh_int track;

    if ( IS_NPC(ch) )
    {
	bug( "add_kill: trying to add kill to npc", 0 );
	return;
    }
    if ( !IS_NPC(mob) )
    {
	bug( "add_kill: trying to add kill non-npc", 0 );
	return;
    }
    vnum = mob->pIndexData->vnum;
    track = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_HERO_MAX, MAX_KILLTRACK );
    for ( x = 0; x < track; x++ )
	if ( ch->pcdata->killed[x].vnum == vnum )
	{
	    if ( ch->pcdata->killed[x].count < 50 )
		++ch->pcdata->killed[x].count;
	    return;
	}
	else
	if ( ch->pcdata->killed[x].vnum == 0 )
	    break;
    memmove( (char *) ch->pcdata->killed+sizeof(KILLED_DATA),
		ch->pcdata->killed, (track-1) * sizeof(KILLED_DATA) );
    ch->pcdata->killed[0].vnum  = vnum;
    ch->pcdata->killed[0].count = 1;
    if ( track < MAX_KILLTRACK )
	ch->pcdata->killed[track].vnum = 0;
}

/*
 * Return how many times this player has killed this mob	-Thoric
 * Only keeps track of so many (MAX_KILLTRACK), and keeps track by vnum
 */
int times_killed( CHAR_DATA *ch, CHAR_DATA *mob )
{
    int x;
    int vnum;
    sh_int track;

    if ( IS_NPC(ch) )
    {
	bug( "times_killed: ch is not a player", 0 );
	return 0;
    }
    if ( !IS_NPC(mob) )
    {
	bug( "add_kill: mob is not a mobile", 0 );
	return 0;
    }

    vnum = mob->pIndexData->vnum;
    track = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_HERO_MAX, MAX_KILLTRACK );
    for ( x = 0; x < track; x++ )
	if ( ch->pcdata->killed[x].vnum == vnum )
	    return ch->pcdata->killed[x].count;
	else
	if ( ch->pcdata->killed[x].vnum == 0 )
	    break;
    return 0;
}

CHAR_DATA* get_obj_carried_by(OBJ_DATA* obj) {
    OBJ_DATA* pobj = obj;

    while ( pobj->GetInObj() )
        pobj = pobj->GetInObj();

    return pobj->GetCarriedBy();
}
