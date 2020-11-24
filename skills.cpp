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
 *			     Player skills module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "commands.h"
//#include "connection.h"


SkillType::SkillType()
{
	spell_fun = NULL;
	skill_fun = NULL;

	target = minimum_position = slot = min_mana = beats = 0;

	min_level = type = 0;
	flags = 0;

	hit_char = hit_vict = hit_room = miss_char = miss_vict
		= miss_room = die_char = die_vict = die_room
		= imm_char = imm_vict = imm_room = dice = NULL;

	value = 0;

	saves = difficulty = 0;

	affects = NULL;
	components = teachers = NULL;

	participants = 0;

	userec.num_uses = 0;

	struct timeval empty;
	empty.tv_sec = 0;
	empty.tv_usec = 0;
	userec.total_time = userec.min_time = userec.max_time = empty;

	Prerequisites = NULL;

	for ( int x = 0; x < MAX_CLASS; x++ )
	{
		skill_level[x] = LEVEL_IMMORTAL;
		skill_adept[x] = 95;
	}

	guild = -1;
}


const char * const spell_flag[] =
{ "water", "earth", "air", "astral", "area", "distant", "reverse",
"save_half_dam", "save_negates", "accumulative", "recastable", "noscribe",
"nobrew", "group", "object", "character", "secretskill", "persistant",
"stoponfail", "specialtrainers"};

const char * const spell_saves[] =
{ "none", "poison_death", "wands", "para_petri", "breath", "spell_staff" };

const char * const spell_damage[] =
{ "none", "fire", "cold", "electricity", "energy", "acid", "poison", "drain" };

const char * const spell_action[] =
{ "none", "create", "destroy", "resist", "suscept", "divinate", "obscure",
"change" };

const char * const spell_power[] =
{ "none", "minor", "greater", "major" };

const char * const spell_class[] =
{ "none", "lunar", "solar", "travel", "summon", "life", "death", "illusion" };

const char * const target_type[] =
{ "ignore", "offensive", "defensive", "self", "objinv" };


void show_char_to_char( CHAR_DATA *List, CHAR_DATA *ch );
void show_list_to_char( OBJ_DATA *List, CHAR_DATA *ch, bool fShort,
	bool fShowN );

int ris_save( CHAR_DATA *ch, int chance, int ris );

/* from magic.c */
void failed_casting( SkillType *skill, CHAR_DATA *ch,
		     CHAR_DATA *victim, OBJ_DATA *obj );


/*
 * Dummy function ** DUMMY
 */
void skill_notfound( CHAR_DATA *ch, const char *argument )
{
    send_to_char( "But you don't know this skill.\n\r", ch );
    return;
}


int get_ssave( const char *name )
{
	unsigned int x;

	for ( x = 0; x < sizeof(spell_saves) / sizeof(spell_saves[0]); x++ )
		if ( !str_cmp( name, spell_saves[x] ) )
			return x;
		return -1;
}

int get_starget( const char *name )
{
	unsigned int x;

	for ( x = 0; x < sizeof(target_type) / sizeof(target_type[0]); x++ )
		if ( !str_cmp( name, target_type[x] ) )
			return x;
		return -1;
}

int get_sflag( const char *name )
{
	unsigned int x;

	for ( x = 0; x < sizeof(spell_flag) / sizeof(spell_flag[0]); x++ )
		if ( !str_cmp( name, spell_flag[x] ) )
			return x;
		return -1;
}

int get_sdamage( const char *name )
{
	unsigned int x;

	for ( x = 0; x < sizeof(spell_damage) / sizeof(spell_damage[0]); x++ )
		if ( !str_cmp( name, spell_damage[x] ) )
			return x;
		return -1;
}

int get_saction( const char *name )
{
	unsigned int x;

	for ( x = 0; x < sizeof(spell_action) / sizeof(spell_action[0]); x++ )
		if ( !str_cmp( name, spell_action[x] ) )
			return x;
		return -1;
}

int get_spower( const char *name )
{
	unsigned int x;

	for ( x = 0; x < sizeof(spell_power) / sizeof(spell_power[0]); x++ )
		if ( !str_cmp( name, spell_power[x] ) )
			return x;
		return -1;
}

int get_sclass( const char *name )
{
	unsigned int x;

	for ( x = 0; x < sizeof(spell_class) / sizeof(spell_class[0]); x++ )
		if ( !str_cmp( name, spell_class[x] ) )
			return x;
		return -1;
}

extern const char *target_name;	/* from magic.c */

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
bool check_skill( CHAR_DATA *ch, const char *command, const char *argument )
{
	int sn;
	int first = gsn_first_skill;
	int top   = gsn_first_weapon-1;
	int mana, blood;
	struct timeval time_used;

	/* bsearch for the skill */
	for (;;)
	{
		sn = (first + top) >> 1;

		if ( LOWER(command[0]) == LOWER(skill_table[sn]->name_.c_str()[0])
				&&  !str_prefix(command, skill_table[sn]->name_.c_str())
				&&  (skill_table[sn]->skill_fun || skill_table[sn]->spell_fun != spell_null)
				&&  (IS_NPC(ch)
					||  (ch->pcdata->learned[sn] > 0
#if 0 /* Testaur - do allow forbidden skills/spells so long as they are learned */
						&&   ch->level >= skill_table[sn]->skill_level[ch->class]
#endif
						)) )
			break;
		if (first >= top)
			return FALSE;
		if (strcmp( command, skill_table[sn]->name_.c_str()) < 1)
			top = sn - 1;
		else
			first = sn + 1;
	}

	if ( !check_pos( ch, skill_table[sn]->minimum_position ) )
		return TRUE;

	if ( IS_NPC(ch)
			&&  (IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS )) )
	{
		send_to_char( "For some reason, you seem unable to perform that...\n\r", ch );
		act( AT_GREY,"$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM );
		return TRUE;
	}

	/* check if mana is required */
	if ( skill_table[sn]->min_mana )
	{
		mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
				100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->Class] ) );
		blood = UMAX(1, (mana+4) / 8);      /* NPCs don't have PCDatas. -- Altrag */
		if ( IS_VAMPIRE(ch) )
		{
			if (ch->pcdata->condition[COND_BLOODTHIRST] < blood)
			{
				send_to_char( "You don't have enough blood power.\n\r", ch );
				return TRUE;
			}
		}
		else
			if ( !IS_NPC(ch) && ch->mana < mana )
			{
				send_to_char( "You don't have enough mana.\n\r", ch );
				return TRUE;
			}
	}
	else
	{
		mana = 0;
		blood = 0;
	}

	/*
	 * Is this a real do-fun, or a really a spell?
	 */
	if ( !skill_table[sn]->skill_fun )
	{
		ch_ret retcode = rNONE;
		void *vo = NULL;
		CHAR_DATA *victim = NULL;
		OBJ_DATA *obj = NULL;

		target_name = "";

		switch ( skill_table[sn]->target )
		{
			default:
				bug( "Check_skill: bad target for sn %d.", sn );
				send_to_char( "Something went wrong...\n\r", ch );
				return TRUE;

			case TAR_IGNORE:
				vo = NULL;
				if ( argument[0] == '\0' )
				{
					if ( (victim=ch->GetVictim()) != NULL )
						target_name = victim->getName().c_str();
				}
				else
					target_name = (char *) argument;
				break;

			case TAR_CHAR_OFFENSIVE:
				if ( argument[0] == '\0'
						&&  (victim=ch->GetVictim()) == NULL )
				{
					ch_printf( ch, "%s who?\n\r", capitalize( skill_table[sn]->name_.c_str() ) );
					return TRUE;
				}
				else
					if ( argument[0] != '\0'
							&&  (victim=get_char_room(ch, (char *) argument)) == NULL )
					{
						send_to_char( "They aren't here.\n\r", ch );
						return TRUE;
					}
				if ( is_safe( ch, victim ) )
					return TRUE;
				vo = (void *) victim;
				break;

			case TAR_CHAR_DEFENSIVE:
				if ( argument[0] != '\0'
						&&  (victim=get_char_room(ch, (char *) argument)) == NULL )
				{
					send_to_char( "They aren't here.\n\r", ch );
					return TRUE;
				}
				if ( !victim )
					victim = ch;
				vo = (void *) victim;
				break;

			case TAR_CHAR_SELF:
				vo = (void *) ch;
				break;

			case TAR_OBJ_INV:
				if ( (obj=get_obj_carry(ch, (char *) argument)) == NULL )
				{
					send_to_char( "You can't find that.\n\r", ch );
					return TRUE;
				}
				vo = (void *) obj;
				break;
		}

		/* waitstate */
		ch->AddWait( skill_table[sn]->beats );
		/* check for failure */
		if ( (number_percent( ) + skill_table[sn]->difficulty * 5)
				> (IS_NPC(ch) ? 75 : ch->pcdata->learned[sn]) )
		{
			failed_casting( skill_table[sn], ch, (CHAR_DATA *) vo, obj );
			learn_from_failure( ch, sn );
			if ( mana )
			{
				if ( IS_VAMPIRE(ch) )
					gain_condition( ch, COND_BLOODTHIRST, - blood/2 );
				else
					ch->mana -= mana/2;
			}
			return TRUE;
		}
		if ( mana )
		{
			if ( IS_VAMPIRE(ch) )
				gain_condition( ch, COND_BLOODTHIRST, - blood );
			else
				ch->mana -= mana;
		}
		start_timer(&time_used);
		retcode = (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, vo );
		end_timer(&time_used);
		update_userec(&time_used, &skill_table[sn]->userec);

		if ( retcode == rCHAR_DIED || retcode == rERROR )
			return TRUE;

		if ( char_died(ch) )
			return TRUE;

		if ( retcode == rSPELL_FAILED )
		{
			learn_from_failure( ch, sn );
			retcode = rNONE;
		}
		else
			learn_from_success( ch, sn );

		if ( skill_table[sn]->target == TAR_CHAR_OFFENSIVE
				&&   victim != ch
				&&  !char_died(victim) )
		{
			CHAR_DATA *vch;
			CHAR_DATA *vch_next;

			for ( vch = ch->GetInRoom()->first_person; vch; vch = vch_next )
			{
				vch_next = vch->next_in_room;
				if ( victim == vch && !victim->IsAttacking() && victim->GetMaster() != ch )
				{
					retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
					break;
				}
			}
		}
		return TRUE;
	}

	if ( mana )
	{
		if ( IS_VAMPIRE(ch) )
			gain_condition( ch, COND_BLOODTHIRST, - blood );
		else
			ch->mana -= mana;
	}
	ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
	ch->last_cmd = skill_table[sn]->skill_fun;
	start_timer(&time_used);
	(*skill_table[sn]->skill_fun) ( ch, (char *) argument );
	end_timer(&time_used);
	update_userec(&time_used, &skill_table[sn]->userec);

	tail_chain( );
	return TRUE;
}

/*
 * Lookup a skills information
 * High god command
 */
void do_slookup(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int sn;
	int iClass;
	SkillType *skill = NULL;

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Slookup what?\n\r", ch );
		return;
	}

	if ( !str_cmp( arg, "all" ) )
	{
		for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name_.length() > 0; sn++ )
			pager_printf( ch, "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r",
					sn, skill_table[sn]->slot, skill_table[sn]->name_.c_str(),
					spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
	}
	else
		if ( !str_cmp( arg, "herbs" ) )
		{
			for ( sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name_.c_str(); sn++ )
				pager_printf( ch, "%d) %s\n\r", sn, herb_table[sn]->name_.c_str() );
		}
		else
		{
			SMAUG_AFF *aff;
			int cnt = 0;

			if ( arg[0] == 'h' && is_number(arg+1) )
			{
				sn = atoi(arg+1);
				if ( !IS_VALID_HERB(sn) )
				{
					send_to_char( "Invalid herb.\n\r", ch );
					return;
				}

				skill = herb_table[sn];
			}
			else
				if ( is_number(arg) )
				{
					sn = atoi(arg);
					if ( (skill=get_skilltype(sn)) == NULL )
					{
						send_to_char( "Invalid sn.\n\r", ch );
						return;
					}
					sn %= 1000;
				}
				else
					if ( ( sn = skill_lookup( arg ) ) >= 0 )
						skill = skill_table[sn];
					else
						if ( ( sn = herb_lookup( arg ) ) >= 0 )
							skill = herb_table[sn];
						else
						{
							send_to_char( "No such skill, spell, proficiency or tongue.\n\r", ch );
							return;
						}
			if ( !skill )
			{
				send_to_char( "Not created yet.\n\r", ch );
				return;
			}

			ch_printf( ch, "Sn: %4d Slot: %4d %s: '%-20s'\n\r",
					sn, skill->slot, skill_tname[skill->type], skill->name_.c_str() );
			if ( skill->flags )
			{
				int x;

				ch_printf( ch, "Damtype: %s  Acttype: %s   Classtype: %s   Powertype: %s\n\r",
						spell_damage[SPELL_DAMAGE( skill )],
						spell_action[SPELL_ACTION( skill )],
						spell_class[SPELL_CLASS( skill )],
						spell_power[SPELL_POWER( skill )] );
				strcpy( buf, "Flags:" );
				for ( x = 11; x < 32; x++ )
					if ( SPELL_FLAG( skill, 1 << x ) )
					{
						strcat( buf, " " );
						strcat( buf, spell_flag[x-11] );
					}
				strcat( buf, "\n\r" );
				send_to_char( buf, ch );
			}
			ch_printf( ch, "Saves: %s\n\r", spell_saves[(int) skill->saves] );

			if ( skill->difficulty != '\0' )
				ch_printf( ch, "Difficulty: %d\n\r", (int) skill->difficulty );

			ch_printf( ch, "Type: %s  Target: %s  Minpos: %d  Mana: %d  Beats: %d\n\r",
					skill_tname[skill->type],
					target_type[URANGE(TAR_IGNORE, skill->target, TAR_OBJ_INV)],
					skill->minimum_position,
					skill->min_mana,
					skill->beats );
			ch_printf( ch, "Value: %d  Guild: %d  Code: %s\n\r",
					skill->value,
					skill->guild,
					skill->skill_fun ? skill_name(skill->skill_fun)
					: spell_name(skill->spell_fun));
			if ( skill->Prerequisites && skill->Prerequisites[0] != '\0')
				ch_printf( ch, "Prereqs: %s\n\r", skill->Prerequisites);
			ch_printf( ch, "Dammsg: %s\n\rWearoff: %s\n",
					skill->nounDamage_.c_str(),
					skill->msgOff_.length() > 0 ? skill->msgOff_.c_str() : "(none set)" );
			if ( skill->dice && skill->dice[0] != '\0' )
				ch_printf( ch, "Dice: %s\n\r", skill->dice );
			if ( skill->teachers && skill->teachers[0] != '\0' )
				ch_printf( ch, "Teachers: %s\n\r", skill->teachers );
			if ( skill->components && skill->components[0] != '\0' )
				ch_printf( ch, "Components: %s\n\r", skill->components );
			if ( skill->participants )
				ch_printf( ch, "Participants: %d\n\r", (int) skill->participants );
			if ( skill->userec.num_uses )
				send_timer(&skill->userec, ch);
			for ( aff = skill->affects; aff; aff = aff->next )
			{
				if ( aff == skill->affects )
					send_to_char( "\n\r", ch );
				sprintf( buf, "Affect %d", ++cnt );
				if ( aff->location )
				{
					strcat( buf, " modifies " );
					strcat( buf, a_types[aff->location % REVERSE_APPLY] );
					strcat( buf, " by '" );
					strcat( buf, aff->modifier );
					if ( aff->bitvector )
						strcat( buf, "' and" );
					else
						strcat( buf, "'" );
				}
				if ( aff->bitvector )
				{
					int x;

					strcat( buf, " applies" );
					for ( x = 0; x < 32; x++ )
						if ( IS_SET(aff->bitvector, 1 << x) )
						{
							strcat( buf, " " );
							strcat( buf, a_flags[x] );
						}
				}
				if ( aff->duration[0] != '\0' && aff->duration[0] != '0' )
				{
					strcat( buf, " for '" );
					strcat( buf, aff->duration );
					strcat( buf, "' rounds" );
				}
				if ( aff->location >= REVERSE_APPLY )
					strcat( buf, " (affects caster only)" );
				strcat( buf, "\n\r" );
				send_to_char( buf, ch );
				if ( !aff->next )
					send_to_char( "\n\r", ch );
			}
			if ( skill->hit_char && skill->hit_char[0] != '\0' )
				ch_printf( ch, "Hitchar   : %s\n\r", skill->hit_char );
			if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
				ch_printf( ch, "Hitvict   : %s\n\r", skill->hit_vict );
			if ( skill->hit_room && skill->hit_room[0] != '\0' )
				ch_printf( ch, "Hitroom   : %s\n\r", skill->hit_room );
			if ( skill->miss_char && skill->miss_char[0] != '\0' )
				ch_printf( ch, "Misschar  : %s\n\r", skill->miss_char );
			if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
				ch_printf( ch, "Missvict  : %s\n\r", skill->miss_vict );
			if ( skill->miss_room && skill->miss_room[0] != '\0' )
				ch_printf( ch, "Missroom  : %s\n\r", skill->miss_room );
			if ( skill->die_char && skill->die_char[0] != '\0' )
				ch_printf( ch, "Diechar   : %s\n\r", skill->die_char );
			if ( skill->die_vict && skill->die_vict[0] != '\0' )
				ch_printf( ch, "Dievict   : %s\n\r", skill->die_vict );
			if ( skill->die_room && skill->die_room[0] != '\0' )
				ch_printf( ch, "Dieroom   : %s\n\r", skill->die_room );
			if ( skill->imm_char && skill->imm_char[0] != '\0' )
				ch_printf( ch, "Immchar   : %s\n\r", skill->imm_char );
			if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
				ch_printf( ch, "Immvict   : %s\n\r", skill->imm_vict );
			if ( skill->imm_room && skill->imm_room[0] != '\0' )
				ch_printf( ch, "Immroom   : %s\n\r", skill->imm_room );
			if ( skill->type != SKILL_HERB )
			{
				for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
				{
					strcpy( buf, class_table[iClass]->whoName_.c_str() );
					sprintf(buf+3, ") lvl: %3d max: %2d%%",
							skill->skill_level[iClass],
							skill->skill_adept[iClass] );
					if ( iClass % 3 == 2 )
						strcat(buf, "\n\r" );
					else
						strcat(buf, "  " );
					send_to_char( buf, ch );
				}
			}
			send_to_char( "\n\r", ch );
		}

	return;
}

/*
 * Set a skill's attributes or what skills a player has.
 * High god command, with support for creating skills/spells/herbs/etc
 */
void do_sset(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;
	int sn;
	bool fAll;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Syntax: sset <victim> <skill> <value>\n\r",	ch );
		send_to_char( "or:     sset <victim> all     <value>\n\r",	ch );
		send_to_char( "or:     sset save skill table\n\r",		ch );
		send_to_char( "or:     sset save herb table\n\r",		ch );
		send_to_char( "or:     sset create skill 'new skill'\n\r",	ch );
		send_to_char( "or:     sset create herb 'new herb'\n\r",	ch );
		send_to_char( "or:     sset <sn>     <field> <value>\n\r",	ch );
		send_to_char( "\n\rField being one of:\n\r",			ch );
		send_to_char( "  name code target minpos slot mana beats dammsg wearoff guild minlevel\n\r", ch );
		send_to_char( "  type damtype acttype classtype powertype flag dice value difficulty affect\n\r", ch );
		send_to_char( "  rmaffect level adept hit miss die imm (char/vict/room)\n\r", ch );
		send_to_char( "  components teachers\n\r",			ch );
		send_to_char( "Affect having the fields: <location> <modfifier> [duration] [bitvector]\n\r", ch );
		send_to_char( "(See AFFECTTYPES for location, and AFFECTED_BY for bitvector)\n\r", ch );
		send_to_char( "Skill being any skill or spell.\n\r",		ch );
		return;
	}

	if (
			!str_cmp( arg1, "save" )
			&&	!str_cmp( argument, "table" ) )
	{
		if ( !str_cmp( arg2, "skill" ) )
		{
			send_to_char( "Saving skill table...\n\r", ch );
			save_skill_table();
			save_classes();
			return;
		}
		if ( !str_cmp( arg2, "herb" ) )
		{
			send_to_char( "Saving herb table...\n\r", ch );
			save_herb_table();
			return;
		}
	}
	if (
			!str_cmp( arg1, "create" )
			&& (!str_cmp( arg2, "skill" ) || !str_cmp( arg2, "herb" )) )
	{
		SkillType * skill;
		sh_int type = SKILL_UNKNOWN;

		if ( !str_cmp( arg2, "herb" ) )
		{
			type = SKILL_HERB;
			if ( top_herb >= MAX_HERB )
			{
				ch_printf( ch, "The current top herb is %d, which is the maximum.  "
						"To add more herbs,\n\rMAX_HERB will have to be "
						"raised in mud.h, and the mud recompiled.\n\r",
						top_sn );
				return;
			}
		}
		else
			if ( top_sn >= MAX_SKILL )
			{
				ch_printf( ch, "The current top sn is %d, which is the maximum.  "
						"To add more skills,\n\rMAX_SKILL will have to be "
						"raised in mud.h, and the mud recompiled.\n\r",
						top_sn );
				return;
			}

		skill = new SkillType;

		if ( type == SKILL_HERB )
		{
			int max, x;

			herb_table[top_herb++] = skill;
			for ( max = x = 0; x < top_herb-1; x++ )
				if ( herb_table[x] && herb_table[x]->slot > max )
					max = herb_table[x]->slot;
			skill->slot = max+1;
		}
		else
			skill_table[top_sn++] = skill;
		skill->name_ = argument;
		skill->nounDamage_ = "";
		skill->msgOff_ = "";
		skill->spell_fun = spell_smaug;
		skill->type = type;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( arg1[0] == 'h' )
		sn = atoi( arg1+1 );
	else
		sn = atoi( arg1 );
	if (
			((arg1[0] == 'h' && is_number(arg1+1) && (sn=atoi(arg1+1))>=0)
			 ||  (is_number(arg1) && (sn=atoi(arg1)) >= 0)) )
	{
		SkillType *skill;

		if ( arg1[0] == 'h' )
		{
			if ( sn >= top_herb )
			{
				send_to_char( "Herb number out of range.\n\r", ch );
				return;
			}
			skill = herb_table[sn];
		}
		else
		{
			if ( (skill=get_skilltype(sn)) == NULL )
			{
				send_to_char( "Skill number out of range.\n\r", ch );
				return;
			}
			sn %= 1000;
		}

		if ( !str_cmp( arg2, "difficulty" ) )
		{
			skill->difficulty = atoi( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "participants" ) )
		{
			skill->participants = atoi( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "prereqs" ) || !str_cmp( arg2, "prerequisites" ) )
		{
			if ( skill->Prerequisites )
				DISPOSE(skill->Prerequisites);
			if ( !str_cmp( argument, "clear" ) )
				skill->Prerequisites = str_dup( "" );
			else
				skill->Prerequisites = str_dup( argument );
			ch_printf(ch, "Prequisites set to %s.\n\r", argument);
			return;
		}
		if ( !str_cmp( arg2, "damtype" ) )
		{
			int x = get_sdamage( argument );

			if ( x == -1 )
				send_to_char( "Not a spell damage type.\n\r", ch );
			else
			{
				SET_SDAM( skill, x );
				send_to_char( "Ok.\n\r", ch );
			}
			return;
		}
		if ( !str_cmp( arg2, "acttype" ) )
		{
			int x = get_saction( argument );

			if ( x == -1 )
				send_to_char( "Not a spell action type.\n\r", ch );
			else
			{
				SET_SACT( skill, x );
				send_to_char( "Ok.\n\r", ch );
			}
			return;
		}
		if ( !str_cmp( arg2, "classtype" ) )
		{
			int x = get_sclass( argument );

			if ( x == -1 )
				send_to_char( "Not a spell class type.\n\r", ch );
			else
			{
				SET_SCLA( skill, x );
				send_to_char( "Ok.\n\r", ch );
			}
			return;
		}
		if ( !str_cmp( arg2, "powertype" ) )
		{
			int x = get_spower( argument );

			if ( x == -1 )
				send_to_char( "Not a spell power type.\n\r", ch );
			else
			{
				SET_SPOW( skill, x );
				send_to_char( "Ok.\n\r", ch );
			}
			return;
		}
		if ( !str_cmp( arg2, "flag" ) )
		{
			int x = get_sflag( argument );

			if ( x == -1 )
				send_to_char( "Not a spell flag.\n\r", ch );
			else
			{
				TOGGLE_BIT( skill->flags, 1 << (x+11) );
				send_to_char( "Ok.\n\r", ch );
			}
			return;
		}
		if ( !str_cmp( arg2, "saves" ) )
		{
			int x = get_ssave( argument );

			if ( x == -1 )
				send_to_char( "Not a saving type.\n\r", ch );
			else
			{
				skill->saves = x;
				send_to_char( "Ok.\n\r", ch );
			}
			return;
		}

		if ( !str_cmp( arg2, "code" ) )
		{
			SPELL_FUN *spellfun;
			DO_FUN    *dofun;

			if ( (spellfun=spell_function(argument)) != spell_notfound )
			{
				skill->spell_fun = spellfun;
				skill->skill_fun = NULL;
			}
			else
				if ( (dofun=skill_function(argument)) != skill_notfound )
				{
					skill->skill_fun = dofun;
					skill->spell_fun = NULL;
				}
				else
				{
					send_to_char( "Not a spell or skill.\n\r", ch );
					return;
				}
			send_to_char( "Ok.\n\r", ch );
			return;
		}

		if ( !str_cmp( arg2, "target" ) )
		{
			int x = get_starget( argument );

			if ( x == -1 )
				send_to_char( "Not a valid target type.\n\r", ch );
			else
			{
				skill->target = x;
				send_to_char( "Ok.\n\r", ch );
			}
			return;
		}
		if ( !str_cmp( arg2, "minpos" ) )
		{
			skill->minimum_position = URANGE( POS_DEAD, atoi( argument ), POS_DRAG );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "minlevel" ) )
		{
			skill->min_level = URANGE( 1, atoi( argument ), MAX_LEVEL );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "slot" ) )
		{
			skill->slot = URANGE( 0, atoi( argument ), 30000 );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "mana" ) )
		{
			skill->min_mana = URANGE( 0, atoi( argument ), 2000 );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "beats" ) )
		{
			skill->beats = URANGE( 0, atoi( argument ), 120 );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "guild" ) )
		{
			skill->guild = atoi( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "value" ) )
		{
			skill->value = atoi( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "type" ) )
		{
			skill->type = get_skill( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "rmaffect" ) )
		{
			SMAUG_AFF *aff = skill->affects;
			SMAUG_AFF *aff_next;
			int num = atoi( argument );
			int cnt = 1;

			if ( !aff )
			{
				send_to_char( "This spell has no special affects to remove.\n\r", ch );
				return;
			}
			if ( num == 1 )
			{
				skill->affects = aff->next;
				DISPOSE( aff->duration );
				DISPOSE( aff->modifier );
				DISPOSE( aff );
				send_to_char( "Removed.\n\r", ch );
				return;
			}
			for ( ; aff; aff = aff->next )
			{
				if ( ++cnt == num && (aff_next=aff->next) != NULL )
				{
					aff->next = aff_next->next;
					DISPOSE( aff_next->duration );
					DISPOSE( aff_next->modifier );
					DISPOSE( aff_next );
					send_to_char( "Removed.\n\r", ch );
					return;
				}
			}
			send_to_char( "Not found.\n\r", ch );
			return;
		}
		/*
		 * affect <location> <modifier> <duration> <bitvector>
		 */
		if ( !str_cmp( arg2, "affect" ) )
		{
			char location[MAX_INPUT_LENGTH];
			char modifier[MAX_INPUT_LENGTH];
			char duration[MAX_INPUT_LENGTH];
			char bitvector[MAX_INPUT_LENGTH];
			int loc, bit, tmpbit;
			SMAUG_AFF *aff;

			argument = one_argument( argument, location );
			argument = one_argument( argument, modifier );
			argument = one_argument( argument, duration );

			if ( location[0] == '!' )
				loc = get_atype( location+1 ) + REVERSE_APPLY;
			else
				loc = get_atype( location );
			if ( (loc % REVERSE_APPLY) < 0
					||   (loc % REVERSE_APPLY) >= MAX_APPLY_TYPE )
			{
				send_to_char( "Unknown affect location.  See AFFECTTYPES.\n\r", ch );
				return;
			}
			bit = 0;
			while ( argument[0] != 0 )
			{
				argument = one_argument( argument, bitvector );
				if ( (tmpbit=get_aflag( bitvector )) == -1 )
					ch_printf( ch, "Unknown bitvector: %s.  See AFFECTED_BY\n\r", bitvector );
				else
					bit |= (1 << tmpbit);
			}
			CREATE( aff, SMAUG_AFF, 1 );
			if ( !str_cmp( duration, "0" ) )
				duration[0] = '\0';
			if ( !str_cmp( modifier, "0" ) )
				modifier[0] = '\0';
			aff->duration = str_dup( duration );
			aff->location = loc;
			aff->modifier = str_dup( modifier );
			aff->bitvector = bit;
			aff->next = skill->affects;
			skill->affects = aff;
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "level" ) )
		{
			char arg3[MAX_INPUT_LENGTH];
			int Class;

			argument = one_argument( argument, arg3 );
			Class = atoi( arg3 );
			if ( Class >= MAX_CLASS || Class < 0 )
				send_to_char( "Not a valid class.\n\r", ch );
			else
				skill->skill_level[Class] =
					URANGE(0, atoi(argument), MAX_LEVEL);
			return;
		}
		if ( !str_cmp( arg2, "adept" ) )
		{
			char arg3[MAX_INPUT_LENGTH];
			int Class;

			argument = one_argument( argument, arg3 );
			Class = atoi( arg3 );
			if ( Class >= MAX_CLASS || Class < 0 )
				send_to_char( "Not a valid class.\n\r", ch );
			else
				skill->skill_adept[Class] =
					URANGE(0, atoi(argument), 100);
			return;
		}
		if ( !str_cmp( arg2, "name" ) )
		{
			skill->name_ = argument;
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "dammsg" ) )
		{
			if ( !str_cmp( argument, "clear" ) )
				skill->nounDamage_ = "";
			else
				skill->nounDamage_ = argument;
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "wearoff" ) )
		{
			if ( !str_cmp( argument, "clear" ) )
				skill->msgOff_ = "";
			else
				skill->msgOff_ = argument;

			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "hitchar" ) )
		{
			if ( skill->hit_char )
				DISPOSE(skill->hit_char);
			if ( str_cmp( argument, "clear" ) )
				skill->hit_char = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "hitvict" ) )
		{
			if ( skill->hit_vict )
				DISPOSE(skill->hit_vict);
			if ( str_cmp( argument, "clear" ) )
				skill->hit_vict = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "hitroom" ) )
		{
			if ( skill->hit_room )
				DISPOSE(skill->hit_room);
			if ( str_cmp( argument, "clear" ) )
				skill->hit_room = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "misschar" ) )
		{
			if ( skill->miss_char )
				DISPOSE(skill->miss_char);
			if ( str_cmp( argument, "clear" ) )
				skill->miss_char = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "missvict" ) )
		{
			if ( skill->miss_vict )
				DISPOSE(skill->miss_vict);
			if ( str_cmp( argument, "clear" ) )
				skill->miss_vict = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "missroom" ) )
		{
			if ( skill->miss_room )
				DISPOSE(skill->miss_room);
			if ( str_cmp( argument, "clear" ) )
				skill->miss_room = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "diechar" ) )
		{
			if ( skill->die_char )
				DISPOSE(skill->die_char);
			if ( str_cmp( argument, "clear" ) )
				skill->die_char = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "dievict" ) )
		{
			if ( skill->die_vict )
				DISPOSE(skill->die_vict);
			if ( str_cmp( argument, "clear" ) )
				skill->die_vict = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "dieroom" ) )
		{
			if ( skill->die_room )
				DISPOSE(skill->die_room);
			if ( str_cmp( argument, "clear" ) )
				skill->die_room = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "immchar" ) )
		{
			if ( skill->imm_char )
				DISPOSE(skill->imm_char);
			if ( str_cmp( argument, "clear" ) )
				skill->imm_char = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "immvict" ) )
		{
			if ( skill->imm_vict )
				DISPOSE(skill->imm_vict);
			if ( str_cmp( argument, "clear" ) )
				skill->imm_vict = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "immroom" ) )
		{
			if ( skill->imm_room )
				DISPOSE(skill->imm_room);
			if ( str_cmp( argument, "clear" ) )
				skill->imm_room = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "dice" ) )
		{
			if ( skill->dice )
				DISPOSE(skill->dice);
			if ( str_cmp( argument, "clear" ) )
				skill->dice = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "components" ) )
		{
			if ( skill->components )
				DISPOSE(skill->components);
			if ( str_cmp( argument, "clear" ) )
				skill->components = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		if ( !str_cmp( arg2, "teachers" ) )
		{
			if ( skill->teachers )
				DISPOSE(skill->teachers);
			if ( str_cmp( argument, "clear" ) )
				skill->teachers = str_dup( argument );
			send_to_char( "Ok.\n\r", ch );
			return;
		}
		do_sset( ch, "" );
		return;
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		if ( (sn = skill_lookup(arg1)) >= 0 )
		{
			sprintf(arg1, "%d %s %s", sn, arg2, argument);
			do_sset(ch, arg1);
		}
		else
			send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_NPC(victim) )
	{
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	}

	fAll = !str_cmp( arg2, "all" );
	sn   = 0;
	if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
	{
		send_to_char( "No such skill or spell.\n\r", ch );
		return;
	}

	/*
	 * Snarf the value.
	 */
	if ( !is_number( argument ) )
	{
		send_to_char( "Value must be numeric.\n\r", ch );
		return;
	}

	value = atoi( argument );
	if ( value < 0 || value > 100 )
	{
		send_to_char( "Value range is 0 to 100.\n\r", ch );
		return;
	}

	if ( fAll )
	{
		for ( sn = 0; sn < top_sn; sn++ )
		{
			/* Fix by Narn to prevent ssetting skills the player shouldn't have. */
			if ( skill_table[sn]->name_.length() > 0
					&& ( victim->level >= skill_table[sn]->skill_level[victim->Class]
						|| value == 0 ) )
				victim->pcdata->learned[sn] = value;
		}
	}
	else
		victim->pcdata->learned[sn] = value;

	return;
}


void learn_from_success( CHAR_DATA *ch, int sn )
{
	int adept, gain, sklvl, learn, percent, chance;

	if ( IS_NPC(ch) || ch->pcdata->learned[sn] == 0 )
		return;
	if (ch->pcdata->area && get_trust(ch)<LEVEL_IMMORTAL)
	{/* player builder cannot earn xp in own area */
		if(  ch->GetInRoom()->vnum >= ch->pcdata->area->low_r_vnum
				&& ch->GetInRoom()->vnum <= ch->pcdata->area->hi_r_vnum)
			return;
	}
	adept = GET_ADEPT(ch,sn);
	sklvl = skill_table[sn]->skill_level[ch->Class];
	if ( sklvl == 0 )
		sklvl = ch->level;
	if ( ch->pcdata->learned[sn] < adept )
	{
		chance = ch->pcdata->learned[sn] + (5 * skill_table[sn]->difficulty);
		percent = number_percent();
		if ( percent >= chance )
			learn = 2;
		else
			if ( chance - percent > 25 )
				return;
			else
				learn = 1;
		ch->pcdata->learned[sn] = UMIN( adept, ch->pcdata->learned[sn] + learn );
		if ( ch->pcdata->learned[sn] == adept )	 /* fully learned! */
		{
			gain = 1000 * sklvl;
			if(ch->Class==0) gain = gain *5;  /* h, mage upgrade */
			set_char_color( AT_WHITE, ch );
			ch_printf( ch, "You are now an adept of %s!  You gain %d bonus experience!\n\r",
					skill_table[sn]->name_.c_str(), gain );
		}
		else
		{
			gain = 20 * sklvl;
			if(ch->Class==0) gain = gain *6;  /* h, mage upgrade */
			if ( !ch->IsFighting() && sn != gsn_hide && sn != gsn_sneak )
			{
				set_char_color( AT_WHITE, ch );
				ch_printf( ch, "You gain %d experience points from your success!\n\r", gain );
			}
		}
		gain_exp( ch, gain, FALSE);
	}
}


void learn_from_failure( CHAR_DATA *ch, int sn )
{
	int adept, chance;

	if ( IS_NPC(ch) || ch->pcdata->learned[sn] == 0 )
		return;

	if (ch->pcdata->area && get_trust(ch)<LEVEL_IMMORTAL)
	{/* player builder cannot earn xp in own area */
		if(  ch->GetInRoom()->vnum >= ch->pcdata->area->low_r_vnum
			&& ch->GetInRoom()->vnum <= ch->pcdata->area->hi_r_vnum)
			return;
	}


	chance = ch->pcdata->learned[sn] + (5 * skill_table[sn]->difficulty);
	if ( chance - number_percent() > 25 )
		return;
	adept = GET_ADEPT(ch, sn);
	if ( ch->pcdata->learned[sn] < (adept-1) )
		ch->pcdata->learned[sn] = UMIN( adept, ch->pcdata->learned[sn] + 1 );
}


void do_gouge(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	sh_int dam;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_gouge] )
	{
		send_to_char("You do not yet know of this skill.\n\r", ch );
		return;
	}

	if ( ch->GetMount() )
	{
		send_to_char( "You can't get close enough while mounted.\n\r", ch );
		return;
	}

	if ( ( victim = ch->GetVictim() ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	if ( ch->ChanceRollSkill(gsn_gouge) )
	{
		dam = number_range( 1, ch->level );
		global_retcode = damage( ch, victim, dam, gsn_gouge, 0 );
		if ( global_retcode == rNONE )
		{
			if ( !IS_AFFECTED( victim, AFF_BLIND ) )
			{
				af.type      = gsn_blindness;
				af.location  = APPLY_HITROLL;
				af.modifier  = -6;
				af.duration  = 3 + (ch->level / 15);
				af.bitvector = AFF_BLIND;
				affect_to_char( victim, &af );
				act( AT_SKILL, "You can't see a thing!", victim, NULL, NULL, TO_CHAR );
			}
			ch->AddWait(     PULSE_VIOLENCE );
			victim->AddWait( PULSE_VIOLENCE );
			/* Taken out by request - put back in by Thoric
			 * This is how it was designed.  You'd be a tad stunned
			 * if someone gouged you in the eye.
			 */
		}
		else
		if ( global_retcode == rVICT_DIED )
		{
			act( AT_BLOOD, "Your fingers plunge into your victim's brain, causing immediate death!",
			ch, NULL, NULL, TO_CHAR );
		}
		if ( global_retcode != rCHAR_DIED && global_retcode != rBOTH_DIED )
			learn_from_success( ch, gsn_gouge );
	}
	else
	{
		ch->AddWait( skill_table[gsn_gouge]->beats );
		global_retcode = damage( ch, victim, 0, gsn_gouge, 0 );
		learn_from_failure( ch, gsn_gouge );
	}

	return;
}

void do_detrap(CHAR_DATA *ch, const char* argument)
{
	char arg  [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *trap;
	bool found;

	switch( ch->substate )
	{
		default:
			if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
			{
				send_to_char( "You can't concentrate enough for that.\n\r", ch );
				return;
			}
			argument = one_argument( argument, arg );
			if ( !IS_NPC(ch) && !ch->pcdata->learned[gsn_detrap] )
			{
				send_to_char("You do not yet know of this skill.\n\r", ch );
				return;
			}
			if ( arg[0] == '\0' )
			{
				send_to_char( "Detrap what?\n\r", ch );
				return;
			}
			if ( ms_find_obj(ch) )
				return;
			found = FALSE;
			if ( ch->GetMount() )
			{
				send_to_char( "You can't do that while mounted.\n\r", ch );
				return;
			}
			if ( !ch->GetInRoom()->first_content )
			{
				send_to_char( "You can't find that here.\n\r", ch );
				return;
			}
			for ( obj = ch->GetInRoom()->first_content; obj; obj = obj->next_content )
			{
				if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name_.c_str() ) )
				{
					found = TRUE;
					break;
				}
			}
			if ( !found )
			{
				send_to_char( "You can't find that here.\n\r", ch );
				return;
			}
			act( AT_ACTION, "You carefully begin your attempt to remove a trap from $p...", ch, obj, NULL, TO_CHAR );
			act( AT_ACTION, "$n carefully attempts to remove a trap from $p...", ch, obj, NULL, TO_ROOM );
			ch->dest_buf = str_dup( obj->name_.c_str() );
			add_timer( ch, TIMER_DO_FUN, 3, do_detrap, 1 );
			/*	    WAIT_STATE( ch, skill_table[gsn_detrap]->beats ); */
			return;
		case 1:
			if ( !ch->dest_buf )
			{
				send_to_char( "Your detrapping was interrupted!\n\r", ch );
				bug( "do_detrap: ch->dest_buf NULL!", 0 );
				return;
			}
			strcpy( arg, (char *) ch->dest_buf );
			DISPOSE( ch->dest_buf );
			ch->dest_buf = NULL;
			ch->substate = SUB_NONE;
			break;
		case SUB_TIMER_DO_ABORT:
			DISPOSE(ch->dest_buf);
			ch->substate = SUB_NONE;
			send_to_char( "You carefully stop what you were doing.\n\r", ch );
			return;
	}

	if ( !ch->GetInRoom()->first_content )
	{
		send_to_char( "You can't find that here.\n\r", ch );
		return;
	}
	for ( obj = ch->GetInRoom()->first_content; obj; obj = obj->next_content )
	{
		if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name_.c_str() ) )
		{
			found = TRUE;
			break;
		}
	}
	if ( !found )
	{
		send_to_char( "You can't find that here.\n\r", ch );
		return;
	}
	if ( (trap = get_trap( obj )) == NULL )
	{
		send_to_char( "You find no trap on that.\n\r", ch );
		return;
	}

	separate_obj(obj);
	if ( !ch->ChanceRollSkill(gsn_detrap, -(ch->level/15) ) )
	{
		send_to_char( "Ooops!\n\r", ch );
		spring_trap( ch, trap );
		learn_from_failure( ch, gsn_detrap );
		return;
	}

	extract_obj( trap, TRUE );

	send_to_char( "You successfully remove a trap.\n\r", ch );
	learn_from_success( ch, gsn_detrap );
	return;
}

void do_dig(CHAR_DATA *ch, const char* argument)
{
	char arg [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *startobj;
	bool found, shovel;
	ExitData *pexit;

	switch( ch->substate )
	{
		default:
			if ( IS_NPC(ch)  && IS_AFFECTED( ch, AFF_CHARM ) )
			{
				send_to_char( "You can't concentrate enough for that.\n\r", ch );
				return;
			}
			if ( ch->GetMount() )
			{
				send_to_char( "You can't do that while mounted.\n\r", ch );
				return;
			}
			one_argument( argument, arg );
			if ( arg[0] != '\0' )
			{
				if ( ( pexit = find_door( ch, arg, TRUE ) ) == NULL
						&&     get_dir(arg) == -1 )
				{
					send_to_char( "What direction is that?\n\r", ch );
					return;
				}
				if ( pexit )
				{
					if ( !IS_SET(pexit->exit_info, EX_DIG)
							&&   !IS_SET(pexit->exit_info, EX_CLOSED) )
					{
						send_to_char( "There is no need to dig out that exit.\n\r", ch );
						return;
					}
				}
			}
			else
			{
				switch( ch->GetInRoom()->sector_type )
				{
					case SECT_CITY:
					case SECT_INSIDE:
						send_to_char( "The floor is too hard to dig through.\n\r", ch );
						return;
					case SECT_WATER_SWIM:
					case SECT_WATER_NOSWIM:
					case SECT_UNDERWATER:
						send_to_char( "You cannot dig here.\n\r", ch );
						return;
					case SECT_AIR:
						send_to_char( "What?  In the air?!\n\r", ch );
						return;
				}
			}
			add_timer( ch, TIMER_DO_FUN, UMIN(skill_table[gsn_dig]->beats / 10, 3),
					do_dig, 1);
			ch->dest_buf = str_dup( arg );
			send_to_char( "You begin digging...\n\r", ch );
			act( AT_PLAIN, "$n begins digging...", ch, NULL, NULL, TO_ROOM );
			return;

		case 1:
			if ( !ch->dest_buf )
			{
				send_to_char( "Your digging was interrupted!\n\r", ch );
				act( AT_PLAIN, "$n's digging was interrupted!", ch, NULL, NULL, TO_ROOM );
				bug( "do_dig: dest_buf NULL", 0 );
				return;
			}
			strcpy( arg, (char *) ch->dest_buf );
			DISPOSE( ch->dest_buf );
			break;

		case SUB_TIMER_DO_ABORT:
			DISPOSE( ch->dest_buf );
			ch->substate = SUB_NONE;
			send_to_char( "You stop digging...\n\r", ch );
			act( AT_PLAIN, "$n stops digging...", ch, NULL, NULL, TO_ROOM );
			return;
	}

	ch->substate = SUB_NONE;

	/* not having a shovel makes it harder to succeed */
	shovel = FALSE;
	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		if ( obj->item_type == ITEM_SHOVEL )
		{
			shovel = TRUE;
			break;
		}

	/* dig out an EX_DIG exit... */
	if ( arg[0] != '\0' )
	{
		if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL
				&&     IS_SET( pexit->exit_info, EX_DIG )
				&&     IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
			/* 4 times harder to dig open a passage without a shovel */
			if ( (number_percent() * (shovel ? 1 : 4)) <
					(IS_NPC(ch) ? 80 : ch->pcdata->learned[gsn_dig]) )
			{
				REMOVE_BIT( pexit->exit_info, EX_CLOSED );
				send_to_char( "You dig open a passageway!\n\r", ch );
				act( AT_PLAIN, "$n digs open a passageway!", ch, NULL, NULL, TO_ROOM );
				learn_from_success( ch, gsn_dig );
				return;
			}
		}
		learn_from_failure( ch, gsn_dig );
		send_to_char( "Your dig did not discover any exit...\n\r", ch );
		act( AT_PLAIN, "$n's dig did not discover any exit...", ch, NULL, NULL, TO_ROOM );
		return;
	}

	startobj = ch->GetInRoom()->first_content;
	found = FALSE;

	for ( obj = startobj; obj; obj = obj->next_content )
	{
		/* twice as hard to find something without a shovel */
		if ( IS_OBJ_STAT( obj, ITEM_BURIED )
				&&  (number_percent() * (shovel ? 1 : 2)) <
				(IS_NPC(ch) ? 80 : ch->pcdata->learned[gsn_dig]) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char( "Your dig uncovered nothing.\n\r", ch );
		act( AT_PLAIN, "$n's dig uncovered nothing.", ch, NULL, NULL, TO_ROOM );
		learn_from_failure( ch, gsn_dig );
		return;
	}

	separate_obj(obj);
	REMOVE_BIT( obj->extra_flags, ITEM_BURIED );
	act( AT_SKILL, "Your dig uncovered $p!", ch, obj, NULL, TO_CHAR );
	act( AT_SKILL, "$n's dig uncovered $p!", ch, obj, NULL, TO_ROOM );
	learn_from_success( ch, gsn_dig );
	if ( obj->item_type == ITEM_CORPSE_PC
			||   obj->item_type == ITEM_CORPSE_NPC )
		adjust_favor( ch, 14, 1 );
	save_buried_items(ch);

	return;
}

/*
 *  CRAFTING AND GATHERING SKILLS
 */

void do_mine(CHAR_DATA *ch, const char* argument)
{
	std::map<int, int>::iterator it;
	OBJ_DATA *obj;

	char arg  [MAX_INPUT_LENGTH];
	int percent;
	bool found;

	switch( ch->substate )
	{
		default:
			if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
			{
				send_to_char( "You can't concentrate enough for that.\n\r", ch );
				return;
			}
			if ( ch->GetMount() )
			{
				send_to_char( "You can't do that while mounted.\n\r", ch );
				return;
			}

			/* Blatant abuse of the find_vnum_component function */
			if( find_vnum_component( ch, 1409 ) == NULL )
			{
				send_to_char("You need something to mine with.\n\r", ch);
				return;
			}

			if( ch->GetInRoom()->mineMap.empty() )
			{
				send_to_char("The surface doesn't seem minable.\n\r", ch);
				return;
			}

         	argument = one_argument( argument, arg );
			add_timer( ch, TIMER_DO_FUN, UMIN(skill_table[gsn_mine]->beats / 10, 3), do_mine, 1 );
			act( AT_GREY, "You begin mining...", ch, NULL, NULL, TO_CHAR );
			act( AT_GREY, "$n begins mining...", ch, NULL, NULL, TO_ROOM );
			ch->dest_buf = str_dup( arg );
			return;

		case 1:
			if ( !ch->dest_buf )
			{
				act( AT_GREY, "Your mining was interrupted!", ch, NULL, NULL, TO_CHAR );
				act( AT_GREY, "$n's mining was interrupted!", ch, NULL, NULL, TO_ROOM );
				bug( "do_mine: dest_buf NULL", 0 );
				return;
			}
			strcpy( arg, (char *) ch->dest_buf );
			DISPOSE( ch->dest_buf );
			break;
		case SUB_TIMER_DO_ABORT:
			DISPOSE( ch->dest_buf );
			ch->substate = SUB_NONE;
				act( AT_GREY, "You stop mining.", ch, NULL, NULL, TO_CHAR );
				act( AT_GREY, "$n stops mining.", ch, NULL, NULL, TO_ROOM );
			return;
	}

	ch->substate = SUB_NONE;

	percent  = number_percent( ) - ( ch->level / 2 );

	if ( percent < ch->pcdata->learned[gsn_mine] )
	{
		for ( it = ch->GetInRoom()->mineMap.begin(); it != ch->GetInRoom()->mineMap.end(); ++it )
		{
			if ( number_percent() <= (*it).second )
			{
				obj = create_object( get_obj_index((*it).first), 0);
				obj_to_room(obj, ch->GetInRoom());
				act( AT_SKILL, "You find $p!", ch, obj, NULL, TO_CHAR );
				act( AT_SKILL, "$n finds $p!", ch, obj, NULL, TO_ROOM );
				learn_from_success( ch, gsn_mine );
				found = TRUE;
				return;
			}
		}
	}

	if ( !found )
	{
		send_to_char( "You find nothing.\n\r", ch );
		learn_from_failure( ch, gsn_mine );
		return;
	}
	return;
}

void do_smelt(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *ore;
	int percent;

	switch( ch->substate )
	{
		default:
			if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
			{
				send_to_char( "You can't concentrate enough for that.\n\r", ch );
				return;
			}
			if( ch->GetMount() )
			{
				send_to_char( "You can't do that while mounted.\n\r", ch );
				return;
			}

         	if( !IS_SET( ch->GetInRoom()->room_flags, ROOM_FORGE ) )
         	{
            	send_to_char( "You need access to a forge to smelt ore into ingots.\r\n", ch );
            	return;
         	}

			argument = one_argument( argument, arg );

    		if( arg[0] == '\0' )
    		{
        		send_to_char("What is it you wish to smelt?\r\n", ch);
        		return;
    		}

			if( ( ore = get_obj_here( ch, arg ) ) == NULL )
			{
				send_to_char( "You don't have that item.\n\r", ch );
				return;
			}

    		if( ore->item_type != ITEM_ORE )
    		{
        		send_to_char("That is not ore!\r\n", ch);
        		return;
    		}

	  		separate_obj( ore ); /* Separates obj from stack of like objects */
         	obj_from_char( ore );
			obj_to_char( ore, ch );

			add_timer( ch, TIMER_DO_FUN, UMIN(skill_table[gsn_smelt]->beats / 10, 3), do_smelt, 1 );
			act( AT_GREY, "You begin smelting $p...", ch, ore, NULL, TO_CHAR );
			act( AT_GREY, "$n begins smelting $p...", ch, ore, NULL, TO_ROOM );
			ch->dest_buf = str_dup( arg );
			return;

		case 1:
			if ( !ch->dest_buf )
			{
				act( AT_GREY, "Your smelting was interrupted!", ch, NULL, NULL, TO_CHAR );
				act( AT_GREY, "$n's smelting was interrupted!", ch, NULL, NULL, TO_ROOM );
				bug( "do_smelt: dest_buf NULL", 0 );
				return;
			}
			strcpy( arg, (char *) ch->dest_buf );
			DISPOSE( ch->dest_buf );
			break;
		case SUB_TIMER_DO_ABORT:
			DISPOSE( ch->dest_buf );
			ch->substate = SUB_NONE;
				act( AT_GREY, "You stop smelting.", ch, NULL, NULL, TO_CHAR );
				act( AT_GREY, "$n stops smelting.", ch, NULL, NULL, TO_ROOM );
			return;
	}

	ch->substate = SUB_NONE;

   	for( ore = ch->last_carrying; ore; ore = ore->prev_content )
      	if( ore->item_type == ITEM_ORE )
			break;

	percent  = number_percent( ) - ( ch->level / 2 );

	if ( percent < ch->pcdata->learned[gsn_smelt] )
	{
		act( AT_GREY, "You successfully smelt $p into an ingot!", ch, ore, NULL, TO_CHAR );
		act( AT_GREY, "$n successfully smelts $p into an ingot!", ch, ore, NULL, TO_ROOM );
		learn_from_success( ch, gsn_smelt );

	    ore->item_type = ITEM_TREASURE;
		if ( ore->pIndexData->vnum == OBJ_VNUM_TIN_ORE )
		{
	        ore->name_ = "tin ingot";
	        ore->shortDesc_ = "an ingot of solid tin";
	        ore->longDesc_ = "An ingot of solid tin lies on the ground.";
		}
		if ( ore->pIndexData->vnum == OBJ_VNUM_COPPER_ORE )
		{
	        ore->name_ = "copper ingot";
	        ore->shortDesc_ = "an ingot of solid copper";
	        ore->longDesc_ = "An ingot of solid copper lies on the ground.";
		}
		if ( ore->pIndexData->vnum == OBJ_VNUM_IRON_ORE )
		{
	        ore->name_ = "iron ingot";
	        ore->shortDesc_ = "an ingot of solid iron";
	        ore->longDesc_ = "An ingot of solid iron lies on the ground.";
		}
		if ( ore->pIndexData->vnum == OBJ_VNUM_LEAD_ORE )
		{
	        ore->name_ = "lead ingot";
	        ore->shortDesc_ = "an ingot of solid lead";
	        ore->longDesc_ = "An ingot of solid lead lies on the ground.";
		}
		if ( ore->pIndexData->vnum == OBJ_VNUM_SILVER_ORE )
		{
	        ore->name_ = "silver ingot";
	        ore->shortDesc_ = "an ingot of solid silver";
	        ore->longDesc_ = "An ingot of solid silver lies on the ground.";
		}
		if ( ore->pIndexData->vnum == OBJ_VNUM_GOLD_ORE )
		{
	        ore->name_ = "gold ingot";
	        ore->shortDesc_ = "an ingot of solid gold";
	        ore->longDesc_ = "An ingot of solid gold lies on the ground.";
		}
		if ( ore->pIndexData->vnum == OBJ_VNUM_MITHRIL_ORE )
		{
	        ore->name_ = "mithril ingot";
	        ore->shortDesc_ = "an ingot of solid mithril";
	        ore->longDesc_ = "An ingot of solid mithril lies on the ground.";
		}
		return;
	}
	else
	{
  		obj_from_char( ore ); /* Removes obj from character */
		send_to_char( "You fail to smelt, destroying the ore in the process.\n\r", ch );
		learn_from_failure( ch, gsn_smelt );
		return;
	}
	return;
}

void do_forge( CHAR_DATA * ch, const char *argument )
{
	send_to_char( "Sorry, this skill isn't finished yet :(\r\n", ch );
}

struct forage_data {
	int   amt;
	const char* short_d;
	const char* long_d;
	const char* action_d;
	const char* ofound;
	const char* cfound;
};
struct forage_data forage_find[SECT_MAX] = {
	{0, NULL, NULL, NULL, NULL},  /* SECT_INSIDE */
	{0, NULL, NULL, NULL, NULL},  /* SECT_CITY   */
	{3,
		"a few roots",
		"A few roots, recently dug out of the ground, lie here.",
		"%s crunch$q on some roots.",
		"$n digs a few roots from the ground.",
		"You dig a few roots from the ground."
	},  /* SECT_FIELD  */
	{5,
		"some berries",
		"Some berries, still attached to a branch, lie here.",
		"%s pick$q the berries off of the branch and put$q them in $s mouth.",
		"$n plucks a few berries from a bush.",
		"You pluck a few berries from a bush."
	},  /* SECT_FOREST */
	{3,
		"a mushroom",
		"A mushroom has been dropped here.",
		"%s plop$q the mushroom in $s mouth.",
		"$n lifts up a rock and plucks a mushroom from underneath.",
		"You lift up a rock and pluck a mushroom from underneath."
	},  /* SECT_HILLS  */
	{1,
		"a patch of fungus",
		"A small patch of something green covers the ground.",
		"%s chew$q thoughtfully on the fungus.",
		"$n scrapes some green stuff from the rocks.",
		"You scrape some green stuff from the rocks."
	},  /* SECT_MOUNTAIN */
	{0, NULL, NULL, NULL, NULL, NULL},  /* SECT_WATER_SWIM */
	{0, NULL, NULL, NULL, NULL, NULL},  /* SECT_WATER_NOSWIM */
	{0, NULL, NULL, NULL, NULL, NULL},  /* SECT_UNDERWATER */
	{0, NULL, NULL, NULL, NULL, NULL},  /* AIR */
	{1,
		"a piece of cactus",
		"A small green plant pokes its thorns at you.",
		"%s pluck$q out a few thorns, and then bite$q at the cactus.",
		"$n reaches down and grabs a small thorny plant.",
		"You reach down and grab a small thorny plant.  OUCH"
	},  /* DESERT */
	{0, NULL, NULL, NULL, NULL, NULL},  /* DUNNO */
	{0, NULL, NULL, NULL, NULL, NULL},  /* OCEANFLOOR */
	{0, NULL, NULL, NULL, NULL, NULL},  /* UNDERGROUND */
	{2,
		"a piece of discarded food",
		"A small piece of discarded food lies in the middle of the road.",
		"%s devour$q the piece of discarded food, and then spit$q out a few bugs.",
		"$n dives into the bushes and comes out with an unrecognizable piece of food.",
		"You dive into the bushes and come out with a half eaten piece of food."
	},  /* ROADS */
};

/*
   Made forage a do_ skill instead of an automatic thing. -KSILYAN
*/
void do_forage(CHAR_DATA *ch, const char* argument)
{
	int sector;
	int sn_forage;
	OBJ_DATA *food;

	sn_forage = skill_lookup("forage");

	if (IS_NPC(ch))
	{
		send_to_char("NPCs can't forage for food!\n\r", ch);
		return;
	}
	if (ch->GetMount())
	{
		send_to_char("You can't forage from on top of your mount.\n\r", ch);
		return;
	}

	sector = ch->GetInRoom()->sector_type;

	if (forage_find[sector].amt <= 0)
	{
		send_to_char("You can't forage here.\n\r", ch);
		return;
	}

	if ( !ch->ChanceRollSkill(sn_forage) )
	{
		learn_from_failure(ch, sn_forage);
		send_to_char("You didn't find anything.", ch);
		ch->AddWait(skill_table[sn_forage]->beats);
		return;
	}

	food = create_object(get_obj_index(OBJ_VNUM_MUSHROOM), 0);
	food->value[0] = forage_find[ch->GetInRoom()->sector_type].amt + ch->level/10;
	food->timer    = 15;

	act(AT_ACTION, forage_find[sector].ofound, ch, NULL, NULL, TO_ROOM);
	act(AT_ACTION, forage_find[sector].cfound, ch, NULL, NULL, TO_CHAR);

	food->name_       = forage_find[sector].short_d;
	food->shortDesc_  = forage_find[sector].short_d;
	food->longDesc_   = forage_find[sector].long_d;
	food->actionDesc_ = forage_find[sector].action_d;

	obj_to_room(food, ch->GetInRoom());

	learn_from_success(ch, sn_forage);
	ch->AddWait(skill_table[sn_forage]->beats);
}


/*
   Added this to let characters search for something to make fires with,
   and light it (to avoid having to make the kindling object... meh)  -KSILYAN
*/
void do_kindle_fire(CHAR_DATA *ch, const char* argument)
{
    int sector;

    if (IS_NPC(ch))
    {
        send_to_char("NPCs can't search for kindling!\n\r", ch);
        return;
    }
    if (ch->GetMount())
    {
        send_to_char("You can't search from on top of your mount.\n\r", ch);
        return;
    }

    sector = ch->GetInRoom()->sector_type;

    if ((sector == SECT_FIELD) || (sector == SECT_HILLS) ||
        (sector == SECT_FOREST) || (sector == SECT_MOUNTAIN) ||
        (sector == SECT_DESERT) )
    {
        if (ch->move < 30 )
        {
            send_to_char("You are far too exhausted to go searching for kindling!\n\r", ch);
            return;
        }
        ch->move -= 30;
        if ( ch->ChanceRollSkill(gsn_kindle_fire) )
        {
            learn_from_success(ch, gsn_kindle_fire);
            switch(sector)
            {
                case SECT_FIELD:
                    send_to_char("You find some dry grass and collect it for your fire.\n\r", ch);
                    break;
                case SECT_FOREST:
                    send_to_char("You gather some dry twigs and leaves to make your fire.\n\r", ch);
                    break;
                case SECT_HILLS:
                    send_to_char("You get a few sticks and leaves to make a fire with.\n\r", ch);
                    break;
                case SECT_MOUNTAIN:
                    send_to_char("You find a few dead branches and leaves to make a fire with.\n\r", ch);
                    break;
                case SECT_DESERT:
                    send_to_char("You manage to find enough stunted vegetation to make a fire.\n\r", ch);
                    break;
            }
            make_fire(ch->GetInRoom(), 3);
            act( AT_FIRE, "$n gathers some kindling and lights a small fire.", ch, NULL, NULL, TO_ROOM    );
            act( AT_FIRE, "Your fire begins to burn brightly.", ch, NULL, NULL, TO_CHAR );
            ch->AddWait( skill_table[gsn_kindle_fire]->beats);
        }
        else
        {
            learn_from_failure(ch, gsn_kindle_fire);
            send_to_char("You didn't find anything.\n\r", ch);
            ch->AddWait( skill_table[gsn_kindle_fire]->beats);
        }
    }
    else
    {
        send_to_char("You can't search for kindling here!\n\r", ch);
    }
}

void do_search(CHAR_DATA *ch, const char* argument)
{
	char arg  [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *container;
	OBJ_DATA *startobj;
	ExitData *pexit;
	int percent, door;
	bool found, room;

	door = -1;
	switch( ch->substate )
	{
		default:
			if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
			{
				send_to_char( "You can't concentrate enough for that.\n\r", ch );
				return;
			}
			if ( ch->GetMount() )
			{
				send_to_char( "You can't do that while mounted.\n\r", ch );
				return;
			}
			argument = one_argument( argument, arg );
			if ( arg[0] != '\0' && (door = get_door( arg )) == -1 )
			{
				container = get_obj_here( ch, arg );
				if ( !container )
				{
					send_to_char( "You can't find that here.\n\r", ch );
					return;
				}
				if ( container->item_type != ITEM_CONTAINER )
				{
					send_to_char( "You can't search in that!\n\r", ch );
					return;
				}
				if ( IS_SET(container->value[1], CONT_CLOSED) )
				{
					send_to_char( "It is closed.\n\r", ch );
					return;
				}
			}
			add_timer( ch, TIMER_DO_FUN, UMIN(skill_table[gsn_search]->beats / 10, 3),
					do_search, 1 );
			send_to_char( "You begin your search...\n\r", ch );
			ch->dest_buf = str_dup( arg );
			return;

		case 1:
			if ( !ch->dest_buf )
			{
				send_to_char( "Your search was interrupted!\n\r", ch );
				bug( "do_search: dest_buf NULL", 0 );
				return;
			}
			strcpy( arg, (char *) ch->dest_buf );
			DISPOSE( ch->dest_buf );
			break;
		case SUB_TIMER_DO_ABORT:
			DISPOSE( ch->dest_buf );
			ch->substate = SUB_NONE;
			send_to_char( "You stop your search...\n\r", ch );
			return;
	}
	ch->substate = SUB_NONE;
	if ( arg[0] == '\0' )
	{
		room = TRUE;
		pexit = ch->GetInRoom()->first_exit;
		startobj = ch->GetInRoom()->first_content;
	}
	else
	{
		if ( (door = get_door( arg )) != -1 )
			startobj = NULL;
		else
		{
			container = get_obj_here( ch, arg );
			if ( !container )
			{
				send_to_char( "You can't find that here.\n\r", ch );
				return;
			}
			startobj = container->first_content;
		}
	}

	found = FALSE;

	if ( (!startobj && !pexit && door == -1) || IS_NPC(ch) )
	{
		send_to_char( "You find nothing.\n\r", ch );
		learn_from_failure( ch, gsn_search );
		return;
	}

	percent  = number_percent( ) + number_percent( ) - ( ch->level / 10 );

	if ( door != -1 )
	{
		ExitData *pexit;

		pexit = get_exit(ch->GetInRoom(), door);

		if (pexit)
		{
			if ( IS_SET( pexit->exit_info, EX_SECRET )
					&&   IS_SET( pexit->exit_info, EX_xSEARCHABLE )
					&&   percent < (IS_NPC(ch) ? 80 : ch->pcdata->learned[gsn_search]) )
			{
				act( AT_SKILL, "Your search reveals the $d!", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );
				act( AT_SKILL, "$n finds the $d!", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
				REMOVE_BIT( pexit->exit_info, EX_SECRET );
				learn_from_success( ch, gsn_search );
				return;
			}
		}
	}
	//else
	//{
	/*
	 * KSILYAN
	 *	Make it possible to find hidden doors without specifying a direction, but it's harder.
	 */
	for (pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next)
	{
		if ( IS_SET( pexit->exit_info, EX_SECRET )
				&&   IS_SET( pexit->exit_info, EX_xSEARCHABLE )
				&&   percent < (IS_NPC(ch) ? 80 : ch->pcdata->learned[gsn_search] * 0.80) )
		{
			act( AT_SKILL, "Your search reveals the $d!", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );
			act( AT_SKILL, "$n finds the $d!", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
			REMOVE_BIT( pexit->exit_info, EX_SECRET );
			learn_from_success( ch, gsn_search );
			return;
		}
	}
	//}
	//	else
	for ( obj = startobj; obj; obj = obj->next_content )
	{
		if ( (IS_OBJ_STAT( obj, ITEM_HIDDEN )
					|| IS_OBJ_STAT(obj, ITEM_BURIED) )
				&&   percent < ch->pcdata->learned[gsn_search] )
		{
			found = TRUE;
			break;
		}
	}

	if ( percent < ch->pcdata->learned[gsn_search] )
	{
		if (rprog_search_trigger(ch))
		{
			learn_from_success( ch, gsn_search );
			found = TRUE;
			return;
		}
	}

	if ( !found )
	{
		send_to_char( "You find nothing.\n\r", ch );
		learn_from_failure( ch, gsn_search );
		return;
	}

	if ( IS_OBJ_STAT(obj, ITEM_BURIED) ) {
		act(AT_SKILL, "The ground seems to have been disturbed recently.", ch, obj, NULL, TO_CHAR);
	} else {
		separate_obj(obj);
		REMOVE_BIT( obj->extra_flags, ITEM_HIDDEN );
		act( AT_SKILL, "Your search reveals $p!", ch, obj, NULL, TO_CHAR );
		act( AT_SKILL, "$n finds $p!", ch, obj, NULL, TO_ROOM );
		learn_from_success( ch, gsn_search );
	}
	return;
}


void do_steal(CHAR_DATA *ch, const char* argument)
{
	char buf  [MAX_STRING_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( ch->GetMount() )
	{
		send_to_char( "You can't do that while mounted.\n\r", ch );
		return;
	}

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Steal what from whom?\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "That's pointless.\n\r", ch );
		return;
	}

	if ( is_safe(ch, victim) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "A magical force interrupts you.\n\r", ch );
		return;
	}

	if ( !IS_IMMORTAL(ch) ) {

		ch->AddWait( skill_table[gsn_steal]->beats );
		percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 )
			- ch->getLuckBonus() + 2 + victim->getLuckBonus(); // +2 makes it harder

		/* Changed the level check, made it 10 levels instead of five and made the
		   victim not attack in the case of a too high level difference.  This is
		   to allow mobprogs where the mob steals eq without having to put level
		   checks into the progs.  Also gave the mobs a 10% chance of failure.
		 */
		if( ch->level + 10 < victim->level )
		{
			send_to_char( "You really don't want to try that!\n\r", ch );
			return;
		}

		if ( victim->position == POS_FIGHTING
				||   percent > ( IS_NPC(ch) ? 90 : ch->pcdata->learned[gsn_steal] ) )
		{
			/*
			 * Failure.
			 */
			send_to_char( "Oops...\n\r", ch );
			act( AT_ACTION, "$n tried to steal from you!\n\r", ch, NULL, victim, TO_VICT    );
			act( AT_ACTION, "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );

			sprintf( buf, "%s is a bloody thief!", ch->getShort().c_str() );
			do_yell( victim, buf );

			learn_from_failure( ch, gsn_steal );
			if ( IS_NPC(victim)  )
			{
				global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
			}
			mprog_steal_trigger(ch);
			return;
		}
	}

	if ( !str_cmp( arg1, "coin"  )
			||   !str_cmp( arg1, "coins" )
			||   !str_cmp( arg1, "gold"  ) )
	{
		int amount;

		amount = (int) (victim->gold * number_range(1, 10) / 100);
		if ( amount <= 0 )
		{
			send_to_char( "You couldn't get any gold.\n\r", ch );
			learn_from_failure( ch, gsn_steal );
			return;
		}

		ch->gold     += amount;
		victim->gold -= amount;
		ch_printf( ch, "Aha!  You got %d gold coins.\n\r", amount );
		learn_from_success( ch, gsn_steal );
		if(!IS_NPC(victim))
		{
			sprintf(buf,"PS: %s steals %d gold coins from %s in room %s",
					NAME(ch),amount,NAME(victim),
					ch->GetInRoom()->name_.c_str());
			log_string(buf);
			to_channel(buf, CHANNEL_MONITOR, "Monitor", 1);
		}
		mprog_steal_trigger(ch);
		return;
	}

	if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
	{
		send_to_char( "You can't seem to find it.\n\r", ch );
		learn_from_failure( ch, gsn_steal );
		return;
	}

	if ( !can_drop_obj( ch, obj )
			||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
			||	 IS_OBJ_STAT(obj, ITEM_PROTOTYPE)
#ifdef USE_OBJECT_LEVELS
			||   obj->level > ch->level )
#else
		)
#endif
		{
			send_to_char( "You can't manage to pry it away.\n\r", ch );
			learn_from_failure( ch, gsn_steal );
			return;
		}

	if ( ch->carry_number + (get_obj_number(obj)/obj->count) > can_carry_n( ch ) )
	{
		send_to_char( "You have your hands full.\n\r", ch );
		learn_from_failure( ch, gsn_steal );
		return;
	}

	if ( ch->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_w( ch ) )
	{
		send_to_char( "You can't carry that much weight.\n\r", ch );
		learn_from_failure( ch, gsn_steal );
		return;
	}

	if(!IS_NPC(victim))
	{
		sprintf(buf,"PS: %s steals '%s' (%s) from %s in room %s",
				NAME(ch),
				obj->shortDesc_.c_str(),
				vnum_to_dotted(obj->pIndexData->vnum),
				NAME(victim),
				ch->GetInRoom()->name_.c_str()
			   );
		log_string(buf);
		to_channel(buf, CHANNEL_MONITOR, "Monitor", 1);
	}
	separate_obj( obj );
	obj_from_char( obj );
	obj_to_char( obj, ch );
	send_to_char( "Ok.\n\r", ch );
	learn_from_success( ch, gsn_steal );
	adjust_favor( ch, 9, 1 );
	mprog_steal_trigger(ch);
	return;
}


void do_backstab(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't do that right now.\n\r", ch );
		return;
	}

	one_argument( argument, arg );

	if ( ch->GetMount() )
	{
		send_to_char( "You can't get close enough while mounted.\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char( "Backstab whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "How can you sneak up on yourself?\n\r", ch );
		return;
	}

	if ( is_safe( ch, victim ) )
		return;

	/* Added stabbing weapon. -Narn */
	if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
			||   ( obj->value[OBJECT_WEAPON_DAMAGETYPE] != DAMAGE_PIERCE) )
	{
		send_to_char( "You need to wield a piercing or stabbing weapon.\n\r", ch );
		return;
	}

	if ( victim->IsFighting() )
	{
		send_to_char( "You can't backstab someone who is in combat.\n\r", ch );
		return;
	}

	/* Can backstab a char even if it's hurt as long as it's sleeping. -Narn */
	if ( victim->hit < victim->max_hit && IS_AWAKE( victim ) )
	{
		act( AT_PLAIN, "$N is hurt and suspicious ... you can't sneak up.",
				ch, NULL, victim, TO_CHAR );
		return;
	}

	ch->AddWait( skill_table[gsn_backstab]->beats );
	if ( !IS_AWAKE(victim)
			||   ch->ChanceRollSkill(gsn_backstab, -victim->getLuckBonus()) )
	{
		learn_from_success( ch, gsn_backstab );
		global_retcode = multi_hit( ch, victim, gsn_backstab );
		adjust_favor( ch, 10, 1 );

	}
	else
	{
		learn_from_failure( ch, gsn_backstab );
		global_retcode = damage( ch, victim, 0, gsn_backstab, 0 );
	}
	return;
}


void do_rescue(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Rescue whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "How about fleeing instead?\n\r", ch );
		return;
	}

	if ( ch->GetMount() )
	{
		send_to_char( "You can't do that while mounted.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
		send_to_char( "Doesn't need your help!\n\r", ch );
		return;
	}

	if ( !ch->IsFighting() )
	{
		send_to_char( "You must be fighting the same person they are...\r\n", ch);
		return;
	}

	if ( ( fch = victim->GetVictim() ) == NULL )
	{
		send_to_char( "They are not fighting right now.\n\r", ch );
		return;
	}

	ch->AddWait( skill_table[gsn_rescue]->beats );
	if ( !ch->ChanceRollSkill(gsn_rescue, victim->getLuckBonus()) )
	{
		send_to_char( "You fail the rescue.\n\r", ch );
		act( AT_SKILL, "$n tries to rescue you!", ch, NULL, victim, TO_VICT   );
		act( AT_SKILL, "$n tries to rescue $N!", ch, NULL, victim, TO_NOTVICT );
		learn_from_failure( ch, gsn_rescue );
		return;
	}

	act( AT_SKILL, "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
	act( AT_SKILL, "$n rescues you!", ch, NULL, victim, TO_VICT    );
	act( AT_SKILL, "$n moves in front of $N!",  ch, NULL, victim, TO_NOTVICT );

	learn_from_success( ch, gsn_rescue );
	adjust_favor( ch, 8, 1 );

	//stop_fighting( fch, FALSE );
	//stop_fighting( victim, FALSE );
	fch->StopAttacking();
	victim->StopAttacking();

	if ( ch->IsFighting() )
		//stop_fighting( ch, FALSE );
		ch->StopAttacking();

	/* check_killer( ch, fch ); */

	//set_fighting( ch, fch );
	//set_fighting( fch, ch );
	ch->StartAttacking(fch);
	fch->StartAttacking(ch);
	return;
}



void do_kick(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	bool Hit;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch)
	     &&   ch->level < skill_table[gsn_kick]->skill_level[ch->Class] )
	{
		send_to_char("You better leave the martial arts to fighters.\n\r", ch );
		return;
	}

	if ( ( victim = ch->GetVictim() ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	ch->AddWait( skill_table[gsn_kick]->beats );

	Hit = MeeleeHit(ch, victim, NULL, (IS_NPC(ch) ? ch->level / 5 : ch->pcdata->learned[gsn_kick]) );

	if ( Hit )
	{
		int Damage;
		Damage = number_range(1, ch->level);

		learn_from_success( ch, gsn_kick );
		global_retcode = damage( ch, victim, Damage, gsn_kick, 0 );
	}
	else
	{
		learn_from_failure( ch, gsn_kick );
		global_retcode = damage( ch, victim, 0, gsn_kick, 0 );
	}
	return;
}

void do_punch(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA *victim;
	bool Hit;
	short Damage;
	OBJ_DATA * Shield;
	short DamageType;
	short DamageMessage;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_punch]->skill_level[ch->Class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( ( victim = ch->GetVictim() ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    ch->AddWait( skill_table[gsn_punch]->beats );
	Hit = MeeleeHit(ch, victim, NULL, (IS_NPC(ch) ? ch->level / 5 : ch->pcdata->learned[gsn_punch]) );

	Shield = get_eq_char(ch, WEAR_SHIELD);

	if (Shield)
	{
		// Shieldbash!
		Damage = number_range(1, ch->level);

		DamageType = TYPE_HIT + DAMAGE_BLUNT;
		DamageMessage = DAMAGE_MSG_SHIELDBASH;
	}
	else
	{
		// Normal punch
		Damage = number_range(1, ch->level);

		DamageType = gsn_punch;
		DamageMessage = 0;
	}

	if ( Hit )
	{

		learn_from_success( ch, gsn_punch );

		global_retcode = damage(ch, victim, Damage, DamageType, DamageMessage);
	}
	else
	{
		learn_from_failure( ch, gsn_punch );
		global_retcode = damage( ch, victim, 0, DamageType, DamageMessage );
	}
    return;
}


void do_bite(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch)
		&&	 ch->level < skill_table[gsn_bite]->skill_level[ch->Class] )
	{
		send_to_char(
			"That isn't quite one of your natural skills.\n\r", ch );
		return;
	}

	if (!IS_NPC(ch) && time_info.hour>=sunrise_hour() && !sun_broken())
	{
		send_to_char("It is not the hour for that.\n\r", ch );
		return;
	}

	if ( ( victim = ch->GetVictim() ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	ch->AddWait( skill_table[gsn_bite]->beats );

	if ( ch->ChanceRollSkill(gsn_bite) )
	{
		learn_from_success( ch, gsn_bite );
		global_retcode = damage( ch, victim, number_range( 1, 2 * ch->level ), TYPE_HIT + DAMAGE_PIERCE , DAMAGE_MSG_BITE);
	}
	else
	{
		learn_from_failure( ch, gsn_bite );
		global_retcode = damage( ch, victim, 0, TYPE_HIT + DAMAGE_PIERCE, DAMAGE_MSG_BITE );
	}

	return;
}


void do_claw(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_claw]->skill_level[ch->Class] )
    {
	send_to_char(
	    "That isn't quite one of your natural skills.\n\r", ch );
	return;
    }

    if ( ( victim = ch->GetVictim() ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    ch->AddWait( skill_table[gsn_claw]->beats );
    if ( ch->ChanceRollSkill(gsn_claw) )
    {
	learn_from_success( ch, gsn_claw );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), TYPE_HIT + DAMAGE_SLASH,DAMAGE_MSG_CLAW );
    }
    else
    {
	learn_from_failure( ch, gsn_claw );
	global_retcode = damage( ch, victim, 0, TYPE_HIT + DAMAGE_SLASH,DAMAGE_MSG_CLAW );
    }
    return;
}


void do_sting(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_sting]->skill_level[ch->Class] )
    {
	send_to_char(
	    "That isn't quite one of your natural skills.\n\r", ch );
	return;
    }

    if ( ( victim = ch->GetVictim() ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    ch->AddWait( skill_table[gsn_sting]->beats );
    if ( ch->ChanceRollSkill(gsn_sting) )
    {
	learn_from_success( ch, gsn_sting );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_sting,0 );
    }
    else
    {
	learn_from_failure( ch, gsn_sting );
	global_retcode = damage( ch, victim, 0, gsn_sting,0 );
    }
    return;
}


void do_tail(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_tail]->skill_level[ch->Class] )
    {
	send_to_char(
	    "That isn't quite one of your natural skills.\n\r", ch );
	return;
    }

    if ( ( victim = ch->GetVictim() ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    ch->AddWait( skill_table[gsn_tail]->beats );
    if ( ch->ChanceRollSkill(gsn_tail) )
    {
	learn_from_success( ch, gsn_tail );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_tail , 0);
    }
    else
    {
	learn_from_failure( ch, gsn_tail );
	global_retcode = damage( ch, victim, 0, gsn_tail , 0);
    }
    return;
}


void do_bash(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	int chance;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch)
			&&   ch->level < skill_table[gsn_bash]->skill_level[ch->Class] )
	{
		send_to_char(
				"You better leave the martial arts to fighters.\n\r", ch );
		return;
	}

	if ( ( victim = ch->GetVictim() ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	chance = (((victim->getDex() + victim->getStr())
				-   (ch->getDex()     + ch->getStr())) * 10) + 10;

	if ( !IS_NPC(ch) && !IS_NPC(victim) )
		chance += 25;
	if ( victim->GetVictim() != ch )
		chance += 19;

	ch->AddWait( skill_table[gsn_bash]->beats );

	if ( ch->ChanceRollSkill(gsn_bash, -chance) )
	{
		learn_from_success( ch, gsn_bash );
		/* do not change anything here!  -Thoric */
		ch->AddWait(     2 * PULSE_VIOLENCE );
		victim->AddWait( 2 * PULSE_VIOLENCE );
		victim->position = POS_SITTING;
		global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_bash ,0);
	}
	else
	{
		ch->AddWait(     2 * PULSE_VIOLENCE );
		learn_from_failure( ch, gsn_bash );
		global_retcode = damage( ch, victim, 0, gsn_bash,0 );
	}
	return;
}


void do_stun(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int chance;
	bool fail;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch)
			&&   ch->level < skill_table[gsn_stun]->skill_level[ch->Class] )
	{
		send_to_char(
				"You better leave the martial arts to fighters.\n\r", ch );
		return;
	}

	if ( ( victim = ch->GetVictim() ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	if ( ch->move < 16 )
	{
		set_char_color( AT_SKILL, ch );
		send_to_char( "You are far too tired to do that.\n\r", ch );
		return;		/* missing return fixed March 11/96 */
	}

	ch->AddWait( skill_table[gsn_stun]->beats );
	fail = FALSE;
	chance = ris_save( victim, ch->level, RIS_PARALYSIS );
	if ( chance == 1000 )
		fail = TRUE;
	else
		fail = saves_para_petri( chance, victim );

	chance = (((victim->getDex() + victim->getStr())
				-   (ch->getDex()     + ch->getStr())) * 10) + 10;
	/* harder for player to stun another player */
	if ( !IS_NPC(ch) && !IS_NPC(victim) )
		chance += sysdata.stun_plr_vs_plr;
	else
		chance += sysdata.stun_regular;
	if ( !fail
			&& ch->ChanceRollSkill(gsn_stun, -chance) )
	{
		learn_from_success( ch, gsn_stun );
		/*    DO *NOT* CHANGE!    -Thoric    */
		ch->move -= 15;
		ch->AddWait(     2 * PULSE_VIOLENCE );
		victim->AddWait( PULSE_VIOLENCE );
		act( AT_SKILL, "$N smashes into you, leaving you stunned!", victim, NULL, ch, TO_CHAR );
		act( AT_SKILL, "You smash into $N, leaving $M stunned!", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n smashes into $N, leaving $M stunned!", ch, NULL, victim, TO_NOTVICT );
		if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
		{
			af.type      = gsn_stun;
			af.location  = APPLY_AC;
			af.modifier  = 20;
			af.duration  = 3;
			af.bitvector = AFF_PARALYSIS;
			affect_to_char( victim, &af );
			update_pos( victim );
		}
	}
	else
	{
		ch->AddWait(     2 * PULSE_VIOLENCE );
		ch->move -= 5;
		learn_from_failure( ch, gsn_stun );
		act( AT_SKILL, "$N charges at you screaming, but you dodge out of the way.", victim, NULL, ch, TO_CHAR );
		act( AT_SKILL, "You try to stun $N, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n charges screaming at $N, but keeps going right on past.", ch, NULL, victim, TO_NOTVICT );
	}
	return;
}


void do_feed(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	sh_int dam;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch)
			&&   !IS_VAMPIRE(ch) )
	{
		send_to_char( "It is not of your nature to feed on living creatures.\n\r", ch );
		return;
	}
	if ( !IS_NPC(ch)
			&&   !ch->pcdata->learned[gsn_feed] )
	{
		send_to_char( "You have not yet practiced your new teeth.\n\r", ch );
		return;
	}

	if ( ( victim = ch->GetVictim() ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	if ( ch->GetMount() )
	{
		send_to_char( "You can't do that while mounted.\n\r", ch );
		return;
	}

	ch->AddWait( skill_table[gsn_feed]->beats );
	if ( ch->ChanceRollSkill(gsn_feed) )
	{
		dam = number_range( 1, ch->level );
		global_retcode = damage( ch, victim, dam, TYPE_HIT + DAMAGE_PIERCE,DAMAGE_MSG_BITE);
		if ( global_retcode == rNONE && !IS_NPC(ch) && dam
				&&  ch->IsFighting()
				&&  ch->pcdata->condition[COND_BLOODTHIRST] < (10 + ch->level) )
		{
			if(sun_broken() && number_bits(2)<SUN_BROKEN_VAMPEFFECT)
			{
				gain_condition( ch, COND_BLOODTHIRST,
						UMIN( number_range(1, (ch->level+victim->level / 20) + 1 ),
							(10 + ch->level) - ch->pcdata->condition[COND_BLOODTHIRST] ) );
				gain_condition( ch, COND_FULL, 1);
				gain_condition( ch, COND_THIRST, 1);
				act( AT_BLOOD, "You feed from $N, but something doesn't taste right.", ch, NULL,
						victim, TO_CHAR );
			}
			else
			{
				gain_condition( ch, COND_BLOODTHIRST,
						UMIN( number_range(1, (ch->level+victim->level / 20) + 3 ),
							(10 + ch->level) - ch->pcdata->condition[COND_BLOODTHIRST] ) );
				gain_condition( ch, COND_FULL, 2);
				gain_condition( ch, COND_THIRST, 2);
				act( AT_BLOOD, "You manage to suck a little life out of $N.", ch, NULL,
						victim, TO_CHAR );
			}
			act( AT_BLOOD, "$n sucks some of your blood!", ch, NULL, victim, TO_VICT );
			learn_from_success( ch, gsn_feed );
		}
	}
	else
	{
		global_retcode = damage( ch, victim, 0, TYPE_HIT + DAMAGE_PIERCE ,DAMAGE_MSG_BITE);
		if ( global_retcode == rNONE && !IS_NPC(ch)
				&&  ch->IsFighting()
				&&  ch->pcdata->condition[COND_BLOODTHIRST] < (10 + ch->level) )
		{
			if(sun_broken() && number_bits(2)<SUN_BROKEN_VAMPEFFECT)
			{
				act( AT_BLOOD, "Something about $N's blood smells off.",
						ch, NULL, victim, TO_CHAR );
			}
			else
			{
				act( AT_BLOOD, "The smell of $N's blood is driving you insane!",
						ch, NULL, victim, TO_CHAR );
			}
			act( AT_BLOOD, "$n is lusting after your blood!", ch, NULL, victim, TO_VICT );
			learn_from_failure( ch, gsn_feed );
		}
	}
	return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 * Check for loyalty flag (weapon disarms to inventory) for pkillers -Blodkai
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
	OBJ_DATA *obj, *tmpobj;

	if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
		return;

	if ( ( tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD ) ) != NULL
			&&     number_bits( 1 ) == 0 )
		obj = tmpobj;

	if ( get_eq_char( ch, WEAR_WIELD ) == NULL && number_bits( 1 ) == 0 )
	{
		learn_from_failure( ch, gsn_disarm );
		return;
	}

	if ( IS_NPC( ch ) && !can_see_obj( ch, obj ) && number_bits( 1 ) == 0)
	{
		learn_from_failure( ch, gsn_disarm );
		return;
	}

	if ( check_grip( ch, victim ) )
	{
		learn_from_failure( ch, gsn_disarm );
		return;
	}

	act( AT_SKILL, "$n DISARMS you!", ch, NULL, victim, TO_VICT    );
	act( AT_SKILL, "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
	act( AT_SKILL, "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );
	learn_from_success( ch, gsn_disarm );

	if ( obj == get_eq_char( victim, WEAR_WIELD )
			&&  (tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD)) != NULL )
		tmpobj->wear_loc = WEAR_WIELD;

	obj_from_char( obj );

	if ( IS_NPC(victim)
			|| ( IS_OBJ_STAT(obj, ITEM_LOYAL) )
			|| ( in_arena(victim) ) )
		obj_to_char( obj, victim );
	else
		obj_to_room( obj, victim->GetInRoom() );

	return;
}


void do_disarm(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	if ( !IS_NPC(ch)
			&&   ch->level < skill_table[gsn_disarm]->skill_level[ch->Class] )
	{
		send_to_char( "You don't know how to disarm opponents.\n\r", ch );
		return;
	}

	if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
	{
		send_to_char( "You must wield a weapon to disarm.\n\r", ch );
		return;
	}

	if ( ( victim = ch->GetVictim() ) == NULL )
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	{
		send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
		return;
	}

	ch->AddWait( skill_table[gsn_disarm]->beats );
	percent = number_percent( ) + victim->level - ch->level
		- ch->getLuckBonus() + victim->getLuckBonus();

	if ( !can_see_obj( ch, obj ) )
		percent += 10;
	if ( IS_NPC(ch) || percent < ch->pcdata->learned[gsn_disarm] * 2 / 3 )
		disarm( ch, victim );
	else
	{
		send_to_char( "You failed.\n\r", ch );
		learn_from_failure( ch, gsn_disarm );
	}
	return;
}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( victim->GetMount() )
	{
		if ( IS_AFFECTED( victim->GetMount(), AFF_FLYING )
			||	 IS_AFFECTED( victim->GetMount(), AFF_FLOATING ) )
			return;
		act( AT_SKILL, "$n trips your mount and you fall off!", ch, NULL, victim, TO_VICT	 );
		act( AT_SKILL, "You trip $N's mount and $N falls off!", ch, NULL, victim, TO_CHAR	 );
		act( AT_SKILL, "$n trips $N's mount and $N falls off!", ch, NULL, victim, TO_NOTVICT );
		REMOVE_BIT( victim->GetMount()->act, ACT_MOUNTED );
		victim->MountId = 0;
		ch->AddWait(	 1 * PULSE_VIOLENCE );
		victim->AddWait( 2 * PULSE_VIOLENCE );
		victim->position = POS_RESTING;
		return;
	}
	if ( victim->GetWait() == 0 )
	{
		if ( IS_AFFECTED( victim, AFF_FLYING )
			||	 IS_AFFECTED( victim, AFF_FLOATING ) )
			return;

		act( AT_SKILL, "$n trips you and you go down!", ch, NULL, victim, TO_VICT	 );
		act( AT_SKILL, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR	 );
		act( AT_SKILL, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );

		ch->AddWait(	 1 * PULSE_VIOLENCE );
		victim->AddWait( 2 * PULSE_VIOLENCE );
		victim->position = POS_RESTING;
	}

	return;
}


void do_pick(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    ExitData *pexit;

    /* Prevent non thief players from picking locks */
/*
    if  ( ch->Class != CLASS_THIEF )
    {
	send_to_char("You wish you knew how to pick locks!\n\r", ch);
	return;
    }
*/

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ch->GetMount() )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    ch->AddWait( skill_table[gsn_pick_lock]->beats );

    /* look for guards */
    for ( gch = ch->GetInRoom()->first_person; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	act( AT_PLAIN, "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !ch->ChanceRollSkill(gsn_pick_lock) )
    {
	send_to_char( "You failed.\n\r", ch);
	learn_from_failure( ch, gsn_pick_lock );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'pick door' */
/*	ROOM_INDEX_DATA *to_room; */ /* Unused */
	ExitData *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) )
	{
	   send_to_char( "You failed.\n\r", ch );
	   learn_from_failure( ch, gsn_pick_lock );
	   check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
	   return;
	}

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n picks the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
	learn_from_success( ch, gsn_pick_lock );
	adjust_favor( ch, 9, 1 );
	/* pick the other side */
	if ( ( pexit_rev = pexit->rexit ) != NULL
	&&   pexit_rev->to_room == ch->GetInRoom() )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
	check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	{
	   send_to_char( "You failed.\n\r", ch );
	   learn_from_failure( ch, gsn_pick_lock );
	   check_for_trap( ch, obj, TRAP_PICK );
	   return;
	}

	separate_obj( obj );
	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n picks $p.", ch, obj, NULL, TO_ROOM );
	learn_from_success( ch, gsn_pick_lock );
	adjust_favor( ch, 9, 1 );
	check_for_trap( ch, obj, TRAP_PICK );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_sneak(CHAR_DATA *ch, const char* argument)
{
    AFFECT_DATA af;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( ch->GetMount() )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if ( ch->ChanceRollSkill(gsn_sneak) )
    {
	af.type      = gsn_sneak;
	af.duration  = (int) ( ch->level * DUR_CONV );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
	learn_from_success( ch, gsn_sneak );
    }
    else
	learn_from_failure( ch, gsn_sneak );

    return;
}



void do_hide(CHAR_DATA *ch, const char* argument)
{
    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( ch->GetMount() )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( ch->ChanceRollSkill(gsn_hide) )
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	learn_from_success( ch, gsn_hide );
    }
    else
	learn_from_failure( ch, gsn_hide );
    return;
}



/*
 * Contributed by Alander.
 */
void do_visible(CHAR_DATA *ch, const char* argument)
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    if (ch->race != RACE_HALFLING) /* Halfling has perm sneak SB */
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_recall(CHAR_DATA *ch, const char* argument)
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *opponent;

    location = NULL;

    if ( !IS_NPC(ch) && ch->pcdata->clan )
      location = get_room_index( ch->pcdata->clan->recall );

/*    if ( !IS_NPC( ch ) && !location && ch->level >= 5)
       location = get_room_index( 3009 ); */

    if( !IS_NPC( ch ) && ch->pcdata->recall_room != 0)
       location = get_room_index( ch->pcdata->recall_room );

    if ( !location )
       location = get_room_index( ROOM_VNUM_TEMPLE );


    if ( !location )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->GetInRoom() == location )
	return;

    if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NO_RECALL) )
    {
	send_to_char( "For some strange reason... nothing happens.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->affected_by, AFF_CURSE) )
    {
        send_to_char("You are cursed and cannot recall!\n\r", ch );
        return;
    }

    if ( ( opponent = ch->GetVictim() ) != NULL )
    {
	int lose;

	if ( number_bits( 1 ) == 0 || ( !IS_NPC( opponent ) && number_bits( 3 ) > 1 ) )
	{
	    ch->AddWait( 4 );
	    lose = (int) ( (exp_level(ch, ch->level+1) - exp_level(ch, ch->level)) * 0.1 );
	    if ( ch->GetConnection() )
	      lose /= 2;
	    gain_exp( ch, 0 - lose, FALSE);
	    ch_printf( ch, "You failed!  You lose %d exps.\n\r", lose );
	    return;
	}

	lose = (int) ( (exp_level(ch, ch->level+1) - exp_level(ch, ch->level)) * 0.2 );
	if ( ch->GetConnection() )
	  lose /= 2;
    lose += (int) ( ch->accumulated_exp*.75 );
    ch->accumulated_exp = 0;
	gain_exp( ch, 0 - lose, FALSE);
	ch_printf( ch, "You recall from combat!  You lose %d exps.\n\r", lose );
	ch->StopAllFights();//stop_fighting( ch, TRUE );
    }

    act( AT_ACTION, "$n disappears in a swirl of smoke.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    if ( ch->GetMount() )
    {
	char_from_room( ch->GetMount() );
	char_to_room( ch->GetMount(), location );
    }
    act( AT_ACTION, "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );

    return;
}


void do_aid(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Aid whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( ch->GetMount() )
	{
		send_to_char( "You can't do that while mounted.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Aid yourself?\n\r", ch );
		return;
	}

	if ( victim->position > POS_STUNNED )
	{
		act( AT_PLAIN, "$N doesn't need your help.", ch, NULL, victim,
			TO_CHAR);
		return;
	}

	if ( victim->hit <= -6 )
	{
		act( AT_PLAIN, "$N's condition is beyond natural help.", ch,
			NULL, victim, TO_CHAR);
		return;
	}

	ch->AddWait( skill_table[gsn_aid]->beats );
	if ( !IS_NPC(ch) && !ch->ChanceRoll( ch->pcdata->learned[gsn_aid] ) )
	{
		send_to_char( "You fail.\n\r", ch );
		learn_from_failure( ch, gsn_aid );
		return;
	}

	act( AT_SKILL, "You aid $N!",  ch, NULL, victim, TO_CHAR	);
	act( AT_SKILL, "$n aids $N!",  ch, NULL, victim, TO_NOTVICT );
	learn_from_success( ch, gsn_aid );
	adjust_favor( ch, 8, 1 );
	if ( victim->hit < 1 )
		victim->hit = 1;

	update_pos( victim );
	act( AT_SKILL, "$n aids you!", ch, NULL, victim, TO_VICT	);
	return;
}


void do_mount(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;

	if ( !IS_NPC(ch)
		&&	 ch->level < skill_table[gsn_mount]->skill_level[ch->Class] )
	{
		send_to_char(
			"I don't think that would be a good idea...\n\r", ch );
		return;
	}

	if ( ch->GetMount() )
	{
		send_to_char( "You're already mounted!\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, argument ) ) == NULL )
	{
		send_to_char( "You can't find that here.\n\r", ch );
		return;
	}

	if ( (!IS_NPC(victim) || (!IS_IMMORTAL(ch) && !IS_SET(victim->act, ACT_MOUNTABLE ))) )
	{
		send_to_char( "You can't mount that!\n\r", ch );
		return;
	}

	if ( IS_SET(victim->act, ACT_MOUNTED ) )
	{
		send_to_char( "That mount already has a rider.\n\r", ch );
		return;
	}

	if ( victim->position < POS_STANDING )
	{
		send_to_char( "Your mount must be standing.\n\r", ch );
		return;
	}

	if ( victim->position == POS_FIGHTING || victim->IsFighting() )
	{
		send_to_char( "Your mount is moving around too much.\n\r", ch );
		return;
	}

	ch->AddWait( skill_table[gsn_mount]->beats );
	if ( ch->ChanceRollSkill(gsn_mount) )
	{
		SET_BIT( victim->act, ACT_MOUNTED );
		ch->MountId = victim->GetId();
		act( AT_SKILL, "You mount $N.", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT );
		act( AT_SKILL, "$n mounts you.", ch, NULL, victim, TO_VICT );
		learn_from_success( ch, gsn_mount );
		ch->position = POS_MOUNTED;
	}
	else
	{
		act( AT_SKILL, "You unsuccessfully try to mount $N.", ch, NULL, victim, TO_CHAR );
		act( AT_SKILL, "$n unsuccessfully attempts to mount $N.", ch, NULL, victim, TO_NOTVICT );
		act( AT_SKILL, "$n tries to mount you.", ch, NULL, victim, TO_VICT );
		learn_from_failure( ch, gsn_mount );
	}
	return;
}


void do_dismount(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA *victim;

    if ( (victim = ch->GetMount()) == NULL )
    {
	send_to_char( "You're not mounted.\n\r", ch );
	return;
    }

    ch->AddWait( skill_table[gsn_mount]->beats );
    if ( ch->ChanceRollSkill(gsn_mount) )
    {
	act( AT_SKILL, "You dismount $N.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n skillfully dismounts $N.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SKILL, "$n dismounts you.  Whew!", ch, NULL, victim, TO_VICT );
	REMOVE_BIT( victim->act, ACT_MOUNTED );
	ch->MountId = 0;
	ch->position = POS_STANDING;
	learn_from_success( ch, gsn_mount );
    }
    else
    {
	act( AT_SKILL, "You fall off while dismounting $N.  Ouch!", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n falls off of $N while dismounting.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SKILL, "$n falls off your back.", ch, NULL, victim, TO_VICT );
	learn_from_failure( ch, gsn_mount );
	REMOVE_BIT( victim->act, ACT_MOUNTED );
	ch->MountId = 0;
	ch->position = POS_SITTING;
	global_retcode = damage( ch, ch, 1, TYPE_UNDEFINED,0 );
    }
    return;
}


/**************************************************************************/


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chances;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) && !IS_SET(victim->defenses, DFND_PARRY) )
      return FALSE;

    if ( IS_NPC(victim) )
    {
	/* Tuan was here.  :) */
	chances	= UMIN( 60, 2 * victim->level );
    }
    else
    {
	if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
	    return FALSE;
	chances	= (int) (victim->pcdata->learned[gsn_parry] / 2);
    }

    /* Put in the call to chance() to allow penalties for misaligned
       clannies.  */
    if ( !victim->ChanceRoll( chances + victim->level - ch->level ) )
    {
	learn_from_failure( victim, gsn_parry );
	return FALSE;
    }
    /*
	 * KSILYAN
	 * Moved the messages to parry_message in fight.c:
	 */

    learn_from_success( victim, gsn_parry );
    return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chances;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) && !IS_SET(victim->defenses, DFND_DODGE) )
      return FALSE;

    if ( IS_NPC(victim) )
	chances  = UMIN( 60, 2 * victim->level );
    else
        chances  = (int) (victim->pcdata->learned[gsn_dodge] / 2);

    /* Consider luck as a factor */
    if ( !victim->ChanceRoll(chances + victim->level - ch->level ) )
    {
	learn_from_failure( victim, gsn_dodge );
        return FALSE;
    }

	/* KSILYAN
	 * Moved the messages to dodge_message in fight.c
    if ( !IS_NPC(victim) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG) )
    act( AT_SKILL, "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );

    if ( !IS_NPC(ch) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG) )
    act( AT_SKILL, "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
	*/

    learn_from_success( victim, gsn_dodge );
    return TRUE;
}

void do_poison_weapon(CHAR_DATA *ch, const char* argument)
{
	OBJ_DATA *obj;
	OBJ_DATA *pobj;
	OBJ_DATA *wobj;
	char      arg [ MAX_INPUT_LENGTH ];

	if ( !IS_NPC( ch )
			&& ch->level < skill_table[gsn_poison_weapon]->skill_level[ch->Class] )
	{
		send_to_char( "What do you think you are, a thief?\n\r", ch );
		return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "What are you trying to poison?\n\r",    ch );
		return;
	}
	if ( ch->IsFighting())
	{
		send_to_char( "While you're fighting?  Nice try.\n\r", ch );
		return;
	}
	if ( ms_find_obj(ch) )
		return;

	if ( !( obj = get_obj_carry( ch, arg ) ) )
	{
		send_to_char( "You do not have that weapon.\n\r",      ch );
		return;
	}
	if ( obj->item_type != ITEM_WEAPON )
	{
		send_to_char( "That item is not a weapon.\n\r",        ch );
		return;
	}
	if ( IS_OBJ_STAT( obj, ITEM_POISONED ) )
	{
		send_to_char( "That weapon is already poisoned.\n\r",  ch );
		return;
	}
	/* Now we have a valid weapon...check to see if we have the powder. */
	for ( pobj = ch->first_carrying; pobj; pobj = pobj->next_content )
	{
		if ( pobj->pIndexData->vnum == OBJ_VNUM_BLACK_POWDER )
			break;
	}
	if ( !pobj )
	{
		send_to_char( "You do not have the black poison powder.\n\r", ch );
		return;
	}
	/* Okay, we have the powder...do we have water? */
	for ( wobj = ch->first_carrying; wobj; wobj = wobj->next_content )
	{
		if ( wobj->item_type == ITEM_DRINK_CON
				&& wobj->value[1]  >  0
				&& wobj->value[2]  == 0 )
			break;
	}
	if ( !wobj )
	{
		send_to_char( "You have no water to mix with the powder.\n\r", ch );
		return;
	}
	/* Great, we have the ingredients...but is the thief smart enough? */
	if ( !IS_NPC( ch ) && ch->getWis() < 16 )
	{
		send_to_char( "You can't quite remember what to do...\n\r", ch );
		return;
	}
	/* And does the thief have steady enough hands? */
	if ( !IS_NPC( ch )
			&& ( (ch->getDex() < 17) || ch->pcdata->condition[COND_DRUNK] > 0 ) )
	{
		send_to_char("Your hands aren't steady enough to properly mix the poison.\n\r", ch );
		return;
	}
	ch->AddWait( skill_table[gsn_poison_weapon]->beats );

	/* Check the skill percentage */
	separate_obj( pobj );
	separate_obj( wobj );
	if ( !ch->ChanceRollSkill(gsn_poison_weapon, -2) )
	{
		set_char_color( AT_RED, ch );
		send_to_char( "You failed and spill some on yourself.  Ouch!\n\r", ch );
		set_char_color( AT_GREY, ch );
		damage( ch, ch, ch->level, gsn_poison_weapon ,0);
		act(AT_RED, "$n spills the poison all over!", ch, NULL, NULL, TO_ROOM );
		extract_obj( pobj, TRUE );
		extract_obj( wobj, TRUE );
		learn_from_failure( ch, gsn_poison_weapon );
		return;
	}
	separate_obj( obj );
	/* Well, I'm tired of waiting.  Are you? */
	act(AT_RED, "You mix $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_CHAR );
	act(AT_RED, "$n mixes $p in $P, creating a deadly poison!",ch, pobj, wobj, TO_ROOM );
	act(AT_GREEN, "You pour the poison over $p, which glistens wickedly!",ch, obj, NULL, TO_CHAR  );
	act(AT_GREEN, "$n pours the poison over $p, which glistens wickedly!",ch, obj, NULL, TO_ROOM  );
	SET_BIT( obj->extra_flags, ITEM_POISONED );
	obj->cost *= ch->level;
	/* Set an object timer.  Don't want proliferation of poisoned weapons */
	obj->timer = 10 + ch->level;

	if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
		obj->timer *= 2;

	if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
		obj->timer *= 2;

	/* WHAT?  All of that, just for that one bit?  How lame. ;) */
	act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_CHAR );
	act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_ROOM );
	extract_obj( pobj, TRUE );
	extract_obj( wobj, TRUE );
	learn_from_success( ch, gsn_poison_weapon );
	return;
}

void do_scribe(CHAR_DATA *ch, const char* argument)
{
    OBJ_DATA *scroll;
    int sn;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    int mana;

    if ( IS_NPC(ch) )
        return;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_scribe]->skill_level[ch->Class] )
    {
	send_to_char( "A skill such as this requires more magical ability than that of your class.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
	send_to_char( "Scribe what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (sn = find_spell( ch, argument, TRUE )) < 0 )
    {
         send_to_char( "You have not learned that spell.\n\r", ch );
         return;
    }

    if ( skill_table[sn]->spell_fun == spell_null )
    {
        send_to_char( "That's not a spell!\n\r", ch );
        return;
    }

    if ( SPELL_FLAG(skill_table[sn], SF_NOSCRIBE) )
    {
        send_to_char( "You cannot scribe that spell.\n\r", ch );
        return;
    }

	mana = IS_NPC(ch)
		? 0
		: UMAX(skill_table[sn]->min_mana,
	           100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->Class] ) );

	if ( !IS_NPC(ch) )
	{
		if (ch->pcdata->learned[sn] > 90)
			mana -= (ch->pcdata->learned[sn] - 90);

		// minimum mana cost: 3 (then * 5 below) = 15
		if ( mana <= 2 )
			mana = 3;
	}

    mana *= 5;

    if ( !IS_NPC(ch) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

     if ( ( scroll = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
     {
	send_to_char( "You must be holding a blank scroll to scribe it.\n\r", ch );
	return;
     }

	/* We need to have both scroll vnums because someone had the bright idea
	 * of making two scribeable vnums!
	 * Ksilyan
	 */

     if( scroll->pIndexData->vnum != OBJ_VNUM_SCROLL_SCRIBING && scroll->pIndexData->vnum != OBJ_VNUM_SCROLL_SCRIBING2 )
     {
	send_to_char( "You must be holding a blank scroll to scribe it.\n\r", ch );
	return;
     }

     if ( ( scroll->value[1] != -1 )
     && ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING || scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING2) )
     {
	send_to_char( "That scroll has already been inscribed.\n\r", ch);
	return;
     }

     if ( !process_spell_components( ch, sn ) )
     {
	learn_from_failure( ch, gsn_scribe );
	ch->mana -= (mana / 2);
	return;
     }

     if ( !ch->ChanceRollSkill(gsn_scribe) || !ch->ChanceRollSkill(sn) )
			/* Ksilyan- made it harder to scribe if you're no good at the spell */
	{
       set_char_color ( AT_MAGIC, ch );
       send_to_char("You failed.\n\r", ch);
       learn_from_failure( ch, gsn_scribe );
       ch->mana -= (mana / 2);
       return;
     }

     scroll->value[1] = sn;
     scroll->value[0] = ch->level;
     sprintf(buf1, "%s scroll", skill_table[sn]->name_.c_str());
     scroll->shortDesc_ = aoran(buf1);

     sprintf(buf2, "A glowing scroll inscribed '%s' lies in the dust.",
                                              skill_table[sn]->name_.c_str());

     scroll->longDesc_ = buf2;

     sprintf(buf3, "scroll scribing %s", skill_table[sn]->name_.c_str());
     scroll->name_ = buf3;

     act( AT_MAGIC, "$n magically scribes $p.",   ch,scroll, NULL, TO_ROOM );
     act( AT_MAGIC, "You magically scribe $p.",   ch,scroll, NULL, TO_CHAR );

     learn_from_success( ch, gsn_scribe );

     ch->mana -= mana;

}

void do_brew(CHAR_DATA *ch, const char* argument)
{
	OBJ_DATA *potion;
	OBJ_DATA *fire;
	int sn;
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	int mana;
	bool found;

	if ( IS_NPC(ch) )
		return;

	if ( !IS_NPC(ch)
			&&   ch->level < skill_table[gsn_brew]->skill_level[ch->Class] )
	{
		send_to_char( "A skill such as this requires more magical ability than that of your class.\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' || !str_cmp(argument, "") )
	{
		send_to_char( "Brew what?\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( (sn = find_spell( ch, argument, TRUE )) < 0 )
	{
		send_to_char( "You have not learned that spell.\n\r", ch );
		return;
	}

	if ( skill_table[sn]->spell_fun == spell_null )
	{
		send_to_char( "That's not a spell!\n\r", ch );
		return;
	}

	if ( SPELL_FLAG(skill_table[sn], SF_NOBREW) )
	{
		send_to_char( "You cannot brew that spell.\n\r", ch );
		return;
	}

	mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
			100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->Class] ) );

	mana *=4;

	if ( !IS_NPC(ch) && ch->mana < mana )
	{
		send_to_char( "You don't have enough mana.\n\r", ch );
		return;
	}

	found = FALSE;

	for ( fire = ch->GetInRoom()->first_content; fire;
			fire = fire->next_content )
	{
		if( fire->item_type == ITEM_FIRE)
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
	{
		send_to_char(
				"There must be a fire in the room to brew a potion.\n\r", ch );
		return;
	}

	if ( ( potion = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
	{
		send_to_char(
				"You must be holding an empty flask to brew a potion.\n\r", ch );
		return;
	}

	if( potion->pIndexData->vnum != OBJ_VNUM_FLASK_BREWING )
	{
		send_to_char( "You must be holding an empty flask to brew a potion.\n\r", ch );
		return;
	}

	if ( ( potion->value[1] != -1 )
			&& ( potion->pIndexData->vnum == OBJ_VNUM_FLASK_BREWING ) )
	{
		send_to_char( "That's not an empty flask.\n\r", ch);
		return;
	}

	if ( !process_spell_components( ch, sn ) )
	{
		learn_from_failure( ch, gsn_brew );
		ch->mana -= (mana / 2);
		return;
	}

	if ( !ch->ChanceRollSkill(gsn_brew) )
	{
		set_char_color ( AT_MAGIC, ch );
		send_to_char("You failed.\n\r", ch);
		learn_from_failure( ch, gsn_brew );
		ch->mana -= (mana / 2);
		return;
	}

	potion->value[1] = sn;
	potion->value[0] = ch->level;
	sprintf(buf1, "%s potion", skill_table[sn]->name_.c_str());
	potion->shortDesc_ = aoran(buf1);

	sprintf(buf2, "A strange potion labelled '%s' sizzles in a glass flask.",
			skill_table[sn]->name_.c_str());

	potion->longDesc_ = buf2;

	sprintf(buf3, "flask potion %s", skill_table[sn]->name_.c_str());
	potion->name_ = buf3;

	act( AT_MAGIC, "$n brews up $p.",   ch,potion, NULL, TO_ROOM );
	act( AT_MAGIC, "You brew up $p.",   ch,potion, NULL, TO_CHAR );

	learn_from_success( ch, gsn_brew );

	ch->mana -= mana;

}

bool check_grip( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance;

	if ( !IS_AWAKE(victim) )
		return FALSE;

	if ( IS_NPC(victim) && !IS_SET(victim->defenses, DFND_GRIP) )
		return FALSE;

	if ( IS_NPC(victim) )
		chance  = UMIN( 60, 2 * victim->level );
	else
		chance  = (int) (victim->pcdata->learned[gsn_grip] / 2);

	/* Consider luck as a factor */
	chance += (2 * victim->getLuckBonus() );

	if ( number_percent( ) >= chance + victim->level - ch->level )
	{
		learn_from_failure( victim, gsn_grip );
		return FALSE;
	}
	act( AT_SKILL, "You evade $n's attempt to disarm you.", ch, NULL, victim, TO_VICT    );
	act( AT_SKILL, "$N holds $S weapon strongly, and is not disarmed.",
			ch, NULL, victim, TO_CHAR    );
	learn_from_success( victim, gsn_grip );
	return TRUE;
}

void do_circle(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	}

	one_argument( argument, arg );

	if ( ch->GetMount() )
	{
		send_to_char( "You can't circle while mounted.\n\r", ch );
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char( "Circle around whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "How can you sneak up on yourself?\n\r", ch );
		return;
	}

	if ( is_safe( ch, victim ) )
		return;

	if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
			||   ( obj->value[OBJECT_WEAPON_DAMAGETYPE] != DAMAGE_PIERCE) )
	{
		send_to_char( "You need to wield a piercing or stabbing weapon.\n\r", ch );
		return;
	}

	if ( !ch->IsFighting())
	{
		send_to_char( "You can't circle when you aren't fighting.\n\r", ch);
		return;
	}

	if ( !victim->IsFighting() )
	{
		send_to_char( "You can't circle around a person who is not fighting.\n\r", ch );
		return;
	}

        /*
	if ( victim->GetNumFighting() < 2 && !IS_AFFECTED(ch, AFF_BLIND) ) //victim either has to be fighting more than 1 person, or blind
	{
		act( AT_PLAIN, "You can't circle around them without a distraction.",
				ch, NULL, victim, TO_CHAR );
		return;
	}
*/
	ch->AddWait( skill_table[gsn_circle]->beats );
	if ( ch->ChanceRollSkill( gsn_circle, -victim->getLuckBonus() ) )
	{
		learn_from_success( ch, gsn_circle );
		ch->AddWait(     2 * PULSE_VIOLENCE );
		global_retcode = multi_hit( ch, victim, gsn_circle );
		adjust_favor( ch, 10, 1 );
	}
	else
	{
		learn_from_failure( ch, gsn_circle );
		ch->AddWait(     2 * PULSE_VIOLENCE );
		global_retcode = damage( ch, victim, 0, gsn_circle,0 );
	}
	return;
}

/* Berserk and HitAll. -- Altrag */
void do_berserk(CHAR_DATA *ch, const char* argument)
{
	AFFECT_DATA af;

	if ( !ch->IsFighting() )
	{
		send_to_char( "But you aren't fighting!\n\r", ch );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_BERSERK) )
	{
		send_to_char( "Your rage is already at its peak!\n\r", ch );
		return;
	}

	ch->AddWait( skill_table[gsn_berserk]->beats);
	if ( !ch->ChanceRollSkill(gsn_berserk) )
	{
		send_to_char( "You couldn't build up enough rage.\n\r", ch);
		learn_from_failure(ch, gsn_berserk);
		return;
	}
	af.type = gsn_berserk;
	/* Hmmm.. 10-20 combat rounds at level 50.. good enough for most mobs,
	   and if not they can always go berserk again.. shrug.. maybe even
	   too high. -- Altrag */
	af.duration = number_range(ch->level/5, ch->level*2/5);
	/* Hmm.. you get stronger when yer really enraged.. mind over matter
	   type thing.. */
	af.location = APPLY_STR;
	af.modifier = 1;
	af.bitvector = AFF_BERSERK;
	affect_to_char(ch, &af);
	send_to_char( "You start to lose control..\n\r", ch );
	learn_from_success(ch, gsn_berserk);
	return;
}

/* External from fight.c */
ch_ret one_hit	 ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) ;
void do_hitall(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	sh_int nvict = 0;
	sh_int nhit = 0;

	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_SAFE) )
	{
		send_to_char( "You cannot do that here.\n\r", ch);
		return;
	}

	if ( !ch->GetInRoom()->first_person )
	{
		send_to_char( "There's no one here!\n\r", ch );
		return;
	}

	for ( vch = ch->GetInRoom()->first_person; vch; vch = vch_next )
	{
		vch_next = vch->next_in_room;
		if ( is_same_group(ch, vch) || !can_see(ch, vch) || is_safe(ch, vch) )
			continue;
		if ( ++nvict > ch->level / 5 )
			break;
		if ( ch->ChanceRollSkill(gsn_hitall) )
		{
			nhit++;
			global_retcode = one_hit(ch, vch, TYPE_UNDEFINED);
		}
		else
			global_retcode = damage(ch, vch, 0, TYPE_UNDEFINED,0);
		/* Fireshield, etc. could kill ch too.. :>.. -- Altrag */
		if ( global_retcode == rCHAR_DIED || global_retcode == rBOTH_DIED
				||   char_died(ch) )
			return;
	}
	if ( !nvict )
	{
		send_to_char( "There's no one here!\n\r", ch );
		return;
	}
	ch->move = UMAX(0, ch->move-nvict*3+nhit);
	if ( nhit )
		learn_from_success(ch, gsn_hitall);
	else
		learn_from_failure(ch, gsn_hitall);
	return;
}

void do_scan(CHAR_DATA *ch, const char* argument)
{
	ROOM_INDEX_DATA *was_in_room;
	ExitData *pexit;
	sh_int dir = -1;
	sh_int dist;
	sh_int max_dist = 5;

	if ( argument[0] == '\0' )
	{
		dir = -1;
	}
	else if ( ( dir = get_door( argument ) ) == -1 )
	{
		send_to_char( "Scan in WHAT direction?\n\r", ch );
		return;
	}

	if ( dir != -1 )
	{
		was_in_room = ch->GetInRoom();
		act( AT_GREY, "Scanning $t...", ch, dir_name[dir], NULL, TO_CHAR );
		act( AT_GREY, "$n scans $t.", ch, dir_name[dir], NULL, TO_ROOM );

		if ( IS_NPC( ch )
				|| !ch->ChanceRollSkill(gsn_scan) )
		{
			act( AT_GREY, "You stop scanning $t as your vision blurs.", ch,
					dir_name[dir], NULL, TO_CHAR );
			learn_from_failure( ch, gsn_scan );
			return;
		}

		if ( IS_VAMPIRE( ch ) && !IS_IMMORTAL(ch) && !sun_broken() )
		{
			if ( time_info.hour <= sunset_hour()
					&& time_info.hour >= sunrise_hour() )
			{
				send_to_char( "You have trouble seeing clearly through all the light.\n\r", ch );
				max_dist = 1;
			}
		}

		if ( ( pexit = get_exit( ch->GetInRoom(), dir ) ) == NULL )
		{
			act( AT_GREY, "You can't see $t.", ch, dir_name[dir], NULL, TO_CHAR );
			return;
		}

		if ( ch->level < 50 ) max_dist--;
		if ( ch->level < 40 ) max_dist--;

		for ( dist = 1; dist <= max_dist; )
		{
			if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
			{
				if ( IS_SET( pexit->exit_info, EX_SECRET ) )
					/* Ksilyan- Why reveal secret doors!
					   act( AT_GREY, "Your view $t is blocked by a wall.", ch,
					   dir_name[dir], NULL, TO_CHAR );
					 */
					break;
				else
					act( AT_GREY, "Your view $t is blocked by a door.", ch,
							dir_name[dir], NULL, TO_CHAR );
				break;
			}
			if ( room_is_private( pexit->to_room )
					&& ch->level < sysdata.level_override_private )
			{
				act( AT_GREY, "Your view $t is blocked by a private room.", ch,
						dir_name[dir], NULL, TO_CHAR );
				break;
			}
			char_from_room( ch );
			char_to_room( ch, pexit->to_room );
			set_char_color( AT_RMNAME, ch );
			send_to_char( ch->GetInRoom()->name_.c_str(), ch );
			send_to_char( "\n\r", ch );
			show_list_to_char( ch->GetInRoom()->first_content, ch, FALSE, FALSE );
			show_char_to_char( ch->GetInRoom()->first_person, ch );

			switch( ch->GetInRoom()->sector_type )
			{
				default: dist++; break;
				case SECT_AIR:
						 if ( number_percent() < 80 ) dist++; break;
				case SECT_INSIDE:
				case SECT_FIELD:
				case SECT_UNDERGROUND:
						 dist++; break;
				case SECT_FOREST:
				case SECT_CITY:
				case SECT_DESERT:
				case SECT_HILLS:
						 dist += 2; break;
				case SECT_WATER_SWIM:
				case SECT_WATER_NOSWIM:
						 dist += 3; break;
				case SECT_MOUNTAIN:
				case SECT_UNDERWATER:
				case SECT_OCEANFLOOR:
						 dist += 4; break;
			}

			if ( dist >= max_dist )
			{
				act( AT_GREY, "Your vision blurs with distance and you see no "
						"farther $t.", ch, dir_name[dir], NULL, TO_CHAR );
				break;
			}
			if ( ( pexit = get_exit( ch->GetInRoom(), dir ) ) == NULL )
			{
				act( AT_GREY, "Your view $t is blocked by a wall.", ch,
						dir_name[dir], NULL, TO_CHAR );
				break;
			}
		}

		char_from_room( ch );
		char_to_room( ch, was_in_room );
		learn_from_success( ch, gsn_scan );
	}
	else
	{
		ExitData* pexit;
		act( AT_GREY, "You scan the area.", ch, NULL, NULL, TO_CHAR );
		act( AT_GREY, "$n scans the area.", ch, NULL, NULL, TO_ROOM );

		if ( IS_VAMPIRE( ch ) && !IS_IMMORTAL(ch) )
		{
			if ( time_info.hour < 21 && time_info.hour > 5 )
			{
				send_to_char( "You have trouble seeing clearly through all the light.\n\r", ch );
				return;
			}
		}

		for ( pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next )
		{
			dir = pexit->vdir;

			if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
			{
				if ( IS_SET( pexit->exit_info, EX_SECRET ) )
					act( AT_GREY, "Your view $t is blocked by a wall.", ch,
							dir_name[dir], NULL, TO_CHAR );
				else
					act( AT_GREY, "Your view $t is blocked by a door.", ch,
							dir_name[dir], NULL, TO_CHAR );
				break;
			}
			if ( room_is_private( pexit->to_room )
					&& ch->level < sysdata.level_override_private )
			{
				act( AT_GREY, "Your view $t is blocked by a private room.", ch,
						dir_name[dir], NULL, TO_CHAR );
				break;
			}

			if ( pexit->to_room && pexit->to_room->first_person )
			{
				/* It's dirty and lazy, I KNOW! :) */
				if ( pexit->vdir == DIR_UP || pexit->vdir == DIR_DOWN )
				{
					act(AT_GREY, "Looking $t you see:", ch,
							dir_name[pexit->vdir],
							NULL, TO_CHAR);
				}
				else
				{
					act(AT_GREY, "Looking to the $t you see:", ch,
							dir_name[pexit->vdir],
							NULL, TO_CHAR);
				}
				show_char_to_char( pexit->to_room->first_person, ch );
			}
		}
		learn_from_success( ch, gsn_scan );
	}
}

/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.   -Thoric
 */
CHAR_DATA *scan_for_victim( CHAR_DATA *ch, ExitData *pexit, char *name )
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *was_in_room;
	sh_int dist, dir;
	sh_int max_dist = 8;

	if ( IS_AFFECTED(ch, AFF_BLIND) || !pexit )
		return NULL;

	was_in_room = ch->GetInRoom();
	if ( IS_VAMPIRE(ch) && sun_broken()
			&& time_info.hour <=sunset_hour()
			&& time_info.hour >=sunrise_hour() )
		max_dist = 1;

	if ( ch->level < 50 ) --max_dist;
	if ( ch->level < 40 ) --max_dist;
	if ( ch->level < 30 ) --max_dist;

	for ( dist = 1; dist <= max_dist; )
	{
		if ( IS_SET(pexit->exit_info, EX_CLOSED) )
			break;

		if ( room_is_private( pexit->to_room )
				&&   ch->level < sysdata.level_override_private )
			break;

		char_from_room( ch );
		char_to_room( ch, pexit->to_room );

		if ( (victim=get_char_room(ch, name)) != NULL )
		{
			char_from_room(ch);
			char_to_room(ch, was_in_room);
			return victim;
		}

		switch( ch->GetInRoom()->sector_type )
		{
			default: dist++; break;
			case SECT_AIR:
					 if ( number_percent() < 80 ) dist++; break;
			case SECT_INSIDE:
			case SECT_FIELD:
			case SECT_UNDERGROUND:
					 dist++; break;
			case SECT_FOREST:
			case SECT_CITY:
			case SECT_DESERT:
			case SECT_HILLS:
					 dist += 2; break;
			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
					 dist += 3; break;
			case SECT_MOUNTAIN:
			case SECT_UNDERWATER:
			case SECT_OCEANFLOOR:
					 dist += 4; break;
		}

		if ( dist >= max_dist )
			break;

		dir = pexit->vdir;
		if ( (pexit=get_exit(ch->GetInRoom(), dir)) == NULL )
			break;
	}

	char_from_room(ch);
	char_to_room(ch, was_in_room);

	return NULL;
}

void do_slice(CHAR_DATA *ch, const char* argument)
{
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *slice;
	bool found;
	MOB_INDEX_DATA *pMobIndex;
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	found = FALSE;


	if ( !IS_NPC(ch) && !IS_IMMORTAL(ch)
			&&   ch->level < skill_table[gsn_slice]->skill_level[ch->Class] )
	{
		send_to_char("You are not learned in this skill.\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char("From what do you wish to slice meat?\n\r", ch);
		return;
	}


	if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
			||   ( obj->value[3] != 1 && obj->value[3] != 2 && obj->value[3] != 3
				&& obj->value[3] != 11) )
	{
		send_to_char( "You need to wield a sharp weapon.\n\r", ch);
		return;
	}

	if ( (corpse = get_obj_here( ch, argument )) == NULL)
	{
		send_to_char("You can't find that here.\n\r", ch);
		return;
	}

	if (corpse->item_type != ITEM_CORPSE_NPC || corpse->weight < 20)
	{
		send_to_char("That is not a suitable source of meat.\n\r", ch);
		return;
	}

	if ( (pMobIndex = get_mob_index((int) -(corpse->cost) )) == NULL )
	{
		bug("Can not find mob for value[2] of corpse, do_slice", 0);
		return;
	}

	if ( corpse->value[3] < 5 ) {
		send_to_char("There isn't enough meat left on that corpse.\r\n", ch);
		return;
	}

	if ( get_obj_index(OBJ_VNUM_SLICE) == NULL )
	{
		bug("Vnum 24 not found for do_slice!", 0);
		return;
	}

	if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) && !ch->ChanceRollSkill(gsn_slice) )
	{
		send_to_char("You fail to slice the meat properly.\n\r", ch);
		learn_from_failure(ch, gsn_slice); /* Just in case they die :> */
		if ( number_percent() + (ch->getDex() - 13) < 10)
		{
			act(AT_BLOOD, "You cut yourself!", ch, NULL, NULL, TO_CHAR);
			damage(ch, ch, ch->level, gsn_slice,0);
		}
		return;
	}

	slice = create_object( get_obj_index(OBJ_VNUM_SLICE), 0 );

	sprintf(buf, "meat fresh slice %s", pMobIndex->playerName_.c_str());
	slice->name_ = buf;

	sprintf(buf, "a slice of raw meat from %s", pMobIndex->shortDesc_.c_str());
	slice->shortDesc_ = buf;

	sprintf(buf1, "A slice of raw meat from %s lies on the ground.", pMobIndex->shortDesc_.c_str());
	slice->longDesc_ = buf1;

	act( AT_BLOOD, "$n cuts a slice of meat from $p.", ch, corpse, NULL, TO_ROOM);
	act( AT_BLOOD, "You cut a slice of meat from $p.", ch, corpse, NULL, TO_CHAR);

	obj_to_char(slice, ch);
	corpse->value[3] -= 5;
	learn_from_success(ch, gsn_slice);
	return;
}

void do_assist(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Assist whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How about fleeing instead?\n\r", ch );
	return;
    }

    if ( ( fch = victim->GetVictim() ) == NULL )
    {
	send_to_char( "They are not fighting right now.\n\r", ch );
	return;
    }

    ch->AddWait( 1*PULSE_VIOLENCE);
    multi_hit(ch, fch, TYPE_UNDEFINED);
    return;
}

void do_meditate(CHAR_DATA*ch, const char* argument)
{
    if ( IS_NPC(ch) ) {
        return;
    }

    if ( !IS_NPC(ch) && !IS_IMMORTAL(ch)
    &&   ch->level < skill_table[gsn_meditate]->skill_level[ch->Class] )
    {
        send_to_char("You are not learned in this skill.\r\n", ch);
        return;
    }

    if ( ch->position < POS_RESTING || ch->position > POS_STANDING
         || ch->position == POS_FIGHTING)
    {
        switch ( ch->position ) {
            case POS_SLEEPING:
                act(AT_SOCIAL, "You snore loudly.", ch, NULL, NULL, TO_CHAR);
                act(AT_SOCIAL, "$n snores loudly.", ch, NULL, NULL, TO_ROOM);
                break;
            case POS_MOUNTED:
                send_to_char("You attempt to tie yourself in a knot, but"
                             " almost fall off of your mount!\r\n", ch);
                break;
            case POS_DEAD:
                send_to_char("In another life, maybe.\r\n", ch);
                break;
            case POS_MORTAL:
            case POS_INCAP:
            case POS_STUNNED:
                send_to_char("It's difficult to concentrate when so much blood"
                             " is spurting from your body!\r\n", ch);
                break;
            case POS_FIGHTING:
                send_to_char("Sitting down right now would not be a good idea.\r\n",
                        ch);
                break;
            case POS_SHOVE:
            case POS_DRAG:
            default:
                send_to_char("Yeah.\r\n", ch);
                break;
        }
        return;
    }

    if ( get_timer(ch, TIMER_RECENTFIGHT) > 0 )
    {
        set_char_color(AT_RED, ch);
	    send_to_char("Your adrenaline is pumping too hard to do that!\n\r",ch);
        return;
    }

    if ( ch->mental_state > 10 ) {
        send_to_char("Your mind is too messed up to meditate right now.\r\n",
                ch);
        return;
    }

    if ( ch->pcdata->condition[COND_DRUNK] > 10 ) {
        send_to_char("*Hic*\r\n", ch);
        return;
    }

    if ( ch->pcdata->condition[COND_THIRST] < 10 ) {
        send_to_char("You are a tad bit too thirsty to concentrate right now.\r\n", ch);
        return;
    }

    if ( ch->pcdata->condition[COND_FULL] < 10 ) {
        send_to_char("It seems like days since you've eaten.\r\n", ch);
        return;
    }

    act(AT_ACTION, "$n ties $mself into a knot, and closes $s eyes.",
            ch, NULL, NULL, TO_ROOM);
    act(AT_ACTION, "You tie yourself into a knot, and close your eyes.",
            ch, NULL, NULL, TO_CHAR);

    learn_from_success(ch, gsn_meditate);

    ch->position = POS_MEDITATING;
}

void do_cook(CHAR_DATA* ch, const char* argument)
{
    OBJ_DATA *meat;
    OBJ_DATA *fire;

    if ( argument[0] == '\0' ) {
        send_to_char("What is it you wish to cook?\r\n", ch);
        return;
    }

    if ( !(meat = get_obj_here(ch, argument)) ) {
        send_to_char("You don't have that meat.\r\n", ch);
        return;
    }

    if ( meat->item_type != ITEM_MEAT ) {
        send_to_char("That is not meat!\r\n", ch);
        return;
    }

    separate_obj(meat);

    for ( fire = ch->GetInRoom()->first_content; fire; fire = fire->next_content ) {
        if ( fire->item_type == ITEM_FIRE )
            break;
    }

    if ( !fire || fire->item_type != ITEM_FIRE ) {
        send_to_char("There is nothing to cook the meat on.\r\n", ch);
        return;
    }

    act(AT_ACTION, "$n squats down in front of the fire, and starts to cook $p.",
            ch, meat, NULL, TO_ROOM);
    act(AT_ACTION, "You squat down in front of the fire, and start to cook $p.",
            ch, meat, NULL, TO_CHAR);

    if ( (!IS_NPC(ch) && !IS_IMMORTAL(ch) && !ch->ChanceRollSkill(gsn_cook))
            || meat->value[4] > COOKED_RAW )
    {
        if ( number_percent() + (ch->getDex() - 13)< 50) {
            act(AT_FIRE, "$n accidently drops $p in the fire, and then falls in after it!",
                ch, meat, NULL, TO_ROOM);
            act(AT_FIRE, "You accidently drop $p in the fire, and then fall in after it!",
                ch, meat, NULL, TO_CHAR);
            damage(ch, ch, ch->level, gsn_cook,0);
            meat->value[4] = COOKED_CHARED;
        } else {
            act(AT_ACTION, "$n burns $p into an inedible mess.", ch, meat, NULL, TO_ROOM);
            act(AT_ACTION, "You burn $p into an inedible mess.", ch, meat, NULL, TO_CHAR);
            meat->value[4] = COOKED_CHARED;
        }
        learn_from_failure(ch, gsn_cook);

        meat->shortDesc_ = "a charred piece of meat";
        meat->longDesc_ = "A charred piece of meat lies on the ground.";
        meat->name_ = "meat charred";
        meat->item_type = ITEM_TRASH;
    } else {
        act(AT_ACTION, "$n finishes cooking $p.", ch, meat, NULL, TO_ROOM);
        act(AT_ACTION, "You finish cooking $p.",  ch, meat, NULL, TO_CHAR);
        meat->value[4] += COOKED_MEDIUM;
        learn_from_success(ch, gsn_cook);

        meat->shortDesc_ = "a cooked piece of meat";
        meat->longDesc_ = "A cooked piece of meat lies on the ground, collecting dust.";
        meat->name_ = "piece meat cooked";
    }
}



/*
 * Search inventory for an appropriate projectile to fire.
 * Also search open quivers.                    -Thoric
 */
OBJ_DATA *find_projectile( CHAR_DATA *ch, int type )
{
	OBJ_DATA *quiver, *projectile;

	quiver = get_eq_char(ch, WEAR_QUIVER);

	if (quiver)
	{
		for (projectile = quiver->last_content; projectile; projectile = projectile->prev_content)
		{
			if (projectile->item_type == ITEM_PROJECTILE
					&& projectile->value[OBJECT_PROJECTILE_AMMOTYPE] == type)
				return projectile;
		}
	}

	return NULL;

    /* KSILYAN
		Commented this out; now we have a quiver wear location so we don't
		have to search for one.
    OBJ_DATA *obj, *obj2;

	for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
    if ( can_see_obj(ch, obj) )
    {
        if ( obj->item_type == ITEM_QUIVER && !IS_SET(obj->value[1], CONT_CLOSED) )
        {
        for ( obj2 = obj->last_content; obj2; obj2 = obj2->prev_content )
        {
            if ( obj2->item_type == ITEM_PROJECTILE
            &&   obj2->value[3] == type )
            return obj2;
        }
        }
        if ( obj->item_type == ITEM_PROJECTILE && obj->value[3] == type )
        return obj;
    }
    }*/

    return NULL;
}


ch_ret spell_attack( int, int, CHAR_DATA *, void * );

sh_int GetCountInQuiver(OBJ_DATA * quiver)
{
	OBJ_DATA * projectile;
	sh_int count = 0;

	if (!quiver)
	{
		bug("GetCountInQuiver: NULL quiver!");
		return -1;
	}

	count = 0;
	for (projectile = quiver->first_content; projectile; projectile = projectile->next_content)
	{
		count += projectile->condition;
	}

	return count;
}

/* Ksilyan:
 * This function takes the projectile, decreases the pile (condition)
 * by one, and if it's empty it removes it from the game.
 */

void ProjectileDecreaseCondition(OBJ_DATA * projectile)
{
	projectile->condition--;

	if (projectile->condition <= 0)
	{
		if (projectile->GetInObj())
			obj_from_obj(projectile);
		if (projectile->GetCarriedBy())
			obj_from_char(projectile);
		extract_obj(projectile, TRUE);
	}
}


/* Ksilyan:
 * This function assumes that gsn is a ranged weapon gsn.
 */

int GetRangedProficiency(CHAR_DATA * ch, int gsn)
{
	if (IS_NPC(ch))
		return UMAX(5, ch->level*2 - 5);

	return ((int) (ch->pcdata->learned[skill_lookup("fire")] / 2)) + ((int) (ch->pcdata->learned[gsn]));
}

/*
 * Perform the actual attack on a victim            -Thoric
 * Adapted to Darkstone by Ksilyan.
 */
ch_ret ranged_got_target( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon,
		OBJ_DATA *projectile, sh_int dist, sh_int dt, const char *stxt, sh_int color )
{
	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_SAFE) )
	{
		/* safe room, bubye projectile */
		if ( projectile )
		{
			ch_printf(ch, "Your %s is blasted from existance by a godly presense.", myobj(projectile) );
			act( color, "A godly presence smites $p!", ch, projectile, NULL, TO_ROOM );
			extract_obj(projectile, TRUE);
		}
		else
		{
			ch_printf(ch, "Your %s is blasted from existance by a godly presense.", stxt );
			act( color, "A godly presence smites $t!", ch, aoran(stxt), NULL, TO_ROOM );
		}
		return rNONE;
	}

	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_SENTINEL)
			&&   ch->GetInRoom() != victim->GetInRoom() )
	{
		/* I'm not sure I want them to learn anything at all from
		shooting at sentinel mobs.  After all, they could sit shooting
		at, say, the innkeeper, learning how to shoot, with no danger */
		if ( projectile )
		{
			/* learn_from_failure( ch, gsn_missile_weapons ); */

			ProjectileDecreaseCondition(projectile);
		}
		return damage( ch, victim, 0, dt , DAMAGE_MSG_ARROW);
	}

	if ( projectile )
		global_retcode = projectile_hit(ch, victim, weapon, projectile, dist );
	else
		global_retcode = spell_attack( dt, ch->level, ch, victim );

	return global_retcode;
}

/*
 * Generic use ranged attack function           -Thoric & Tricops
 * Adapted to Darkstone by Ksilyan.
 */
ch_ret ranged_attack( CHAR_DATA *ch, const char *argument, OBJ_DATA *weapon,
              OBJ_DATA *projectile, sh_int dt, sh_int range )
{
	CHAR_DATA *vch = NULL, *victim = NULL;
	ExitData *pexit = NULL;
	ROOM_INDEX_DATA *was_in_room;
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char temp[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	SkillType *skill = NULL;
	sh_int dir = -1, dist = 0, color = AT_GREY;
	const char *dtxt = "somewhere";
	const char *stxt = "burst of energy";
	int prof_gsn = -1;

	if ( argument && argument[0] != '\0' && argument[0] == '\'')
	{
		one_argument( argument, temp );
		argument = temp;
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg1);

	if ( arg[0] == '\0' )
	{
		if (ch->IsAttacking())
		{
			vch = ch->GetVictim();
		}
		else
		{
			send_to_char( "Where? At whom?\n\r", ch );
			return rNONE;
		}
	}

	if ( !projectile )
	{
		send_to_char("This shouldn't happen, but this isn't working out.\r\n", ch);
		send_to_char("Error code: ranged_attack:1. Notify an immortal.\r\n", ch);
		log_string("ranged_attack: NULL projectile!");
		return rNONE;
	}

	if ( weapon )
		prof_gsn = get_weapon_prof_gsn(weapon->value[OBJECT_WEAPON_WEAPONTYPE]);

	if ( !IS_NPC(ch) )
	{
		if ( prof_gsn == -1 )
		{
			send_to_char("This should not happen, but this isn't working out.\r\n", ch);
			send_to_char("Error code: ranged_attack:2. Notify an immortal.\r\n", ch);
			log_string("ranged_attack: prof_gsn == -1");
			return rNONE;
		}
		if ( ch->pcdata->learned[prof_gsn] <= 0 )
		{
			send_to_char("You don't know how to use that weapon.\r\n", ch);
			return rNONE;
		}
	}

	/* search for either victim here, or exit */
	if (!vch) /* if we already have a victim (because we're fighting someone) don't do following */
	if ( (vch = get_char_room(ch, arg)) == NULL )
	{
		/* We didn't find a person named arg. So, either we have exit+person
		 * or just exit.
		 */
		/* get an exit */
		if ( (pexit = find_door(ch, arg, TRUE)) == NULL )
		{
			send_to_char("Aim in what direction?\r\n", ch);
			return rNONE;
		}
		else
		{
			dir = pexit->vdir;
		}

		if ( IS_VALID_SN(dt) )
		{
			skill = skill_table[dt];
		}

		if ( pexit && !pexit->to_room )
		{
			send_to_char( "Are you expecting to fire through a wall?\n\r", ch );
			return rNONE;
		}

		/* Check for obstruction */
		if ( pexit && IS_SET(pexit->exit_info, EX_CLOSED) )
		{
			if ( IS_SET(pexit->exit_info, EX_SECRET)
					|| IS_SET(pexit->exit_info, EX_DIG)
					|| IS_SET(pexit->exit_info, EX_HIDDEN) )
			{
				send_to_char( "Are you expecting to fire through a wall?\n\r", ch );
			}
			else
			{
				send_to_char( "Are you expecting to fire through a door?\n\r", ch );
			}
			return rNONE;
		}

		vch = NULL;
		if ( pexit && arg1[0] != '\0' )
		{
			if ( (vch=scan_for_victim(ch, pexit, arg1)) == NULL )
			{
				send_to_char( "You cannot see your target.\n\r", ch );
				return rNONE;
			}

			/* can't properly target someone heavily in battle */
			if ( vch->GetNumFighting() > max_fight(vch) )
			{
				send_to_char( "There is too much activity there for you to get a clear shot.\n\r", ch );
				return rNONE;
			}
		}
	}

	if ( vch )
	{
		if ( vch && is_safe(ch, vch) )
		{
			act( AT_PLAIN, "You cannot attack $N.", ch, vch, NULL, TO_CHAR);
			return rNONE;
		}
	}

	was_in_room = ch->GetInRoom();

	if ( projectile )
	{
		separate_obj(projectile);
		if ( weapon )
		{
			if ( dir == -1 )
			{
				/* The only way to not have a dir is to have a victim in this room.
				 * So we can safely assume that vch is not null.
				 */
				act( AT_SKILL, "You fire $p at $N.", ch, projectile, vch, TO_CHAR );
				act( AT_SKILL, "$n fires $p at $N.", ch, projectile, vch, TO_NOTVICT );
				act( AT_SKILL, "$n fires $p at you!", ch, projectile, vch, TO_VICT );
			}
			else
			{
				act( AT_SKILL, "You fire $p $T.",      ch, projectile, dir_name[dir], TO_CHAR );
				act( AT_SKILL, "$n fires $p $T.",      ch, projectile, dir_name[dir], TO_ROOM );
			}
		}
		else
		{
			act( AT_SKILL, "You throw $p $T.",     ch, projectile, dir_name[dir], TO_CHAR );
			act( AT_SKILL, "$n throw $p $T.",      ch, projectile, dir_name[dir], TO_ROOM );
		}
	}
	else if ( skill )
	{
		if ( skill->nounDamage_.length() > 0 )
			stxt = skill->nounDamage_.c_str();
		else
			stxt = skill->name_.c_str();

		/* a plain "spell" flying around seems boring */
		if ( !str_cmp(stxt, "spell") )
		{
			stxt = "magical burst of energy";
		}

		if ( skill->type == SKILL_SPELL )
		{
			color = AT_MAGIC;
			act( AT_MAGIC, "You release $t $T.",        ch, aoran(stxt), dir_name[dir], TO_CHAR );
			act( AT_MAGIC, "$n releases $s $t $T.",     ch,       stxt,  dir_name[dir], TO_ROOM );
		}
	}
	else
	{
		bug( "Ranged_attack: no projectile, no skill dt %d", dt );
		return rNONE;
	}

	/* assign scanned victim */
	victim = vch;

	if (pexit)
	{
		/* This means that we have a direction to go. */
		/* reverse direction text from move_char */
		dtxt = rev_exit(pexit->vdir);
	}

	if (victim && victim->GetInRoom() == ch->GetInRoom())
		return ranged_got_target( ch, victim, weapon, projectile, 0, dt, stxt, color );

	while ( TRUE )
	{
		char_from_room(ch);
		char_to_room(ch, pexit->to_room);

		if ( dist >= range )
		{
			/* If we're out of movement then flop! */

			if ( projectile )
			{
				if (dir != -1) /* last ditch security!! */
					act( color, "Your $t falls harmlessly to the ground to the $T.",
							ch, myobj(projectile), dir_name[dir], TO_CHAR );
				else
					act( color, "Your $t falls harmlessly to the ground.",
							ch, myobj(projectile), NULL, TO_CHAR );

				act( color, "$p flies in from $T and falls harmlessly to the ground here.",
						ch, projectile, dtxt, TO_ROOM );

				ProjectileDecreaseCondition(projectile);
			}
			else
			{
				act( color, "Your $t fizzles out harmlessly to the $T.", ch, stxt, dir_name[dir], TO_CHAR );
				act( color, "$t flies in from $T and fizzles out harmlessly.",
						ch, aoran(stxt), dtxt, TO_ROOM );
			}
			break;
		}

		dist += movement_loss[ch->GetInRoom()->sector_type];

		pexit = get_exit( ch->GetInRoom(), dir );

		if ( pexit && IS_SET(pexit->exit_info, EX_CLOSED) )
		{
			/* whadoyahknow, the door's closed */
			if ( projectile )
			{
				sprintf(buf,"You see your %s hit a door in the distance to the %s.",
						myobj(projectile), dir_name[dir] );

				act( color, buf, ch, NULL, NULL, TO_CHAR );

				sprintf(buf,"$p flies in from %s and hits the %sern door.",
						dtxt, dir_name[dir] );
				act( color, buf, ch, projectile, NULL, TO_ROOM );

				ProjectileDecreaseCondition(projectile);
			}
			else
			{
				sprintf(buf, "You see your %s hit a door in the distance to the %s.",
						stxt, dir_name[dir] );
				act( color, buf, ch, NULL, NULL, TO_CHAR );

				sprintf(buf, "%s flies in from %s and implants itself solidly in the %sern door.",
						aoran(stxt), dtxt, dir_name[dir] );
				buf[0] = UPPER(buf[0]);
				act( color, buf, ch, NULL, NULL, TO_ROOM );
			}
			break;
		}

		if ( !victim || (victim && victim->GetInRoom() != ch->GetInRoom() ) )
		{
			char buf[MAX_STRING_LENGTH];
			/* We're passing through a room with other people in it.
			 * There's still a small chance we might hit someone else.
			 */
			int chance = 85;
			bool SpecifiedVictim = (victim ? TRUE : FALSE);

			if ( !IS_NPC(ch) )
			{
				chance -= UMAX(0, (75 - GetRangedProficiency(ch, prof_gsn)));
			}

			if ( ch->getName() == "Tobi" )
			{
				sprintf(buf, "Tobi has %d chance of hitting someone else in this room.", chance);
				log_string(buf);
			}

			/* If we're not aiming for anyone in particular, it's easier... */
			if (number_percent() > (victim ? chance : 40) )
			{
				chance = 80;
				for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next_in_room )
				{
					if (vch == ch)
						continue;
					if (number_percent() < chance)
					{
						/* We just found a victim */
						victim = vch;

						if ( is_safe(ch, victim) )
						{
							char_from_room(ch);
							char_to_room(ch, was_in_room);
							return rNONE;
						}
						else
						{
							if ( !IS_NPC(victim) && !IS_NPC(ch) && !SpecifiedVictim )
							{
								char buf[MAX_STRING_LENGTH];

								sprintf(buf,"PK: %s hit %s by accident with a ranged attack.\n\r",
										ch->getName().c_str(), victim->getName().c_str() );
								to_channel(buf,CHANNEL_MONITOR,"Monitor",0);
								log_string(buf);
							}
							break;
						}
					}
					else
					{
						/* How often are people in a line?
						 * therefore we reduce the probability of hitting
						 * anyone by a fair amount every time we go by
						 * someone.
						 */
						chance = chance / 2;
					}
				}
			}
		}

		/* In the same room as our victim? */
		if ( victim && ch->GetInRoom() == victim->GetInRoom() )
		{
			if ( projectile )
			{
				act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
			}
			else
			{
				act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );
			}

			/* get back before the action starts */
			char_from_room(ch);
			char_to_room(ch, was_in_room);

			return ranged_got_target( ch, victim, weapon, projectile,
					dist, dt, stxt, color );
		}


		/* If we get here, that means that we haven't hit anything at all, and we move on. */

		if ( pexit == NULL )
		{
			if ( projectile )
			{
				act( color, "Your $t hits a wall and bounces harmlessly to the ground to the $T.",
						ch, myobj(projectile), dir_name[dir], TO_CHAR );
				act( color, "$p strikes the $Tern wall and falls harmlessly to the ground.",
						ch, projectile, dir_name[dir], TO_ROOM );

				ProjectileDecreaseCondition(projectile);
			}
			else
			{
				act( color, "Your $t harmlessly hits a wall to the $T.",
				ch, stxt, dir_name[dir], TO_CHAR );
				act( color, "$t strikes the $Tern wall and falls harmlessly to the ground.",
				ch, aoran(stxt), dir_name[dir], TO_ROOM );
			}
			break;
		}

		if ( projectile )
		{
			act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
			act( color, "$p flies off to the $T.", ch, projectile, dir_name[dir], TO_ROOM );
		}
		else
		{
			act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );
			act( color, "$t flies off to the $T.", ch, aoran(stxt), dir_name[dir], TO_ROOM );
		}

	}

	char_from_room( ch );
	char_to_room( ch, was_in_room );

	return rNONE;
}

/*
 * Fire <direction> <target>
 *
 * Fire a projectile from a missile weapon (bow, crossbow, etc)
 *
 * Design by Thoric, coding by Thoric and Tricops.
 *
 * Support code (see projectile_hit(), quiver support, other changes to
 * fight.c, etc by Thoric.
 *
 * Modified for Darkstone by Ksilyan.
 */
void do_fire(CHAR_DATA *ch, const char* argument)
{
	OBJ_DATA *arrow;
	OBJ_DATA *bow;
	sh_int max_dist;
	int ammo_type;
	int power;

	if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_SAFE ) ) {
		set_char_color( AT_MAGIC, ch );
		send_to_char( "A magical force prevents you from attacking.\n\r", ch );
		return;
	}

	bow = get_eq_char(ch, WEAR_WIELD);

	if ( !bow || !IS_RANGED_WEAPON_OBJ(bow) )
	{
		send_to_char( "You are not wielding a missile weapon.\n\r", ch );
		return;
	}

	if ( get_eq_char(ch, WEAR_DUAL_WIELD) || get_eq_char(ch, WEAR_SHIELD)
			|| get_eq_char(ch, WEAR_HOLD) ) {
		ch_printf(ch, "You need at least one hand free to fire %s.\n\r", bow->shortDesc_.c_str());
		return;
	}

	ammo_type = bow->value[OBJECT_WEAPON_AMMOTYPE];

	if ( ammo_type < 0 || ammo_type > MAX_AMMO_TYPE ) {
		send_to_char("Your weapon is a missile weapon, but it doesn't have the right ammo type.\r\n", ch);
		send_to_char("Let an immortal know, please.\r\n", ch);
		return;
	}

	if ( (arrow=find_projectile(ch, ammo_type)) == NULL ) {
		switch( ammo_type ) {
			case AMMO_LONGARROW:
				send_to_char("Some long arrows would help things along...\r\n", ch);
				break;
			case AMMO_SHORTARROW:
				send_to_char("It might help if you had a short arrow or two.\r\n", ch);
				break;
			case AMMO_BOLT:
				send_to_char("You can't expect to shoot a crossbow without bolts.\r\n", ch);
				break;
			case AMMO_STONE:
				send_to_char("Planning on slinging air, or are you going to get a stone?\r\n", ch);
				break;
		}
		return;
	}

	/* Distance takes the range of the arrow and multiplies it by the bow's power. */
	power = bow->value[OBJECT_WEAPON_POWER];
	max_dist = short( ((power*power) / 10000.0) * arrow->value[OBJECT_PROJECTILE_RANGE] );

	/* Add wait state to fire for pkill, etc... */
	ch->AddWait( 6 );

	/* handle the ranged attack */
	ranged_attack( ch, argument, bow, arrow, TYPE_HIT, max_dist );

	return;
}

void do_throw(CHAR_DATA* ch, const char* argument) {
    //OBJ_DATA* arrow;
    OBJ_DATA* bow;
    sh_int max_dist;

    if ( argument[0] == '\0' || !str_cmp(argument, " ") ) {
        send_to_char( "Throw what where at whom?\n\r", ch );
        return;
    }

    if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_SAFE ) ) {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "A magical force prevents you from attacking.\n\r", ch );
        return;
    }

    /*
     * Find the projectile weapon
     */
    if ( (bow=get_eq_char(ch, WEAR_WIELD)) != NULL ) {
        if ( !(bow->item_type == ITEM_WEAPON) ||
              (bow->value[3] != WEAPON_THROWINGKNIFE && bow->value[3] != WEAPON_THROWINGSPEAR) ) {
            bow = NULL;
        }
    }

    if ( !bow && (bow=get_eq_char(ch, WEAR_DUAL_WIELD)) ) {
        if ( bow->item_type == ITEM_WEAPON ||
           (bow->value[3] != WEAPON_THROWINGKNIFE && bow->value[3] != WEAPON_THROWINGSPEAR) ) {
            send_to_char("The item you wish to throw must be your primary weapon.\r\n", ch);
            return;
        }

        bow = NULL;
    }

    if ( !bow ) {
        send_to_char( "You are not wielding a throwable weapon.\n\r", ch );
        return;
    }

    max_dist = URANGE(1, bow->value[4], 5);

    ch->AddWait( 6);

    ranged_attack( ch, argument, bow, bow, TYPE_HIT + bow->value[OBJECT_WEAPON_WEAPONTYPE], max_dist);
}

