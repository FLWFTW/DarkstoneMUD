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
 *			    Battle & death module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"
#include "connection.h"
#include "commands.h"

#include "World.h"

extern char		lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *	gch_prev;
extern CHAR_DATA *      lastplayer;
/*test*/

/*
 * Local functions.
 */

void	parry_message  (CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam_msg) ;
void	dodge_message  (CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam_msg) ;

void	dam_message	( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt , int dam_msg);
void	death_cry	 ( CHAR_DATA *ch ) ;
void	group_gain	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
int	xp_compute	( CHAR_DATA *gch, CHAR_DATA *victim, int levelPenalty = 0 );
int	align_compute	 ( CHAR_DATA *gch, CHAR_DATA *victim ) ;
ch_ret	one_hit		 ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) ;
int	obj_hitroll	 ( OBJ_DATA *obj ) ;
bool	dual_flip = FALSE;


/* Ksilyan:
 * A very cheat hack to make it work for now. I promise, this will
 * be fixed at some point.
 * The need here is to make shields NOT work when triggered by
 * projectile_hit ... so we have to fool damage into thinking that
 * shields are off (which it does to prevent endless shield recursions.
 */
    static bool bNoShields = FALSE;

/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
         OBJ_DATA *obj;

         if ( ( obj = get_eq_char( ch, WEAR_WIELD ) 	)
         &&   (IS_SET( obj->extra_flags, ITEM_POISONED) )	)
                  return TRUE;

         return FALSE;

}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
      return FALSE;

    return TRUE;
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hating || ch->hating->who != victim )
      return FALSE;

    return TRUE;
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
      return FALSE;

    return TRUE;
}

void stop_hunting( CHAR_DATA *ch )
{
	if ( ch->hunting )
	{
		delete ch->hunting;
		ch->hunting = NULL;
	}
	return;
}

void stop_hating( CHAR_DATA *ch )
{
	if ( ch->hating )
	{
		delete ch->hating;
		ch->hating = NULL;
	}
	return;
}

void stop_fearing( CHAR_DATA *ch )
{
	if ( ch->fearing )
	{
		delete ch->fearing;
		ch->fearing = NULL;
	}
	return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
      stop_hunting( ch );

    ch->hunting = new HuntHateFear;
    ch->hunting->name_ = victim->getName();
    ch->hunting->who  = victim;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( ch->hating )
		stop_hating( ch );

	ch->hating = new HuntHateFear;
	ch->hating->name_ = victim->getName();
	ch->hating->who  = victim;
	return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( ch->fearing )
		stop_fearing( ch );

	ch->fearing = new HuntHateFear;
	ch->fearing->name_ = victim->getName();
	ch->fearing->who  = victim;
	return;
}

/*
 * Get the current armor class for a vampire based on time of day
 */
sh_int VAMP_AC( const CHAR_DATA * ch )
{
  if ( IS_VAMPIRE( ch ) || ch->isOutside() )
  {
    switch(weather_info.sunlight)
    {
    case SUN_DARK:
      return -8;
    case SUN_RISE:
      return 5;
    case SUN_LIGHT:
      return 10;
    case SUN_SET:
      return 2;
    default:
      return 0;
    }
  }
  else
    return 0;
}

uint max_fight( CHAR_DATA *ch )
{
    return 8;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 */
void violence_update( void )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	CHAR_DATA *lst_ch;
	CHAR_DATA *victim;
	CHAR_DATA *rch, *rch_next;
	AFFECT_DATA *paf, *paf_next;
	TIMER	*timer, *timer_next;
	ch_ret	   retcode;
	int 	   x, attacktype = 0, cnt;
	SkillType	*skill;

	lst_ch = NULL;
	for ( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
	{
		set_cur_char( ch );

		if ( ch == first_char && ch->prev )
		{
			bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
			ch->prev = NULL;
		}

		gch_prev	= ch->prev;

		if ( gch_prev && gch_prev->next != ch )
		{
			sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.",
				ch->getName().c_str() );
			bug( buf, 0 );
			bug( "Short-cutting here", 0 );
			ch->prev = NULL;
			gch_prev = NULL;
			do_shout( ch, "Tarin says, 'Prepare for the worst!'" );
		}

		/*
		* See if we got a pointer to someone who recently died...
		* if so, either the pointer is bad... or it's a player who
		* "died", and is back at the healer...
		* Since he/she's in the char_list, it's likely to be the latter...
		* and should not already be in another fight already
		*/
		if ( char_died(ch) )
			continue;

		/*
		 * See if we got a pointer to some bad looking data...
		 */
		/* removed by Ksilyan - seems no longer relevant.
		if ( !ch->GetInRoom() || !ch->name )
		{
			log_string( "violence_update: bad ch record!  (Shortcutting.)" );
			sprintf( buf, "ch: %d  ch->GetInRoom(): %d  ch->prev: %d  ch->next: %d",
				(int) ch, (int) ch->GetInRoom(), (int) ch->prev, (int) ch->next );
			log_string( buf );
			log_string( lastplayercmd );
			if ( lst_ch )
				sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
				(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
			else
				strcpy( buf, "lst_ch: NULL" );
			log_string( buf );
			gch_prev = NULL;
			continue;
		}
		end of remove by Ksilyan*/

		/*
		* Experience gained during battle deceases as battle drags on
		*/

		// Removed by Ksilyan - it seems that fighting->duration is never set!
		// Hmm... oh well.
		#if 0
		if ( ch->IsFighting() )
		{
			if ( (++ch->fighting->duration % 24) == 0 )
				ch->fighting->xp = ((ch->fighting->xp * 9) / 10);
		}
		#endif


		for ( timer = ch->first_timer; timer; timer = timer_next )
		{
			timer_next = timer->next;
			if ( --timer->count <= 0 )
			{
				if ( timer->type == TIMER_DO_FUN )
				{
					int tempsub;

					tempsub = ch->substate;
					ch->substate = timer->value;
					(timer->do_fun)( ch, "" );
					if ( char_died(ch) )
						break;
					ch->substate = tempsub;
				}
				extract_timer( ch, timer );
			}
		}

		if ( char_died(ch) )
			continue;

		/*
		 * We need spells that have shorter durations than an hour.
		 * So a melee round sounds good to me... -Thoric
		 */
		for ( paf = ch->first_affect; paf; paf = paf_next )
		{
			paf_next	= paf->next;

			if ( paf->duration > 0 )
			{
				paf->duration--;
			}
			else if ( paf->duration < 0 )
			{
			}
			else
			{
				if ( !paf_next
					||	  paf_next->type != paf->type
					||	  paf_next->duration > 0 )
				{
					skill = get_skilltype(paf->type);
					if ( paf->type > 0 && skill && skill->msgOff_.length() > 0 )
					{
						set_char_color( AT_WEAROFF, ch );
						ch->sendText( skill->msgOff_.str() );
						send_to_char( "\n\r", ch );
					}
				}
				if (paf->type == gsn_possess)
				{
					ch->GetConnection()->CurrentCharId = ch->GetConnection()->OriginalCharId;
					ch->GetConnection()->OriginalCharId = 0;
					ch->GetConnection()->GetCharacter()->SwitchedCharId = 0;
					ch->GetConnection()->GetCharacter()->SetConnection( ch->GetConnection() );
					ch->SetConnection( NULL );
				}
				// We need to remove the spell memory from both people.
				list<SpellMemory*>::iterator itor;

				// Create a local copy of the list to loop through.
				list<SpellMemory*> localList = ch->spellMemory_;

				for (itor = localList.begin(); itor != localList.end(); itor++)
				{
					SpellMemory * spell = *itor;

					if (spell->Spell == paf)
					{
						spell->Target->spellMemory_.remove(spell);
						spell->Caster->spellMemory_.remove(spell);
						break;
					}

				}

				affect_remove( ch, paf );
			}
		}

		// check for victim, and paralysis, to handle fight code
		if ( ( victim = ch->GetVictim() ) == NULL
			||	 IS_AFFECTED( ch, AFF_PARALYSIS ) )
			continue;

		// Start fight code

		retcode = rNONE;

		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_SAFE ) )
		{
			sprintf( buf, "violence_update: %s fighting %s in a SAFE room.",
				ch->getName().c_str(), victim->getName().c_str() );
			log_string( buf );
			//stop_fighting( ch, TRUE );
			ch->StopAttacking();
		}
		else if ( IS_AWAKE(ch) && ch->GetInRoom() == victim->GetInRoom() )
			retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
		else
			ch->StopAttacking();//stop_fighting( ch, FALSE );

		if ( char_died(ch) )
			continue;

		if ( retcode == rCHAR_DIED
			|| ( victim = ch->GetVictim() ) == NULL )
			continue;

		/*
		 *  Mob triggers
		 */
		rprog_rfight_trigger( ch );
		if ( char_died(ch) )
			continue;
		mprog_hitprcnt_trigger( ch, victim );
		if ( char_died(ch) )
			continue;
		mprog_fight_trigger( ch, victim );
		if ( char_died(ch) )
			continue;

		/*
		 * NPC special attack flags				-Thoric
		 */
		if ( IS_NPC(ch) )
		{
			cnt = 0;
			if ( ch->attacks )
			{
				for ( ;; )
				{
					if ( cnt++ > 10 )
					{
						attacktype = 0;
						break;
					}
					x = number_range( 7, 31 );
					attacktype = 1 << x;
					if ( IS_SET( ch->attacks, attacktype ) )
						break;
				}
			}

			if ( 30 + (ch->level/4) < number_percent( ) )
				attacktype = 0;
			switch( attacktype )
			{
				case ATCK_BASH:
					do_bash( ch, "" );
					retcode = global_retcode;
					break;
				case ATCK_STUN:
					do_stun( ch, "" );
					retcode = global_retcode;
					break;
				case ATCK_GOUGE:
					do_gouge( ch, "" );
					retcode = global_retcode;
					break;
				case ATCK_FEED:
					do_gouge( ch, "" );
					retcode = global_retcode;
					break;
				case ATCK_DRAIN:
					retcode = spell_energy_drain( skill_lookup( "energy drain" ), ch->level, ch, victim );
					break;
				case ATCK_FIREBREATH:
					retcode = spell_fire_breath( skill_lookup( "fire breath" ), ch->level, ch, victim );
					break;
				case ATCK_FROSTBREATH:
					retcode = spell_frost_breath( skill_lookup( "frost breath" ), ch->level, ch, victim );
					break;
				case ATCK_ACIDBREATH:
					retcode = spell_acid_breath( skill_lookup( "acid breath" ), ch->level, ch, victim );
					break;
				case ATCK_LIGHTNBREATH:
					retcode = spell_lightning_breath( skill_lookup( "lightning breath" ), ch->level, ch, victim );
					break;
				case ATCK_GASBREATH:
					retcode = spell_gas_breath( skill_lookup( "gas breath" ), ch->level, ch, victim );
					break;
				case ATCK_SPIRALBLAST:
					retcode = spell_spiral_blast( skill_lookup( "spiral blast" ),
						ch->level, ch, victim );
					break;
				case ATCK_POISON:
					retcode = spell_poison( gsn_poison, ch->level, ch, victim );
					break;
				case ATCK_NASTYPOISON:
				/*
				retcode = spell_nasty_poison( skill_lookup( "nasty poison" ), ch->level, ch, victim );
					*/
					break;
				case ATCK_GAZE:
				/*
				retcode = spell_gaze( skill_lookup( "gaze" ), ch->level, ch, victim );
					*/
					break;
				case ATCK_BLINDNESS:
					retcode = spell_blindness( gsn_blindness, ch->level, ch, victim );
					break;
				case ATCK_CAUSESERIOUS:
					retcode = spell_cause_serious( skill_lookup( "cause serious" ), ch->level, ch, victim );
					break;
				case ATCK_EARTHQUAKE:
					retcode = spell_earthquake( skill_lookup( "earthquake" ), ch->level, ch, victim );
					break;
				case ATCK_CAUSECRITICAL:
					retcode = spell_cause_critical( skill_lookup( "cause critical" ), ch->level, ch, victim );
					break;
				case ATCK_CURSE:
					retcode = spell_curse( skill_lookup( "curse" ), ch->level, ch, victim );
					break;
				case ATCK_FLAMESTRIKE:
					retcode = spell_flamestrike( skill_lookup( "flamestrike" ), ch->level, ch, victim );
					break;
				case ATCK_HARM:
					retcode = spell_harm( skill_lookup( "harm" ), ch->level, ch, victim );
					break;
				case ATCK_FIREBALL:
					retcode = spell_fireball( skill_lookup( "fireball" ), ch->level, ch, victim );
					break;
				case ATCK_COLORSPRAY:
					retcode = spell_colour_spray( skill_lookup( "colour spray" ), ch->level, ch, victim );
					break;
				case ATCK_WEAKEN:
					retcode = spell_weaken( skill_lookup( "weaken" ), ch->level, ch, victim );
					break;
			}
			if ( retcode == rCHAR_DIED || (char_died(ch)) )
				continue;

			/*
			 * NPC special defense flags				-Thoric
			 */
			cnt = 0;
			if ( ch->defenses )
			{
				for ( ;; )
				{
					if ( cnt++ > 10 )
					{
						attacktype = 0;
						break;
					}
					x = number_range( 2, 18 );
					attacktype = 1 << x;
					if ( IS_SET( ch->defenses, attacktype ) )
						break;
				}
			}
			if ( 50 + (ch->level/4) < number_percent( ) )
				attacktype = 0;

			switch( attacktype )
			{
				case DFND_CURELIGHT:
					act( AT_MAGIC, "$n mutters a few incantations...and looks a little better.", ch, NULL, NULL, TO_ROOM );
					retcode = spell_smaug( skill_lookup( "cure light" ), ch->level, ch, ch );
					break;
				case DFND_CURESERIOUS:
					act( AT_MAGIC, "$n mutters a few incantations...and looks a bit better.", ch, NULL, NULL, TO_ROOM );
					retcode = spell_smaug( skill_lookup( "cure serious" ), ch->level, ch, ch );
					break;
				case DFND_CURECRITICAL:
					act( AT_MAGIC, "$n mutters a few incantations...and looks a bit healthier.", ch, NULL, NULL, TO_ROOM );
					retcode = spell_smaug( skill_lookup( "cure critical" ), ch->level, ch, ch );
					break;
				case DFND_DISPELMAGIC:
					act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
					retcode = spell_dispel_magic( skill_lookup( "dispel magic" ), ch->level, ch, victim );
					break;
				case DFND_DISPELEVIL:
					act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
					retcode = spell_dispel_evil( skill_lookup( "dispel evil" ), ch->level, ch, victim );
					break;
				case DFND_SANCTUARY:
					if ( !IS_AFFECTED(victim, AFF_SANCTUARY) )
					{
						act( AT_MAGIC, "$n mutters a few incantations...", ch, NULL, NULL, TO_ROOM );
						retcode = spell_smaug( skill_lookup( "sanctuary" ), ch->level, ch, ch );
					}
					else
						retcode = rNONE;
					break;
			}
			if ( retcode == rCHAR_DIED || (char_died(ch)) )
				continue;
		}
		/*
		 * Fun for the whole family!
		 */
		for ( rch = ch->GetInRoom()->first_person; rch; rch = rch_next )
		{
			rch_next = rch->next_in_room;

			if ( IS_AWAKE(rch) && !rch->IsAttacking() )
			{
				/*
				 * PC's auto-assist others in their group.
				 */
				if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) )
				{
					if ( ( !IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM) )
						&&	 is_same_group(ch, rch) )
						multi_hit( rch, victim, TYPE_UNDEFINED );
					continue;
				}

				// NPCs will defend their master, if they are guardians
				// This also works if they are mounted...
				//    -Ksilyan
				if ( IS_NPC(rch) && IS_SET(rch->act, ACT_GUARDIAN) )
				{
					gTheWorld->LogCodeString(string(rch->getName().c_str()) + " is a guardian!");
					if ( rch->MasterId == ch->GetId() // ch is rch's master
						|| ch->MountId == rch->GetId() ) // rch is ch's mount

				{
					multi_hit( rch, victim, TYPE_UNDEFINED );
					continue;
				}
				}

				/*
				* NPC's assist NPC's of same type or 12.5% chance regardless.
				*/
				if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM)
					&&	!IS_SET(rch->act, ACT_NOASSIST) )
				{
					if ( char_died(ch) )
						break;
					if ( rch->pIndexData == ch->pIndexData
						||	 number_bits( 3 ) == 0 )
					{
						CHAR_DATA *vch;
						CHAR_DATA *target;
						int number;

						target = NULL;
						number = 0;
						for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next )
						{
							if ( can_see( rch, vch )
								&&	 is_same_group( vch, victim )
								&&	 number_range( 0, number ) == 0 )
							{
								target = vch;
								number++;
							}
						}

						if ( target )
							multi_hit( rch, target, TYPE_UNDEFINED );
					}
				}
			}
		}
    }

    return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	int chance;
	int 	dual_bonus;
	ch_ret	retcode;

	/* add timer if player is attacking another player */
	if ( !IS_NPC(ch) && !IS_NPC(victim) )
		add_timer( ch, TIMER_RECENTFIGHT, 20, NULL, 0 );

	if ( !IS_NPC(ch) && IS_SET( ch->act, PLR_NICE ) && !IS_NPC( victim ) && !in_arena(ch) )
		return rNONE;

	if( !IS_NPC(ch) && !IS_NPC(victim) && !in_arena(ch) && lastplayer )
	{
		if(ch->getName().c_str() != lastplayer->getName().c_str()
				&& ch->GetVictim() != victim
				&& lastplayer->getName().c_str()==victim->getName().c_str())
		{
			char buf[MAX_STRING_LENGTH];

			sprintf(buf,"PK: %s provoked by %s's %s.\n\r"
					,ch->getName().c_str(),lastplayer->getName().c_str(),lastplayercmd);
			to_channel(buf,CHANNEL_MONITOR,"Monitor",0);
		}
	}

	retcode = one_hit(ch, victim, dt);


	if ( (retcode != rNONE) && (retcode != rAVOIDED) )
		return retcode;

	if ( ch->GetVictim() != victim || dt == gsn_backstab || dt == gsn_circle)
		return rNONE;

	/* Very high chance of hitting compared to chance of going berserk */
	/* 40% or higher is always hit.. don't learn anything here though. */
	/* -- Altrag */
	chance = IS_NPC(ch) ? 100 : (ch->pcdata->learned[gsn_berserk]*5/2);
	if ( IS_AFFECTED(ch, AFF_BERSERK) && number_percent() < chance )
	{
		if ( (retcode = one_hit( ch, victim, dt )) != rNONE || ch->GetVictim() != victim )
			return retcode;
	}

	if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
	{
		dual_bonus = IS_NPC(ch) ? (ch->level / 10) : (ch->pcdata->learned[gsn_dual_wield] / 10);
		chance = IS_NPC(ch) ? ch->level : ch->pcdata->learned[gsn_dual_wield];
		if ( number_percent( ) < chance )
		{
			learn_from_success( ch, gsn_dual_wield );
			retcode = one_hit( ch, victim, dt );
			if ( ( (retcode != rNONE) && (retcode != rAVOIDED) ) || ( ch->GetVictim() != victim ) )
				return retcode;
		}
		else
		{
			learn_from_failure( ch, gsn_dual_wield );
		}
	}
	else
		dual_bonus = 0;

	if ( ch->move < 10 )
		dual_bonus = -20;

	/*
	 * NPC predetermined number of attacks			-Thoric
	 */
	if ( IS_NPC(ch) && ch->numattacks > 0 )
	{
		for ( chance = 0; chance <= ch->numattacks; chance++ )
		{
			retcode = one_hit( ch, victim, dt );
			if ( ( (retcode != rNONE) && (retcode != rAVOIDED) ) || ( ch->GetVictim() != victim ) )
				return retcode;
		}
		return retcode;
	}

	{
		chance = IS_NPC(ch) ? ch->level
			: (int) ((ch->pcdata->learned[gsn_second_attack]+dual_bonus)/1.5);
		if ( number_percent( ) < chance )
		{
			learn_from_success( ch, gsn_second_attack );
			retcode = one_hit( ch, victim, dt );
			if ( ( (retcode != rNONE) && (retcode != rAVOIDED) ) || ( ch->GetVictim() != victim ) )
				return retcode;
		}
		else {
			learn_from_failure( ch, gsn_second_attack );
		}

		chance = IS_NPC(ch) ? ch->level
			: (int) ((ch->pcdata->learned[gsn_third_attack]+(dual_bonus*1.5))/2);
		if ( number_percent( ) < chance )
		{
			learn_from_success( ch, gsn_third_attack );
			retcode = one_hit( ch, victim, dt );
			if ( ( (retcode != rNONE) && (retcode != rAVOIDED) ) || ( ch->GetVictim() != victim ) )
				return retcode;
		}
		else
		{
			learn_from_failure( ch, gsn_third_attack );
		}

		chance = IS_NPC(ch) ? ch->level
			: (int) ((ch->pcdata->learned[gsn_fourth_attack]+(dual_bonus*2))/3);
		if ( number_percent( ) < chance )
		{
			learn_from_success( ch, gsn_fourth_attack );
			retcode = one_hit( ch, victim, dt );
			if ( ( (retcode != rNONE) && (retcode != rAVOIDED) ) || ( ch->GetVictim() != victim ) )
				return retcode;
		}
		else
		{
			learn_from_failure( ch, gsn_fourth_attack );
		}

		chance = IS_NPC(ch) ? ch->level
			: (int) ((ch->pcdata->learned[gsn_fifth_attack]+(dual_bonus*3))/4);
		if ( number_percent( ) < chance )
		{
			learn_from_success( ch, gsn_fifth_attack );
			retcode = one_hit( ch, victim, dt );
			if ( ( (retcode != rNONE) && (retcode != rAVOIDED) ) || ( ch->GetVictim() != victim ) )
				return retcode;
		}
		else
		{
			learn_from_failure( ch, gsn_fifth_attack );
		}

		retcode = rNONE;

		chance = IS_NPC(ch) ? (int) ((ch->level+dual_bonus) / 2) : 0;
		if ( number_percent( ) < chance ) {
			retcode = one_hit( ch, victim, dt );
		} else {
		}

		if ( retcode == rNONE )
		{
			int move;

			if ( !IS_AFFECTED(ch, AFF_FLYING)
					&&   !IS_AFFECTED(ch, AFF_FLOATING) )
			{
				if ( !ch->GetInRoom() )
				{
					ostringstream os;
					os << __FILE__ << "::" << __LINE__ << ":: null in_room!" << endl;
					gTheWorld->LogString( os.str() );
					return rNONE;
				}
				move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, ch->GetInRoom()->sector_type)] );
			}
			else
				move = encumbrance( ch, 1 );
			if ( ch->move )
				ch->move = UMAX( 0, ch->move - move );
		}
	}

	return retcode;
}

/*
 * Weapon types, haus
 */

/* move check for weapon type out of weapon_prof_bonus_check */
int get_weapon_prof_gsn(int weapon_type) {
	char buf[MAX_INPUT_LENGTH];
	switch(weapon_type)
	{
		default:
			sprintf(buf, "Unknown weapon type: %d . Object vnum unavailable.", weapon_type);
			bug(buf);
			return -1;
			break;
		case WEAPON_NONE:
			sprintf(buf, "Unknown weapon type: %d . Object vnum unavailable.", weapon_type);
			bug(buf);
			return -1;
			break;
		case WEAPON_PUGILISM:
			return gsn_pugilism;
		case WEAPON_LONGBLADE:
			return gsn_long_blades;
		case WEAPON_SHORTBLADE:
			return gsn_short_blades;
		case WEAPON_FLEXIBLE:
			return gsn_flexible_arms;
		case WEAPON_BLUDGEON:
			return gsn_bludgeons;
		case WEAPON_POLEARM:
			return gsn_polearms;
		case WEAPON_LONGBOW:
			return gsn_long_bows;
		case WEAPON_SHORTBOW:
			return gsn_short_bows;
		case WEAPON_CROSSBOW:
			return gsn_cross_bows;
		case WEAPON_SLING:
			return gsn_slings;
		case WEAPON_THROWINGSPEAR:
			return gsn_throwing_spears;
		case WEAPON_THROWINGKNIFE:
			return gsn_throwing_daggers;
	}
}

int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
    int bonus;

    bonus = 0;	*gsn_ptr = -1;
    if ( !IS_NPC(ch) && ch->level > 5 && wield )
    {
        *gsn_ptr = get_weapon_prof_gsn(wield->value[OBJECT_WEAPON_WEAPONTYPE]);
	if ( *gsn_ptr != -1 )
	  bonus = (int) ((ch->pcdata->learned[*gsn_ptr] -50)/10);

       /* Reduce weapon bonuses for misaligned clannies.
       if ( IS_CLANNED(ch) )
       {
          bonus = bonus /
          ( 1 + abs( ch->alignment - ch->pcdata->clan->alignment ) / 1000 );
       }*/

	if ( IS_DEVOTED( ch ) )
	{
	   bonus = bonus - abs( ch->pcdata->favor ) / -100 ;
	}

    }
    return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll( OBJ_DATA *obj )
{
	int tohit = 0;
	AFFECT_DATA *paf;

	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	for ( paf = obj->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	return tohit;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
{
    sh_int lvl;

    if ( !IS_NPC(ch) )		/* players get much less effect */
    {
	lvl = UMAX( 1, (ch->level - 10) / 2 );
	if ( number_percent() + (victim->level - lvl) < 35 )
	  return lvl;
	else
	  return 0;
    }
    else
    {
	lvl = ch->level / 2;
	if ( number_percent() + (victim->level - lvl) < 70 )
	  return lvl;
	else
	  return 0;
    }
}

/*
 * KSILYAN
 * Meelee hit.
 * Attempt to whack a target, see if we hit or not.
 * True if hit, false if missed.
 *
 * The code is based on the old one_hit code.
 * At some point, the hit-miss determining will be
 * altogether removed from one_hit, and this function
 * will be checked before one_hit is called.
 */

bool MeeleeHit( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * Wielded, short HitBonus )
{
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int temp_hr;
    int diceroll;
    int	prof_bonus;
    int	prof_gsn;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->GetInRoom() != victim->GetInRoom() )
    {
    	return false;
    }

	if ( Wielded && IS_RANGED_WEAPON_OBJ(Wielded) )
	{
		/* Can't whack someone with your bow! */
		Wielded = NULL;
	}

	prof_bonus = weapon_prof_bonus_check( ch, Wielded, &prof_gsn );

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
	if ( IS_NPC(ch) )
	{
		thac0_00 = ch->mobthac0;
		thac0_32 =  0;
	}
	else
	{
		thac0_00 = class_table[ch->Class]->thac0_00;
		thac0_32 = class_table[ch->Class]->thac0_32;
	}

    /* changes to give knights a combat bonus when mounted - fiz */
    temp_hr = ch->getHitRoll() + HitBonus;

    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - temp_hr;
    victim_ac = UMAX( -19, (int) (victim->getAC() / 10) );

    /* If victim can't see weapon, AC penalty */
	if ( Wielded && !can_see_obj( victim, Wielded ) )
		victim_ac += 4;

	/* If victim can't see attacker, AC penalty */
	if ( !can_see( victim, ch) )
		victim_ac += 9;

	/* If attacker can't see victim, AC bonus */
    if ( !can_see( ch, victim ) )
		victim_ac -= 9;

    /*
     * "learning" between combatants.  Takes the intelligence difference,
     * and multiplies by the times killed to make up a learning bonus
     * given to whoever is more intelligent		-Thoric
     */
	if ( ch->GetVictim() == victim )
	{
		sh_int times = ch->GetTimesKilled(victim);

		if ( times )
		{
			sh_int intdiff = ch->getInt() - victim->getInt();

			if ( intdiff != 0 )
				victim_ac += (intdiff*times)/10;
		}
	}

	/* Weapon proficiency bonus */
	victim_ac += prof_bonus;

	/*
	 * The moment of excitement!
	 */
	while ( ( diceroll = number_bits( 5 ) ) >= 20 )
		;

	if ( diceroll == 0
	     || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
	{
		/* Miss. */
		if ( prof_gsn != -1 )
			learn_from_failure( ch, prof_gsn );
		tail_chain( );
		return false;
	}

    /*
     * Hit.
     */

	return true;
}

/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    int temp_hr;
    int dam, x;
    int diceroll;
    int attacktype, cnt;
    int	prof_bonus;
    int	prof_gsn;
	int dam_msg;
    ch_ret retcode;

	dam_msg = DAMAGE_MSG_DEFAULT;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->GetInRoom() != victim->GetInRoom() )
    {
    	return rVICT_DIED;
    }

    /*
     * Figure out the weapon doing the damage			-Thoric
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
		if ( dual_flip == FALSE )
		{
			dual_flip = TRUE;
			wield = get_eq_char( ch, WEAR_WIELD );
		}
		else
			dual_flip = FALSE;
	}
	else
		wield = get_eq_char( ch, WEAR_WIELD );

	if ( wield && IS_RANGED_WEAPON_OBJ(wield) )
	{
		/* Can't whack someone with your bow! */
		wield = NULL;
	}

	prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );

	/* make sure fight is already started */
	if ( ch->IsFighting()
			&&   dt == TYPE_UNDEFINED
			&&   IS_NPC(ch)
			&&   ch->attacks != 0 )
	{
		cnt = 0;
		for ( ;; )
		{
			x = number_range( 0, 6 );
			attacktype = 1 << x;
			if ( IS_SET( ch->attacks, attacktype ) )
				break;
			if ( cnt++ > 16 )
			{
				attacktype = 0;
				break;
			}
		}
		if ( attacktype == ATCK_BACKSTAB )
			attacktype = 0;
		if ( wield && number_percent( ) > 25 )
			attacktype = 0;
		switch ( attacktype )
		{
			default:
				break;
			case ATCK_BITE:
				do_bite( ch, "" );
				retcode = global_retcode;
				break;
			case ATCK_CLAWS:
				do_claw( ch, "" );
				retcode = global_retcode;
				break;
			case ATCK_TAIL:
				do_tail( ch, "" );
				retcode = global_retcode;
				break;
			case ATCK_STING:
				do_sting( ch, "" );
				retcode = global_retcode;
				break;
			case ATCK_PUNCH:
				do_punch( ch, "" );
				retcode = global_retcode;
				break;
			case ATCK_KICK:
				do_kick( ch, "" );
				retcode = global_retcode;
				break;
			case ATCK_TRIP:
				attacktype = 0;
				break;
		}
		if ( attacktype )
			return retcode;
	}

    if ( dt == TYPE_UNDEFINED )
    {
		dt = TYPE_HIT;
		if ( wield && wield->item_type == ITEM_WEAPON )
		{
			dt += wield->value[OBJECT_WEAPON_DAMAGETYPE];
	    	dam_msg = wield->value[OBJECT_WEAPON_DAMAGEMESSAGE];
		}
		else
		{
			dam_msg = DAMAGE_MSG_DEFAULT;
		}
	}


	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if ( IS_NPC(ch) )
	{
		thac0_00 = ch->mobthac0;
		thac0_32 =	0;
	}
	else
	{
		thac0_00 = class_table[ch->Class]->thac0_00;
		thac0_32 = class_table[ch->Class]->thac0_32;
	}

	/* changes to give knights a combat bonus when mounted - fiz */
	temp_hr = ch->getHitRoll();

	thac0	  = interpolate( ch->level, thac0_00, thac0_32 ) - temp_hr;
	victim_ac = UMAX( -19, (int) (victim->getAC() / 10) );

	/* If victim can't see weapon, AC penalty */
	if ( wield && !can_see_obj( victim, wield) )
		victim_ac += 4;

	/* If victim can't see attacker, AC penalty */
	if ( !can_see( victim, ch) )
		victim_ac += 9;

	/* If attacker can't see victim, AC bonus */
	if ( !can_see( ch, victim ) )
		victim_ac -= 9;

	/*
	 * "learning" between combatients.  Takes the intelligence difference,
	 * and multiplies by the times killed to make up a learning bonus
	 * given to whoever is more intelligent		-Thoric
	 */
	if ( ch->GetVictim() == victim )
	{
		sh_int times = ch->GetTimesKilled(victim);

		if ( times )
		{
			sh_int intdiff = ch->getInt() - victim->getInt();

			if ( intdiff != 0 )
				victim_ac += (intdiff*times)/10;
		}
	}

	/* Weapon proficiency bonus */
	victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
	while ( ( diceroll = number_bits( 5 ) ) >= 20 )
		;

	if ( diceroll == 0
		|| ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
	{
		/* Miss. */
		if ( prof_gsn != -1 )
			learn_from_failure( ch, prof_gsn );
		damage( ch, victim, 0, dt, dam_msg);
		tail_chain( );
		return rAVOIDED;
	}

    /*
     * Hit.
     * Calc damage.
     */

	if ( !wield )		/* dice formula fixed by Thoric */
		dam = number_range( ch->barenumdie, ch->baresizedie * ch->barenumdie );
	else
	{
		dam = number_range( wield->value[OBJECT_WEAPON_MINDAMAGE], wield->value[OBJECT_WEAPON_MAXDAMAGE] );

/* this was a debug hack -- Warp
       if(strcmp(capitalize(ch->name), "Warp") == 0) {
	  sprintf( log_buf, "Warp hits for %dd%d: %d", wield->value[1], wield->value[2], dam );
	  to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_ETERNAL);
       }
*/
    }

    /*
     * Bonuses.
     */
	dam += ch->getDamRoll();

	if ( prof_bonus )
		dam += prof_bonus / 4;

	if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
	{
		dam += (int) (dam * ch->pcdata->learned[gsn_enhanced_damage] / 120);
		learn_from_success( ch, gsn_enhanced_damage );
	}

	if ( !IS_AWAKE(victim) )
		dam *= 2;
	if ( dt == gsn_backstab )
		dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 8);

	if ( dt == gsn_circle )
		dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 16);

	if ( dam <= 0 )
		dam = 1;

	plusris = 0;

	if ( wield )
	{
		if ( IS_SET( wield->extra_flags, ITEM_MAGIC ) )
			dam = ris_damage( victim, dam, RIS_MAGIC );
		else
			dam = ris_damage( victim, dam, RIS_NONMAGIC );

		/*
		 * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
		 */
		plusris = obj_hitroll( wield );
	}
	else
		dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam )
    {
		int x, res, imm, sus, mod;

		if ( plusris )
			plusris = RIS_PLUS1 << UMIN(plusris, 7);

		/* initialize values to handle a zero plusris */
		imm = res = -1;  sus = 1;

		/* find high ris */
		for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
		{
			if ( IS_SET( victim->immune, x ) )
				imm = x;
			if ( IS_SET( victim->resistant, x ) )
				res = x;
			if ( IS_SET( victim->susceptible, x ) )
				sus = x;
		}
		mod = 10;
		if ( imm >= plusris )
			mod -= 10;
		if ( res >= plusris )
			mod -= 2;
		if ( sus <= plusris )
			mod += 2;

		/* check if immune */
		if ( mod <= 0 )
			dam = -1;
		if ( mod != 10 )
			dam = (dam * mod) / 10;
	}

	if ( prof_gsn != -1 )
	{
		if ( dam > 0 )
			learn_from_success( ch, prof_gsn );
		else
			learn_from_failure( ch, prof_gsn );
	}

	/* immune to damage */
	if ( dam == -1 )
	{
		if ( dt >= 0 && dt < top_sn )
		{
			SkillType *skill = skill_table[dt];
			bool found = FALSE;

			if ( skill->imm_char && skill->imm_char[0] != '\0' )
			{
				act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
				found = TRUE;
			}
			if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
			{
				act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
				found = TRUE;
			}
			if ( skill->imm_room && skill->imm_room[0] != '\0' )
			{
				act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
				found = TRUE;
			}
			if ( found )
			{
				return rNONE;
			}
		}
		dam = 0;
	}

	if ( (retcode = damage( ch, victim, dam, dt, dam_msg )) != rNONE )
	{
		return retcode;
	}
	if ( char_died(ch) )
	{
		return rCHAR_DIED;
	}
	if ( char_died(victim) )
	{
		return rVICT_DIED;
	}

	retcode = rNONE;
	if ( dam == 0 )
	{
		return retcode;
	}

	/* weapon spells	-Thoric */
	if ( wield
		&&	!IS_SET(victim->immune, RIS_MAGIC)
		&&	!IS_SET(victim->GetInRoom()->room_flags, ROOM_NO_MAGIC) )
	{
		AFFECT_DATA *aff;

		for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
			if ( aff->location == APPLY_WEAPONSPELL
				&&	 IS_VALID_SN(aff->modifier)
				&&	 skill_table[aff->modifier]->spell_fun )
				retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier,
#ifdef USE_OBJECT_LEVELS
				(wield->level+3)/3,
#else
				ch->level,
#endif
				ch, victim );
			if ( retcode != rNONE || char_died(ch) || char_died(victim) )
			{
				return retcode;
			}
			for ( aff = wield->first_affect; aff; aff = aff->next )
				if ( aff->location == APPLY_WEAPONSPELL
					&&	 IS_VALID_SN(aff->modifier)
					&&	 skill_table[aff->modifier]->spell_fun )
					retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier,
#ifdef USE_OBJECT_LEVELS
					(wield->level+3)/3,
#else
					ch->level,
#endif
					ch, victim );
				if ( retcode != rNONE || char_died(ch) || char_died(victim) )
				{
					return retcode;
				}
	}


	tail_chain( );
	return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage( CHAR_DATA *ch, sh_int dam, int ris )
{
   sh_int modifier;

   modifier = 10;
   if ( IS_SET(ch->immune, ris ) )
     modifier -= 10;
   if ( IS_SET(ch->resistant, ris ) )
     modifier -= 2;
   if ( IS_SET(ch->susceptible, ris ) )
     modifier += 2;
   if ( modifier <= 0 )
     return -1;
   if ( modifier == 10 )
     return dam;
   return (dam * modifier) / 10;
}

/*
 * Inflict damage from a hit.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_msg)
{
	char buf[MAX_STRING_LENGTH];
	bool npcvict, victim_is_ch, npcInanimate;
	int  xp_gain;
	OBJ_DATA *damobj;
	ch_ret retcode;
	sh_int dampmod;
	int	bodypart; /* Ksilyan- added support for hitting a specific body part */
	int percent;

	int init_gold, new_gold;
	int oldhit;
	int con;

	retcode = rNONE;

	if ( !ch )
	{
		bug( "Damage: null ch!", 0 );
		return rERROR;
	}
	if ( !victim )
	{
		bug( "Damage: null victim!", 0 );
		return rVICT_DIED;
	}

	if ( victim->position == POS_DEAD )
		return rVICT_DIED;

	npcvict = IS_NPC(victim);
	if ( npcvict && IS_SET(victim->act, ACT_INANIMATE) )
		npcInanimate = true;
	else
		npcInanimate = false;

	victim_is_ch = ch == victim;

	/*
	 * Check damage types for RIS				-Thoric
	 */
	if ( dam && dt != TYPE_UNDEFINED )
	{
		if ( IS_FIRE(dt) )
			dam = ris_damage(victim, dam, RIS_FIRE);
		else if ( IS_COLD(dt) )
			dam = ris_damage(victim, dam, RIS_COLD);
		else if ( IS_ACID(dt) )
			dam = ris_damage(victim, dam, RIS_ACID);
		else if ( IS_ELECTRICITY(dt) )
			dam = ris_damage(victim, dam, RIS_ELECTRICITY);
		else if ( IS_ENERGY(dt) )
			dam = ris_damage(victim, dam, RIS_ENERGY);
		else if ( IS_DRAIN(dt) )
			dam = ris_damage(victim, dam, RIS_DRAIN);
		else if ( dt == gsn_poison || IS_POISON(dt) )
			dam = ris_damage(victim, dam, RIS_POISON);

		if ( dam == -1 )
		{
			if ( dt >= 0 && dt < top_sn )
			{
				bool found = FALSE;
				SkillType *skill = skill_table[dt];

				if ( skill->imm_char && skill->imm_char[0] != '\0' )
				{
					act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
					found = TRUE;
				}
				if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
				{
					act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
					found = TRUE;
				}
				if ( skill->imm_room && skill->imm_room[0] != '\0' )
				{
					act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
					found = TRUE;
				}
				if ( found )
					return rNONE;
			}
			dam = 0;
		}
	}

	// inanimate mobs don't hate/hunt  - Ksilyan
	if ( dam && npcvict && ch != victim && !IS_SET(victim->act, ACT_INANIMATE) )
	{
		if ( !IS_SET( victim->act, ACT_SENTINEL ) )
		{
			if ( victim->hunting )
			{
				if ( victim->hunting->who != ch )
				{
					victim->hunting->name_ = ch->getName();
					victim->hunting->who  = ch;
				}
			}
			else
				start_hunting( victim, ch );
		}

		if ( victim->hating )
		{
			if ( victim->hating->who != ch )
			{
				victim->hating->name_ = ch->getName();
				victim->hating->who  = ch;
			}
		}
		else
			start_hating( victim, ch );
	}

	/*
	 * Why are we taking down damage done in player vs player?
	 *    -Ksilyan
	 */

	if ( !IS_NPC(ch) && !IS_NPC(victim) ) {
		dam = dam/4;
	}

	/*
	 * Stop up any residual loopholes.
	 */

	/*
	   KSILYAN
	   Again - why are we limiting damage? Seems that we want to apply all
	   bonuses, and not take any away.

	   maxdam = ch->level * 40;
	   if ( dt == gsn_backstab )
	   maxdam = ch->level * 80;
	   if ( dam > maxdam && !IS_IMMORTAL(ch) )
	   {
	   sprintf( buf, "Damage: %d more than %d points!", dam, maxdam );
	   bug( buf, dam );
	   sprintf(buf, "** %s (lvl %d) -> %s **",
	   ch->name,
	   ch->level,
	   victim->name);
	   bug( buf, 0 );
	   dam = maxdam;
	   }
	 */

	/* KSILYAN
	   Get the body part that is to be hit or missed. Do this now because
	   we might need this in parry/dodge (if we make even fancier messages...
	 */

	percent = number_percent();
	if (percent >= 1 && percent <= 10) // HEAD SHOT!!!
		bodypart = BODYPART_HEAD;
	else if (percent >= 11 && percent <= 14)
		bodypart = BODYPART_NECK;
	else if (percent >= 15 && percent <= 26)
		bodypart = BODYPART_LEFTARM;
	else if (percent >= 27 && percent <= 38)
		bodypart = BODYPART_RIGHTARM;
	else if (percent >= 39 && percent <= 63)
		bodypart = BODYPART_CHEST;
	else if (percent >= 64 && percent <= 68)
		bodypart = BODYPART_WAIST;
	else if (percent >= 69 && percent <= 78)
		bodypart = BODYPART_LEFTLEG;
	else if (percent >= 79 && percent <= 88)
		bodypart = BODYPART_RIGHTLEG;
	else if (percent >= 89 && percent <= 90)
		bodypart = BODYPART_LEFTFOOT;
	else if (percent >= 91 && percent <= 92)
		bodypart = BODYPART_RIGHTFOOT;
	else if (percent >= 93 && percent <= 96)
		bodypart = BODYPART_LEFTHAND;
	else if (percent >= 97 && percent <= 100)
		bodypart = BODYPART_RIGHTHAND;
	else
		bug("Umm, how did we get a number percent beyond 100 in body part searching??\n\r");

	if ( victim != ch )
	{
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if ( is_safe( ch, victim ) )
			return rNONE;

		if ( victim->position > POS_STUNNED )
		{
			// if the victim was not attacking already, then begin fighting...
			// UNLESS the victim is an inanimate "npc".
			if ( !victim->IsAttacking() && (!IS_NPC(victim) || !IS_SET(victim->act, ACT_INANIMATE) )  )
				victim->StartAttacking(ch);//set_fighting( victim, ch );
			if ( victim->IsFighting() )
				victim->position = POS_FIGHTING;
		}

		if ( victim->position > POS_STUNNED )
		{
			if ( !ch->IsAttacking() )
				ch->StartAttacking(victim);//set_fighting( ch, victim );

			/*
			 * If victim is charmed, ch might attack victim's master.
			 */
			if ( IS_NPC(ch)
					&&   npcvict
					&&   IS_AFFECTED(victim, AFF_CHARM)
					&&   victim->GetMaster() != NULL
					&&   victim->GetMaster()->GetInRoom() == ch->GetInRoom()
					&&   number_bits( 3 ) == 0 )
			{
				ch->StopAttacking();//stop_fighting( ch, FALSE );
				retcode = multi_hit( ch, victim->GetMaster(), TYPE_UNDEFINED );
				return retcode;
			}
		}


		/*
		 * More charm stuff.
		 */
		if ( victim->MasterId == ch->GetId() )
			stop_follower( victim );

		/*
		 * Inviso attacks ... not.
		 */
		if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
		{
			affect_strip( ch, gsn_invis );
			affect_strip( ch, gsn_mass_invis );
			REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
			act( AT_MAGIC, "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
		}

		/* Take away Hide */
		if ( IS_AFFECTED(ch, AFF_HIDE) )
			REMOVE_BIT(ch->affected_by, AFF_HIDE);
		/*
		 * Damage modifiers.
		 */
		if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
			dam /= 2;

		if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
			dam -= (int) (dam / 4);

		if ( dam < 0 )
			dam = 0;


		/*
		 * Check for disarm, trip, parry, and dodge.
		 */
		if ( dt >= TYPE_HIT)
		{
			if ( IS_NPC(ch)
					&&   IS_SET( ch->attacks, DFND_DISARM )
					&&   ch->level > 9
					&&   number_percent( ) < ch->level / 2 )
				disarm( ch, victim );

			if ( IS_NPC(ch)
					&&   IS_SET( ch->attacks, ATCK_TRIP )
					&&   ch->level > 5
					&&   number_percent( ) < ch->level / 2 )
				trip( ch, victim );

			if ( (dam_msg == DAMAGE_MSG_BITE) && (number_bits(2) == 0) )
			{
				/* Cheap fix to avoid blocking feed every time... */
			}
			else
			{
				OBJ_DATA * wield;
				bool isranged = FALSE;

				/* Ksilyan:
				   An extremely cheap fix for now to see if we're being attacked
				   with a ranged weapon. If the wielded is a ranged, then don't
				   parry.
				 */

				wield = get_eq_char(ch, WEAR_WIELD);
				if (!wield)
					isranged = FALSE;
				else
				{
					if (IS_RANGED_WEAPON_OBJ(wield))
						isranged = TRUE;
					else
						isranged = FALSE;
				}

				if (!isranged)
				{
					if ( check_parry( ch, victim ) )
					{
						parry_message(ch, victim, dt, dam_msg);
						return rAVOIDED;
					}
					if ( check_dodge( ch, victim ) )
					{
						dodge_message(ch, victim, dt, dam_msg);
						return rAVOIDED;
					}
				}
				else
				{
					/* We have a ranged weapon, so huge penalties. */
					if (number_bits(4) == 0)
					{
						if ( check_dodge(ch, victim) )
						{
							dodge_message(ch, victim, dt, dam_msg);
							return rAVOIDED;
						}
					}
				}
			}
		}

		/*
		 * Check control panel settings and modify damage
		 */
		if ( IS_NPC(ch) )
		{
			if ( npcvict )
				dampmod = sysdata.dam_mob_vs_mob;
			else
				dampmod = sysdata.dam_mob_vs_plr;
		}
		else
		{
			if ( npcvict )
				dampmod = sysdata.dam_plr_vs_mob;
			else
				dampmod = sysdata.dam_plr_vs_plr;
		}
		if ( dampmod > 0 )
			dam = ( dam * dampmod ) / 100;


		if ( !bNoShields )
		{
			bNoShields = TRUE;

			if ( IS_AFFECTED( victim, AFF_FIRESHIELD )
					&&  !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
			{
				retcode = spell_fireball( gsn_fireball, victim->level, victim, ch );
			}

			if ( retcode != rNONE || char_died(ch) || char_died(victim) )
			{
				bNoShields = FALSE;
				return retcode;
			}

			if ( IS_AFFECTED( victim, AFF_ICESHIELD )
					&&  !IS_AFFECTED( ch, AFF_ICESHIELD ) )
			{
				retcode = spell_chill_touch( gsn_chill_touch, victim->level, victim, ch );
			}

			if ( retcode != rNONE || char_died(ch) || char_died(victim) )
			{
				bNoShields = FALSE;
				return retcode;
			}

			if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD )
					&&  !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
			{
				retcode = spell_lightning_bolt( gsn_lightning_bolt, victim->level, victim, ch );
			}
			if ( retcode != rNONE || char_died(ch) || char_died(victim) )
			{
				bNoShields = FALSE;
				return retcode;
			}
			bNoShields = FALSE;
		}
	}

	/*
	 * Code to handle equipment getting damaged, and also support  -Thoric
	 * bonuses/penalties for having or not having equipment where hit
	 * redone slightly by Ksilyan
	 */
	if ( dt != TYPE_UNDEFINED && !in_arena(ch))
	{
		damobj = get_eq_char(victim, bodypart_to_wearloc(bodypart) );
		if ( damobj )
		{
			if ( dam > get_obj_resistance(damobj) )
			{
				set_cur_obj(damobj);
				damage_obj(damobj);
			}
			dam = (int) (dam * 0.95);  /* add a 5% bonus for having something to block the blow */
			if ( dam < 1 )
				dam = 1;
		}
		else
			dam = (int) (dam * 1.15);  /* add 15% penalty for bare skin! */
	}

	/*
	 * Ksilyan:
	 * Constitution bonus/penalty.
	 */
	con = ch->getCon();
	if ( con < 10 )
	{
		dam += 3 * (10 - con);
	}
	else if ( con > 15 && dam > 1 )
	{
		dam -= 3 * (con - 15);
		if ( dam < 1 )
			dam = 1;
	}

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */

	dam_message( ch, victim, dam, dt, dam_msg);
	oldhit = victim->hit;
	victim->hit -= dam;

	ArgumentNumber arg1 = ArgumentNumber(-dam);
	list<Argument*> arguments;
	arguments.push_back(&arg1);
	victim->skryptSendEvent("healthChanged", arguments);
	arguments.clear();

	/*
	 * Get experience based on % of damage done			-Thoric
	 */
	if ( dam && ch != victim && !IS_NPC(ch) && ch->IsFighting() && !IS_SET( victim->act, ACT_TRAIN ) )
	{
		short minLevel = MAX_LEVEL, maxLevel = 0;
		CHAR_DATA *gch;

		if (dam > oldhit + 4)		/* +4 because at 0 you ain't dead - Ksilyan */
			dam = oldhit + 4;

		for ( gch = ch->GetInRoom()->first_person; gch; gch = gch->next_in_room )
		{
			if ( is_same_group(gch, ch) )
			{
				if ( gch->IsFighting() )
				{
					maxLevel = max(gch->level, maxLevel);
					minLevel = min(gch->level, minLevel);
				}
			}
		}

		/*if ( ch->GetVictim() == victim )
		  xp_gain = (int) (ch->fighting->xp * dam) / victim->max_hit;
		  else*/

		// we divide by two because the other half is given at the kill.
		float xp = xp_compute(ch, victim, maxLevel-minLevel) / 2.0;
		xp_gain = (int) (   float( xp * dam ) / victim->max_hit   );

		// Multiply the XP gain by .9 ^ (group level difference)
		if ( xp_gain > 1 )
			xp_gain = (int) ( xp_gain * pow(.9, maxLevel - minLevel ) );

		if ( xp_gain < 1 )
			xp_gain = 1;

		/* Changed 'true' to 'false' so that experience gain is not immediate(only takes place when fight is won) */
		gain_exp( ch, xp_gain, false );
	}

	if ( !IS_NPC(victim)
			&&	 victim->level >= LEVEL_IMMORTAL
			&&	 victim->hit < 1
			&& !in_arena(victim))
		victim->hit = 1;

	/* Make sure newbies dont die */

	if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
		victim->hit = 1;

	if ( dam > 0 && dt > TYPE_HIT
			&& !IS_AFFECTED( victim, AFF_POISON )
			&&	is_wielding_poisoned( ch )
			&& !IS_SET( victim->immune, RIS_POISON )
			&& !saves_poison_death( ch->level, victim ) )
	{
		AFFECT_DATA af;

		af.type 	 = gsn_poison;
		af.duration  = 20;
		af.location  = APPLY_STR;
		af.modifier  = -2;
		af.bitvector = AFF_POISON;
		affect_join( victim, &af );
		ch->mental_state = URANGE( 20, ch->mental_state + 2, 100 );
	}

	/*
	 * Vampire self preservation 			-Thoric
	 */
	if ( IS_VAMPIRE(victim) )
	{
		if ( dam >= (victim->max_hit / 10) )	/* get hit hard, lose blood */
			gain_condition(victim, COND_BLOODTHIRST, -1 - (victim->level / 20));
		if ( victim->hit <= (victim->max_hit / 8)
				&& victim->pcdata->condition[COND_BLOODTHIRST]>5 )
		{
			gain_condition(victim, COND_BLOODTHIRST,
					-URANGE(3, victim->level / 10, 8) );
			victim->hit += URANGE( 4, (victim->max_hit / 30), 15);
			set_char_color(AT_BLOOD, victim);
			send_to_char("You howl with rage as the beast within stirs!\n\r", victim);
		}
	}

	if ( !npcvict
			&&	 get_trust(victim) >= LEVEL_IMMORTAL
			&&	 get_trust(ch)	   >= LEVEL_IMMORTAL
			&&	 victim->hit < 1 )
	{
		victim->hit = 1;
	}
	update_pos( victim );

	switch( victim->position )
	{
		case POS_MORTAL:
			act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
					victim, NULL, NULL, TO_ROOM );
			set_char_color(AT_DANGER, victim);
			send_to_char("You are mortally wounded, and will die soon if not aided.\r\n", victim);
			break;

		case POS_INCAP:
			act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
					victim, NULL, NULL, TO_ROOM );
			set_char_color(AT_DANGER, victim);
			send_to_char("You are incapacitated and will slowly die, if not aided.\r\n", victim);
			break;

		case POS_STUNNED:
			if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
			{
				act( AT_ACTION, "$n is stunned, but will probably recover.",
						victim, NULL, NULL, TO_ROOM );
				set_char_color(AT_HURT, victim);
				send_to_char("You are stunned, but will probably recover.\r\n", victim);
			}
			break;

		case POS_DEAD:
			if ( dt >= 0 && dt < top_sn )
			{
				SkillType *skill = skill_table[dt];

				if ( skill->die_char && skill->die_char[0] != '\0' )
					act( AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR );
				if ( skill->die_vict && skill->die_vict[0] != '\0' )
					act( AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT );
				if ( skill->die_room && skill->die_room[0] != '\0' )
					act( AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT );
			}
			// inanimates get a special message
			if ( npcInanimate )
				act( AT_DEAD, "$n is DESTROYED!", victim, 0, 0, TO_ROOM );
			else
				act( AT_DEAD, "$n is DEAD!", victim, 0, 0, TO_ROOM );

			set_char_color(AT_DEAD, victim);
			send_to_char("You have been KILLED!\r\n", victim);
			break;

		default:
			if ( dam > victim->max_hit / 4 )
			{
				act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
				if ( number_bits(3) == 0 )
					worsen_mental_state( ch, 1 );
			}
			if ( victim->hit < victim->max_hit / 4 )
			{
				act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
						victim, 0, 0, TO_CHAR );
				if ( number_bits(2) == 0 )
					worsen_mental_state( ch, 1 );
			}
			break;
	}

	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if ( !IS_AWAKE(victim)		/* lets make NPC's not slaughter PC's */
			&&	 !IS_AFFECTED( victim, AFF_PARALYSIS ) )
	{
		if ( victim->GetVictim()
				&&	 victim->GetVictim()->hunting
				&&	 victim->GetVictim()->hunting->who == victim )
			stop_hunting( victim->GetVictim() );

		if ( victim->GetVictim()
				&&	 victim->GetVictim()->hating
				&&	 victim->GetVictim()->hating->who == victim )
			stop_hating( victim->GetVictim() );


		// NPCs stop beating up on unconscious folks
		if (!npcvict && IS_NPC(ch))
			ch->StopAttacking();

		// In all cases, the victim stops attacking
		victim->StopAttacking();
	}

	/*
	 * Payoff for killing things.
	 */
	if ( victim->position == POS_DEAD )
	{
		// inanimate victims don't give XP
		if ( !npcInanimate )
			group_gain( ch, victim );

		/* new arenarank rating system */
		if(in_arena(victim) && !IS_NPC(ch) && !IS_NPC(victim))
		{
			int gain;

			gain= (
					(ch->pcdata->rating >= victim->pcdata->rating + 200)
					? 0 /* lopsided victimkill pays nothing */
					: (victim->pcdata->rating>>7) /* 1/128th of rating */
				  );

			victim->pcdata->rating-=gain;
			ch->pcdata->rating+=gain;
			update_arena_rank(ch);
			update_arena_rank(victim);
		}

		if ( !in_arena(victim) )
		{
			if ( !npcvict )
			{
				int half_level;

				sprintf( log_buf, "%s killed by %s at %s",
						victim->getShort().c_str(),
						ch->getShort().c_str(),
						victim->GetInRoom()->name_.c_str()  /* players should never see vnums! */
					   );
				to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 0 );

				sprintf( log_buf, "%s killed by %s at %s",
						victim->getShort().c_str(),
						ch->getShort().c_str(),
						vnum_to_dotted(victim->GetInRoom()->vnum) /* imms can't live without vnums! */
					   );
				log_string( log_buf );


				ch->accumulated_exp = 0;
#if 0
				/*
				 * Dying penalty:
				 * 1/2 way back to previous level.
				 */
				if ( victim->exp > ((exp_level(victim, victim->level+1) - exp_level(victim, victim->level))/2)+exp_level(victim, victim->level) ) {
					half_level = (exp_level(victim, victim->level+1)-exp_level(victim,victim->level))/2;
				} else if ( victim->exp > exp_level(victim, victim->level) ) {
					half_level = victim->exp - exp_level(victim, victim->level);
				} else {
					half_level = (exp_level(victim, victim->level)-exp_level(victim, victim->level-1))/2;
				}
#else
				half_level = (victim->exp * 2)/100; /* 2 % of exp */
				if(half_level >=1000000)
					half_level=1000000; 	 /* max 1 million */
#endif
				gain_exp(victim, -half_level, FALSE);
			}
			else if ( !IS_NPC(ch) )		/* keep track of mob vnum killed */
				add_kill( ch, victim );
		}

		if ( !IS_NPC( ch ) && IS_NPC(victim) && !in_arena(ch))
		{
			int level_ratio;
			level_ratio = URANGE( 1, ch->level / victim->level, 50);
			if ( ch->pcdata->clan )
				ch->pcdata->clan->mkills++;
			ch->pcdata->mkills++;
			ch->GetInRoom()->area->mkills++;
			if ( ch->pcdata->deity )
			{
				if ( victim->race == ch->pcdata->deity->npcrace )
					adjust_favor( ch, 3, level_ratio );
				else
					if ( victim->race == ch->pcdata->deity->npcfoe )
						adjust_favor( ch, 17, level_ratio );
					else
						adjust_favor( ch, 2, level_ratio );
			}
		}
		else if ( !IS_NPC(ch) && !IS_NPC(victim) && ch != victim && !in_arena(ch))
		{
			if ( ch->pcdata->clan )
				ch->pcdata->clan->pkills++;
			ch->pcdata->pkills++;
			ch->GetInRoom()->area->pkills++;
		}

		if ( !IS_NPC(victim) && !in_arena(ch) )
		{
			if ( IS_NPC(ch) )
			{
				if ( victim->pcdata->clan )
					victim->pcdata->clan->mdeaths++;
				victim->pcdata->mdeaths++;
				victim->GetInRoom()->area->mdeaths++;
			} else if ( ch != victim )
			{
				if ( victim->pcdata->clan )
					victim->pcdata->clan->pdeaths++;
				victim->pcdata->pdeaths++;
				victim->GetInRoom()->area->pdeaths++;
			}
		}

		if ( !IS_NPC(ch) && IS_NPC(victim) && !IS_SET(victim->act, ACT_NOGEMDROP) )
		{
			OBJ_DATA * Gem;
			Gem = RandomGemDrop(victim);
			if (Gem)
				obj_to_char(Gem, victim);
		}
		set_cur_char(victim);
		raw_kill( ch, victim );
		victim = NULL;

		if ( !IS_NPC(ch) && !in_arena(ch) && !victim_is_ch &&
				!npcInanimate )
		{
			// autogold and autoloot - only do this stuff if
			// victim is not an inanimate NPC

			/* Autogold by Scryn 8/12 */
			if ( IS_SET(ch->act, PLR_AUTOGOLD) )
			{
				int gold_obtained;
				if ( npcvict )
				{
					OBJ_DATA * gold = NULL;
					OBJ_DATA * tempobj;
					OBJ_DATA * corpse;

					corpse = get_obj_here(ch, "corpse");
					if (corpse)
					{
						for (tempobj = corpse->first_content; tempobj; tempobj = tempobj->next)
						{
							if ( tempobj->pIndexData->vnum == OBJ_VNUM_MONEY_SOME )
							{
								gold = tempobj;
								break;
							}
						}
						if (gold)
						{
							if ( ch->getName() == "Tobi" )
								ch_printf(ch, "Old gold: %d    ", gold->value[0]);

							gold->value[0] = number_range((int) (0.8 * (1.0*gold->value[0])), (int) (1.2 * (1.0*gold->value[0])));

							if ( ch->getName() == "Tobi" )
								ch_printf(ch, "New gold: %d\n\r", gold->value[0]);

							{
								ostringstream os;
								os << gold->value[0] << " gold coins";
								gold->shortDesc_ = os.str();
							}
							{
								ostringstream os;
								os << "A pile of " << gold->value[0] << " gold coins.";
								gold->longDesc_ = os.str();
							}
						}
						else
						{
							if ( ch->getName() == "Tobi" )
							{
								send_to_char("No coins!\n\r", ch);
							}
						}
					}
				}

				init_gold = ch->gold;
				do_get( ch, "coins corpse" );
				new_gold = ch->gold;
				gold_obtained = new_gold - init_gold;

				if ( ch->getName() == "Tobi" )
					ch_printf(ch, "You got %d coins to split.\n\r", gold_obtained);

				if (gold_obtained > 0)
				{
					sprintf(buf,"%d",gold_obtained);
					do_split( ch, buf );
				}
			}
			if ( IS_SET(ch->act, PLR_AUTOLOOT) )
				do_get( ch, "all corpse" );
			else
				do_look( ch, "in corpse" );

			if ( IS_SET(ch->act, PLR_AUTOSAC) )
				do_sacrifice( ch, "corpse" );
		}

		if ( IS_SET( sysdata.save_flags, SV_KILL ) )
			save_char_obj( ch );
		return rVICT_DIED;
	}

	if ( victim == ch )
		return rNONE;

	/*
	 * Take care of link dead people.
	 */
	if ( !npcvict && !victim->GetConnection()
			&& !IS_SET( victim->pcdata->flags, PCFLAG_NORECALL )
			&& !in_arena(victim))
	{
		if ( number_range( 0, victim->GetWait() ) == 0)
		{
			do_recall( victim, "" );
			return rNONE;
		}
	}

	/*
	 * Wimp out?
	 */
	if ( npcvict && dam > 0 )
	{
		if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
					&&	 victim->hit < victim->max_hit / 2 )
				||	 ( IS_AFFECTED(victim, AFF_CHARM) && victim->GetMaster() != NULL
					&&	   victim->GetMaster()->GetInRoom() != victim->GetInRoom() ) )
		{
			start_fearing( victim, ch );
			stop_hunting( victim );
			do_flee( victim, "" );
		}
	}

	if ( !npcvict
			&&	 victim->hit > 0
			&&	 victim->hit <= victim->wimpy
			&&	 victim->GetWait() == 0 )
	{
		do_flee( victim, "" );
	}
	else if ( !npcvict && IS_SET( victim->act, PLR_FLEE ) )
	{
		// Ksilyan: only allow people to +flee from NPCs
		if ( IS_NPC(ch) )
			do_flee( victim, "" );
	}

	tail_chain( );
	return rNONE;
}


bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
	/* Thx Josh! */
	if ( ch->GetVictim() == ch )
		return FALSE;

	if ( !victim )
		return TRUE;
	if ( !victim->GetInRoom() )
		return TRUE;

	// Disabled for now.
#if 0
	if ( !IS_NPC(ch) ) {
		if ( victim && ((IS_SET(victim->act, PLR_AFK) || (!IS_NPC(victim) && !victim->Connection))) ) {
			send_to_char( "That wouldn't be nice at all!\r\n", ch );
			return TRUE;
		}
	}
#endif


	if ( IS_SET( victim->GetInRoom()->room_flags, ROOM_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "A magical force prevents you from attacking.\n\r", ch );
		return TRUE;
	}

	if ( IS_NPC(victim) && IS_SET( victim->act, ACT_SAFE ) )
	{
		set_char_color( AT_MAGIC, ch );
		act( AT_MAGIC, "A magical force prevents you from attacking $N.", ch, NULL, victim, TO_CHAR );
		return TRUE;
	}

	if ( ch->level > LEVEL_IMMORTAL )
		return FALSE;

	return FALSE;}

/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
	if ( !victim )
	{
		bug( "update_pos: null victim", 0 );
		return;
	}

	if ( victim->position != POS_SITTING && victim->position != POS_RESTING && victim->SittingOnId != 0 ) {
		victim->SittingOnId = 0;
	}

	if ( victim->hit > 0 )
	{
		if ( victim->position <= POS_STUNNED )
			victim->position = POS_STANDING;
		if ( IS_AFFECTED( victim, AFF_PARALYSIS ) )
			victim->position = POS_STUNNED;
		return;
	}

	if ( IS_NPC(victim) || victim->hit < -3 )
	{
		if ( victim->GetMount() )
		{
			act( AT_ACTION, "$n falls from $N.",
					victim, NULL, victim->GetMount(), TO_ROOM );
			REMOVE_BIT( victim->GetMount()->act, ACT_MOUNTED );
			victim->MountId = 0;
		}
		victim->position = POS_DEAD;
		return;
	}

	if ( victim->hit == -1 ) victim->position = POS_STUNNED;
	else if ( victim->hit == -2 ) victim->position = POS_INCAP;
	else if ( victim->hit == -3 ) victim->position = POS_MORTAL;

	if ( victim->position > POS_STUNNED
			&&   IS_AFFECTED( victim, AFF_PARALYSIS ) )
		victim->position = POS_STUNNED;

	if ( victim->GetMount() )
	{
		act( AT_ACTION, "$n falls unconscious from $N.",
				victim, NULL, victim->GetMount(), TO_ROOM );
		REMOVE_BIT( victim->GetMount()->act, ACT_MOUNTED );
		victim->MountId = 0;
	}
	return;
}




#if 0
/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	FIGHT_DATA *fight;

	if ( ch->fighting )
	{
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Set_fighting: %s -> %s (already fighting %s)",
			ch->name, victim->name, ch->fighting->who->name );
		bug( buf, 0 );
		return;
	}


	if ( in_arena(ch) && !(IS_NPC(ch) && IS_NPC(victim)) && (!victim->fighting || victim->fighting->who != ch)) {
		char buf[MAX_STRING_LENGTH];

		sprintf(buf, "%s snarls in rage as %s throws %sself against %s.",
			NAME(victim),
			SEX_HE(victim),
			SEX_HIM(victim),
			NAME(ch));
		talk_channel(ch, buf, CHANNEL_ARENA, "");
	}

	/* Testaur - log (non-arena) fights against players */
	/* note: victim is actually the aggressor,	ch is the one attacked! */

	if(!in_arena(ch) && !IS_NPC(ch) && (!victim->fighting || victim->fighting->who != ch))
	{/* some player has been attacked for real... */
		char buf[MAX_STRING_LENGTH];
		char *ptr;
		CHAR_DATA *whodunit;

		/* whodunit? */
		whodunit=NULL;
		if(IS_NPC(victim))
		{/* see if this NPC is acting on some player's orders */
			if(IS_AFFECTED( victim , AFF_CHARM ) && victim->MasterId != 0)
			{
				if (!IS_NPC( CharacterMap[victim->MasterId] ))
				{
					whodunit=CharacterMap[victim->MasterId];
					sprintf( buf, "PK: %s's charmed %s", NAME(whodunit), NAME(victim));
				}
			}
		}
		else
		{/* a player did it. */
			whodunit=victim;
			sprintf( buf, "PK: %s", NAME(whodunit));
		}

		if( whodunit )	/* a player was the instigator */
		{
			for(ptr=buf; *ptr ; ptr++)
				;
			/* now determine legality of attack */
			if( whodunit->level - ch->level > 5 )
			{/* illegal attack */
				sprintf(ptr, " unfairly");
			}
			else
			{
				if( ch->level - whodunit->level > 5 )
				{/* suicidal attack */
					sprintf(ptr, " foolishly");
				}
			}
			while(*ptr)
				ptr++;

			sprintf( ptr, " attacks %s", NAME(ch) );
			log_string(buf);
			to_channel(buf,CHANNEL_MONITOR,"Monitor",0);
		}
	} /* end of log section */

	if ( IS_AFFECTED(ch, AFF_SLEEP) )
		affect_strip( ch, gsn_sleep );

	/* Limit attackers -Thoric */
	if ( victim->num_fighting > max_fight(victim) )
	{
		send_to_char( "There are too many people fighting for you to join in.\n\r", ch );
		return;
	}

	CREATE( fight, FIGHT_DATA, 1 );
	fight->who	 = victim;
	fight->xp	 = (int) xp_compute( ch, victim );
	fight->align = align_compute( ch, victim );
	if ( !IS_NPC(ch) && IS_NPC(victim) )
		fight->timeskilled = times_killed(ch, victim);
	ch->num_fighting = 1;
	ch->fighting = fight;
	ch->position = POS_FIGHTING;
	victim->num_fighting++;
	if ( victim->SwitchedCharId != 0 && IS_AFFECTED(CharacterMap[victim->SwitchedCharId], AFF_POSSESS) )
	{
		send_to_char( "You are disturbed!\n\r", CharacterMap[victim->SwitchedCharId] );
		do_return( CharacterMap[victim->SwitchedCharId], "" );
	}
	return;
}
#endif


#if 0
CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
	if ( !ch )
	{
		bug( "who_fighting: null ch", 0 );
		return NULL;
	}
	if ( !ch->fighting )
		return NULL;
	return ch->fighting->who;
}

void free_fight( CHAR_DATA *ch )
{
	if ( !ch )
	{
		bug( "Free_fight: null ch!", 0 );
		return;
	}
	if ( ch->fighting )
	{
		if ( !char_died(ch->fighting->who) )
			--ch->fighting->who->num_fighting;
		DISPOSE( ch->fighting );
	}

	ch->fighting = NULL;
	if ( ch->GetMount() )
		ch->position = POS_MOUNTED;
	else
		ch->position = POS_STANDING;

	/* Berserk wears off after combat. -- Altrag */
	if ( IS_AFFECTED(ch, AFF_BERSERK) )
	{
		affect_strip(ch, gsn_berserk);
		set_char_color(AT_WEAROFF, ch);
		send_to_char(skill_table[gsn_berserk]->msg_off, ch);
		send_to_char("\n\r", ch);
	}
	return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    free_fight( ch );
    update_pos( ch );

    if ( !fBoth )   /* major short cut here by Thoric */
      return;

	for ( fch = first_char; fch; fch = fch->next )
	{
		if ( who_fighting( fch ) == ch )
		{
			free_fight( fch );
			update_pos( fch );
		}
	}
    return;
}
#endif


/*
 * Improved Death_cry contributed by Diavolo.
 * Additional improvement by Thoric (and removal of turds... sheesh!)
 */
void death_cry( CHAR_DATA *ch )
{
	ROOM_INDEX_DATA *was_in_room;
	const char *msg;
	ExitData *pexit;
	int vnum;

	if ( !ch )
	{
		bug( "DEATH_CRY: null ch!", 0 );
		return;
	}

	vnum = 0;
	switch ( number_bits( 4 ) )
	{
	default: msg  = "You hear $n's death cry."; 			break;
	case  0: msg  = "$n hits the ground ... DEAD."; 		break;
	case  1: msg  = "$n splatters blood on your armor.";		break;
#if 0
	case  2: if ( HAS_BODYPART(ch, PART_GUTS) )
			 {
				 msg  = "$n's guts spill all over the ground.";
				 vnum = OBJ_VNUM_SPILLED_GUTS;
			 }
		else
			msg = "$n collapses lifeless to the ground.";
		break;
		case  3: /*if ( HAS_BODYPART(ch, PART_HEAD) )
				 {
				 msg  = "$n's severed head plops on the ground.";
				 vnum = OBJ_VNUM_SEVERED_HEAD;
				 }
			else*/
			msg = "You hear $n's death cry.";
			break;
		case  4: if ( HAS_BODYPART(ch, PART_HEART) )
				 {
					 msg  = "$n's heart is torn from $s chest.";
					 vnum = OBJ_VNUM_TORN_HEART;
				 }
			else
				msg = "$n collapses lifeless to the ground.";
			break;
		case  5: if ( HAS_BODYPART(ch, PART_ARMS) )
				 {
					 msg  = "$n's arm is sliced from $s dead body.";
					 vnum = OBJ_VNUM_SLICED_ARM;
				 }
			else
				msg = "You hear $n's death cry.";
			break;
		case  6: if ( HAS_BODYPART( ch, PART_LEGS) )
				 {
					 msg  = "$n's leg is sliced from $s dead body.";
					 vnum = OBJ_VNUM_SLICED_LEG;
				 }
			else
				msg = "$n collapses lifeless to the ground.";
			break;
#endif
	}

	act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );

	if ( vnum )
	{
		char buf[MAX_STRING_LENGTH];
		OBJ_DATA *obj;
		const char * name;

		name = ch->getShort().c_str();
		obj 	= create_object( get_obj_index( vnum ), 0 );
		obj->timer	= number_range( 4, 7 );

		sprintf( buf, obj->shortDesc_.c_str(), name );
		obj->shortDesc_ = buf;

		sprintf( buf, obj->longDesc_.c_str(), name );
		obj->longDesc_ = buf;

		obj = obj_to_room( obj, ch->GetInRoom() );
	}

	if ( IS_NPC(ch) )
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	was_in_room = ch->GetInRoom();
	for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room
			&&	 pexit->to_room != was_in_room )
		{
			ch->InRoomId = pexit->to_room->GetId();
			act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->InRoomId = was_in_room->GetId();

	return;
}



void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *victmp;
	char buf[MAX_STRING_LENGTH];

	if ( !victim )
	{
		bug( "raw_kill: null victim!", 0 );
		return;
	}
	/* backup in case hp goes below 1 */
	if (NOT_AUTHED(victim))
	{
		bug( "raw_kill: killing unauthed", 0 );
		return;
	}

	// so that programs know the victim is dead    -Ksilyan
	if ( victim->hit > 0 )
		victim->hit = -10;

	victim->StopAllFights();

	/* Take care of polymorphed chars */
	if(IS_NPC(victim) && IS_SET(victim->act, ACT_POLYMORPHED))
	{
		Character * ori = CharacterMap[victim->GetConnection()->OriginalCharId];
		if ( !ori )
		{
			gTheWorld->LogBugString("raw_kill: polymorphed character had invalid OriginalId! Char: " +
				victim->codeGetBasicInfo() );
			return;
		}
		char_from_room( ori );
		char_to_room( ori, victim->GetInRoom() );
		victmp = ori;
		do_revert(victim, "");
		raw_kill(ch, victmp);
		return;
	}

	mprog_death_trigger( ch, victim );
	if ( char_died(victim) )
		return;

	/* death_cry( victim ); - the mprog death trigger handles this */

	rprog_death_trigger( ch, victim );
	if ( char_died(victim) )
		return;

	mprog_otherdeath_trigger( victim );
	if ( char_died(victim) )
		return;

	if ( in_arena(ch) && !(IS_NPC(ch) && IS_NPC(victim))) {
		sprintf(buf, "%s is devoured by hellfire as %s is slain by the mighty %s.",
			NAME(victim),
			SEX_HE(victim),
			NAME(ch));
		talk_channel(ch, buf, CHANNEL_ARENA, "");
	} else {
		if ( !IS_NPC(victim) || (IS_NPC(victim) && !IS_SET(victim->act, ACT_INANIMATE)) )
		{
			// Inanimate mobs do not create corpses or blood
			make_corpse( victim, ch );
			make_blood( victim );
		}
	}

	if ( IS_NPC(victim) )
	{
		victim->pIndexData->killed++;
		extract_char( victim, TRUE );
		victim = NULL;
		return;
	}

	set_char_color( AT_DIEMSG, victim );
	do_help(victim, "_DIEMSG_" );


	if ( in_arena(victim) ) {
		arena_clear_char(victim);
	} else {
		while ( victim->first_affect )
			affect_remove( victim, victim->first_affect );
		fix_affected_by ( victim );
		victim->resistant	= 0;
		victim->susceptible = 0;
		victim->immune		= 0;
		victim->carry_weight= 0;
		victim->armor	= 100;
		victim->mod_str = 0;
		victim->mod_dex = 0;
		victim->mod_wis = 0;
		victim->mod_int = 0;
		victim->mod_con = 0;
		victim->mod_cha = 0;
		victim->mod_lck 	= 0;
		victim->damroll = 0;
		victim->hitroll = 0;
		victim->mental_state = -10;
		victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
		victim->saving_spell_staff = 0;
		victim->position	= POS_RESTING;
		victim->hit 	= UMAX( 1, victim->hit	);
		victim->mana	= UMAX( 1, victim->mana );
		victim->move	= UMAX( 1, victim->move );

		victim->pcdata->condition[COND_FULL]   = 12;
		victim->pcdata->condition[COND_THIRST] = 12;
		if ( IS_VAMPIRE( victim ) )
			victim->pcdata->condition[COND_BLOODTHIRST] = (victim->level / 2);
	}

	extract_char( victim, FALSE );
	if ( !victim )
	{
		bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
		return;
	}

	if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
		save_char_obj( victim );
	return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    short minLevel = MAX_LEVEL;
    short maxLevel = 0;
    //char buf[MAX_INPUT_LENGTH];

    if ( in_arena(ch) || in_arena(victim) ) {
        return;
    }

    /*
     * Monsters don't get kill xp's or alignment changes.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( IS_NPC(ch) || victim == ch )
	return;

    //Mobs flagged as not giving experience will not give experience
    if( IS_SET( victim->act, ACT_TRAIN ) )
       return;

    members = 0;

    lch = ch->LeaderId != 0 ? CharacterMap[ch->LeaderId] : ch;

    for ( gch = ch->GetInRoom()->first_person; gch; gch = gch->next_in_room )
	{
		if ( !is_same_group( gch, ch ) )
			continue;

		members++;

		if ( gch->IsFighting() ) {
			maxLevel = max(gch->level, maxLevel);
			minLevel = min(gch->level, minLevel);
		}
	}

    if ( members == 0 )
    {
		bug( "Group_gain: 0 members.", members );
		members = 1;
    }

    for ( gch = ch->GetInRoom()->first_person; gch; gch = gch->next_in_room )
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		if ( !is_same_group(gch, ch) )
			continue;

		float xpBonus = ( xp_compute( gch, victim ) );
		if( members > 1 )
		{
			/*xpBonus *= ( 10 - ( 0.1 * members ) );*/
			/*^^old exp bonus for groups way too much--Rassik*/
			xpBonus = xpBonus + ( xpBonus * (0.1 * members ) );
			/*new exp bonus, 10 extra exp per member in group*/
			xpBonus *= pow( 0.9, pow( 1.15, maxLevel - minLevel ) );
		}

		xp = int(xpBonus);

		if ( xp < 1 )
			xp = 1;

		if ( !gch->IsFighting() )
			xp = 0;

		gch->alignment = align_compute( gch, victim );

		gch->accumulated_exp += xp;
		gch->GainAccumulatedXp();

		for ( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if ( obj->wear_loc == WEAR_NONE )
				continue;

			if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
			{
				act( AT_MAGIC, "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
				act( AT_MAGIC, "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );

				obj_from_char( obj );
				obj = obj_to_room( obj, ch->GetInRoom() );

				if ( !obj ) { return; }

				oprog_zap_trigger(ch, obj);  /* mudprogs */
				if ( char_died(ch) )
					return;
			}
		}
	}

    return;
}


int align_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align, newalign;

    align = gch->alignment - victim->alignment;

    if ( align >  500 )
	newalign  = UMIN( gch->alignment + (align-500)/4,  1000 );
    else
    if ( align < -500 )
	newalign  = UMAX( gch->alignment + (align+500)/4, -1000 );
    else
	newalign  = gch->alignment - (int) (gch->alignment / 4);

    return newalign;
}


/*
 * Calculate how much XP gch should gain for killing victim - Thoric
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int levPenalty )
{
	int xp;
	int gchlev = gch->level + levPenalty;

	xp = (get_exp_worth( victim ) * URANGE( 0, (victim->level - gchlev) + 10, 13 )) / 10;


    /* get 1/4 exp for players					-Thoric */
    if ( !IS_NPC( victim ) )
    	xp /= 4;
    else
	/* Updated this table to be a bit more steamlined -Zoie */
	/* Reduced exp for repeatedly killing the same mob - Thoric */
	if ( !IS_NPC( gch ) )
	{
		int times = times_killed( gch, victim );

		if ( times >= 25 )
			xp /= 10;
		else if ( times > 20 )
			xp /= 5;
		else if ( times > 15 )
			xp /= 3;
		else if ( times > 10 )
			xp /= 2;
	}

	/*
	 * Level based experience gain cap.  Cannot get more experience for
	 * a kill than the amount for your current experience level   -Thoric
	 */
	return URANGE(0, xp, exp_level( gch, gchlev ));
}

/* KSILYAN
 * Functions to give more interesting parry and dodge messages.
 */

void dodge_message( CHAR_DATA * ch, CHAR_DATA * victim, int dt, int dam_msg)
{
	char act_vict[MAX_INPUT_LENGTH];
	char act_char[MAX_INPUT_LENGTH];

	if ( (dt >= TYPE_HIT + MIN_DAMAGE_TYPE) && (dt <= TYPE_HIT + MAX_DAMAGE_TYPE) )
	{
		switch(dam_msg)
		{
			default:
				strcpy(act_vict, "You dodge $n's attack.");
		        strcpy(act_char, "$N dodges your attack.");
				break;
			case DAMAGE_MSG_SWING:
				strcpy(act_vict, "You duck under $n's swing.");
				strcpy(act_char, "Your swing almost connects with $N but is dodged.");
				break;
			case DAMAGE_MSG_WHIP:
				strcpy(act_vict, "You narrowly dodge the crack of $n's whip.");
				strcpy(act_char, "$N narrowly avoids the crack of your whip.");
				break;
			case DAMAGE_MSG_STAB:
				strcpy(act_vict, "You deftly sidestep $n's lunge.");
				strcpy(act_char, "$N deftly sidesteps your lunge.");
				break;
			case DAMAGE_MSG_CLAW:
				strcpy(act_vict, "You jump back and avoid $n's sweeping claws.");
				strcpy(act_char, "$N jumps back from your swipe.");
				break;
			case DAMAGE_MSG_BITE:
				strcpy(act_vict, "You dodge just in time to avoid $n's bite.");
				strcpy(act_char, "$N avoids your bite just in time.");
				break;
			case DAMAGE_MSG_POUND:
				strcpy(act_vict, "You twist your body and avoid $n's pound.");
				strcpy(act_char, "Your pound meets with empty air where $N used to be.");
				break;
			case DAMAGE_MSG_ARROW:
				strcpy(act_vict, "$n's arrow whizzes past as you swerve out of the way.");
				strcpy(act_char, "$N narrowly avoids your arrow, which flies past $M.");
				break;
		}
	}
	else
	{	/* for other, generic attacks that we don't feel like handling */
		strcpy(act_vict, "You dodge $n's attack.");
		strcpy(act_char, "$N dodges your attack.");
	}


	if ( !IS_NPC(victim) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG) )
		act( AT_SKILL, act_vict, ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG) )
	    act( AT_SKILL, act_char, ch, NULL, victim, TO_CHAR    );
}

void parry_message( CHAR_DATA * ch, CHAR_DATA * victim, int dt, int dam_msg)
{
	char act_vict[MAX_INPUT_LENGTH];
    char act_char[MAX_INPUT_LENGTH];

	if ( (dt >= TYPE_HIT + MIN_DAMAGE_TYPE) && (dt <= TYPE_HIT + MAX_DAMAGE_TYPE) )
	{
		switch(dam_msg)
		{
			default:
				strcpy(act_vict, "You parry $n's attack.");
				strcpy(act_char, "$N parries your attack.");
				break;
			case DAMAGE_MSG_SWING:
				strcpy(act_vict, "You deflect $n's swing.");
				strcpy(act_char, "$N deflects your swing.");
				break;
			case DAMAGE_MSG_WHIP:
				strcpy(act_vict, "You block $n's whip with your weapon.");
				strcpy(act_char, "$N blocks your whip with $S weapon.");
				break;
			case DAMAGE_MSG_STAB:
				strcpy(act_vict, "You fend off $n's lunge.");
				strcpy(act_char, "$N fends off your lunge.");
				break;
			case DAMAGE_MSG_CLAW:
				strcpy(act_vict, "You intercept $n's swiping claws.");
				strcpy(act_char, "$N intercepts your claws.");
				break;
			case DAMAGE_MSG_BITE:
				strcpy(act_vict, "You stop $n's bite with your weapon.");
				strcpy(act_char, "$N sticks $S weapon in your face, preventing you from biting $M.");
				break;
			case DAMAGE_MSG_POUND:
				strcpy(act_vict, "You parry $n's crushing blow.");
				strcpy(act_char, "$N parries your crushing blow.");
				break;
		}
	}
	else
	{	/* for other, generic attacks that we don't feel like handling */
		strcpy(act_vict, "You parry $n's attack.");
		strcpy(act_char, "$N parries your attack.");
	}


	if ( !IS_NPC(victim) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG) )
		act( AT_SKILL, act_vict, ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG) )
		act( AT_SKILL, act_char, ch, NULL, victim, TO_CHAR    );
}


/*
 * Revamped by Thoric to be more realistic
 */
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_msg)
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    sh_int dampc;
    SkillType *skill = NULL;
    bool gcflag = FALSE;
    bool gvflag = FALSE;

	if (dam_msg == 0 && victim == ch )
	{
		/* KSILYAN
		 * I'm not sure how exactly this was dealt with before,
		 * but it seems that by adding in the dam_msg when it's = 0
		 * confuses the game and makes all "misc" damage messages
		 * (such as when you get hurt from hunger, sunlight, thirst...)
		 * come out as the player whacking himself.
		 * This is obviously no good, so we're going to just leave for
		 * now, and check later to see if there is some kind of more
		 * elegant way we could handle this.
		 */
		return;
	}

    if ( ! dam )
		dampc = 0;
	else
		dampc = ( (dam * 1000) / victim->max_hit) + ( 50 - ((victim->hit * 50) / victim->max_hit) );

	/*		     10 * percent					*/
	if 		( dam ==      0 ) { vs = "miss";			vp = "misses";		}
    else if ( dampc <=    5 ) { vs = "barely scratch";	vp = "barely scratches";}
    else if ( dampc <=   10 ) { vs = "scratch";			vp = "scratches";	}
    else if ( dampc <=   20 ) { vs = "nick";			vp = "nicks";		}
    else if ( dampc <=   30 ) { vs = "graze";			vp = "grazes";		}
    else if ( dampc <=   40 ) { vs = "bruise";			vp = "bruises";		}
    else if ( dampc <=   50 ) { vs = "hit";				vp = "hits";		}
    else if ( dampc <=   60 ) { vs = "injure";			vp = "injures";		}
    else if ( dampc <=   75 ) { vs = "thrash";			vp = "thrashes";	}
    else if ( dampc <=   80 ) { vs = "wound";			vp = "wounds";		}
    else if ( dampc <=   90 ) { vs = "maul";			vp = "mauls";		}
    else if ( dampc <=  125 ) { vs = "decimate";		vp = "decimates";	}
    else if ( dampc <=  150 ) { vs = "devastate";		vp = "devastates";	}
    else if ( dampc <=  200 ) { vs = "maim";			vp = "maims";		}
    else if ( dampc <=  300 ) { vs = "MUTILATE";		vp = "MUTILATES";	}
    else if ( dampc <=  400 ) { vs = "DISEMBOWEL";		vp = "DISEMBOWELS";	}
    else if ( dampc <=  500 ) { vs = "MASSACRE";		vp = "MASSACRES";	}
    else if ( dampc <=  600 ) { vs = "PULVERIZE";		vp = "PULVERIZES";	}
    else if ( dampc <=  750 ) { vs = "EVISCERATE";		vp = "EVISCERATES";	}
    else if ( dampc <=  990 ) { vs = "* OBLITERATE *";	vp = "* OBLITERATES *";			}
    else                      { vs = "*** ANNIHILATE ***"; vp = "*** ANNIHILATES ***";		}

    punct   = (dampc <= 30) ? '.' : '!';

    if ( dam == 0 && (!IS_NPC(ch) &&
		(IS_SET(ch->pcdata->flags, PCFLAG_GAG)))) gcflag = TRUE;

    if ( dam == 0 && (!IS_NPC(victim) &&
		(IS_SET(victim->pcdata->flags, PCFLAG_GAG)))) gvflag = TRUE;

	if ( dt >=0 && dt < top_sn )
		skill = skill_table[dt];

	if ( (dt == TYPE_HIT)
			|| ( (dt >= TYPE_HIT + MIN_DAMAGE_TYPE) && (dt <= TYPE_HIT + MAX_DAMAGE_TYPE) )
			)
	{
		if (!(dam_msg >= MIN_DAMAGE_MSG && dam_msg <= MAX_DAMAGE_MSG))
		{
			bug( "Dam_message: bad dam_msg %d %s to %s.", dam_msg, NAME(ch),NAME(victim) );
			dam_msg = DAMAGE_MSG_DEFAULT;
		}
		attack = damagemsg_number_to_name(dam_msg, dam);

		if ( is_wielding_poisoned( ch ) )
		{
			sprintf( buf1, "$n's poisoned %s %s $N%c", attack, vp, punct );
			sprintf( buf2, "Your poisoned %s %s $N%c", attack, vp, punct );
			sprintf( buf3, "$n's poisoned %s %s you%c", attack, vp, punct );
	    }
		else
		{
			sprintf( buf1, "$n's %s %s $N%c", attack, vp, punct );
			sprintf( buf2, "Your %s %s $N%c", attack, vp, punct );
			sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
		}
	}
    else
    {
		if ( skill )
		{
			attack	= skill->nounDamage_.c_str();
			if ( dam == 0 )
			{
				bool found = FALSE;

				if ( skill->miss_char && skill->miss_char[0] != '\0' )
				{
					act( AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR );
					found = TRUE;
				}
				if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
				{
					act( AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT );
					found = TRUE;
				}
				if ( skill->miss_room && skill->miss_room[0] != '\0' )
				{
					act( AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOTVICT );
					found = TRUE;
				}
				if ( found )	/* miss message already sent */
					return;
			}
			else
			{
				if ( skill->hit_char && skill->hit_char[0] != '\0' )
					act( AT_HIT, skill->hit_char, ch, NULL, victim, TO_CHAR );
				if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
					act( AT_HITME, skill->hit_vict, ch, NULL, victim, TO_VICT );
				if ( skill->hit_room && skill->hit_room[0] != '\0' )
					act( AT_ACTION, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
			}
		}
		else
		{
			/* KSILYAN
				If we're not hitting, and we're not using a skill,
				then we must be doing unknown damage type... right?
				So we don't want to display any message?
			*/
			return;
		}

		sprintf( buf1, "$n's %s %s $N%c", attack, vp, punct );
		sprintf( buf2, "Your %s %s $N%c", attack, vp, punct );
		sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
	}


	act( AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT );
	if (!gcflag) act( AT_HIT, buf2, ch, NULL, victim, TO_CHAR );
	if (!gvflag) act( AT_HITME, buf3, ch, NULL, victim, TO_VICT );

	return;
}


void do_kill(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Kill whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( is_safe( ch, victim ) )
		return;

	if ( victim == ch )
	{
		send_to_char( "You hit yourself.  Ouch!\n\r", ch );
		multi_hit( ch, ch, TYPE_UNDEFINED );
		return;
	}

	if ( !IS_NPC(victim) ) {
		if ( IS_SET(victim->act, PLR_AFK) || !victim->GetConnection() ) {
			send_to_char("That wouldn't be nice at all!\r\n", ch);
			return;
		}
	}

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->MasterId == victim->GetId() )
	{
		act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
		return;
	}

	/*
	 * Removal by Ksilyan to allow the changing of targets.
	if ( ch->position == POS_FIGHTING )
	{
	send_to_char( "You do the best you can!\n\r", ch );
	return;
	}
	 */

	// More fixes by Ksilyan: if we just multi_hit then people
	// can attack as many times as they type kill.
	// So, we have to first check if ch is fighting... if so,
	// check the target... if target != victim, STOP fighting,
	// and then multihit the new target.

	if ( ch->IsAttacking() )
	{
		if ( ch->GetVictim() == victim )
		{
			ch->sendText("You do the best you can!\n\r");
			return;
		}
		else
		{
			// stop fighting and change target
			ch->StopAttacking();
			ch->StartAttacking(victim);
			ch->sendText("You change targets!\n\r");
		}
	}
	else
	{
		ch->StartAttacking(victim);
		ch->AddWait(1 * PULSE_VIOLENCE);
		multi_hit( ch, victim, TYPE_UNDEFINED );
	}

	return;
}



void do_murde(CHAR_DATA *ch, const char* argument)
{
   do_kill(ch, argument);
}



void do_murder(CHAR_DATA *ch, const char* argument)
{
   do_kill(ch, argument);
}


void do_flee(CHAR_DATA *ch, const char* argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	char buf[MAX_STRING_LENGTH];
	int attempt, los;
	sh_int door;
	ExitData *pexit;

	if ( !ch->IsFighting() )
	{
		if ( ch->position == POS_FIGHTING )
		{
			if ( ch->GetMount() )
				ch->position = POS_MOUNTED;
			else
				ch->position = POS_STANDING;
		}
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		return;
	}

	if ( ch->move <= 0 )
	{
		send_to_char( "You're too exhausted to flee from combat!\n\r", ch );
		return;
	}

	/* No fleeing while stunned. - Narn */
	if ( ch->position < POS_FIGHTING )
		return;

	was_in = ch->GetInRoom();
	for ( attempt = 0; attempt < 8; attempt++ )
	{

		door = number_door( );
		if ( ( pexit = get_exit(was_in, door) ) == NULL
				||   !pexit->to_room
				|| ( IS_SET(pexit->exit_info, EX_CLOSED)
					&&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
				|| ( IS_NPC(ch)
					&&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
			continue;

		affect_strip ( ch, gsn_sneak );
		REMOVE_BIT   ( ch->affected_by, AFF_SNEAK );
		if ( ch->GetMount() && ch->GetMount()->IsFighting() )
			ch->GetMount()->StopAllFights();//stop_fighting( ch->GetMount(), TRUE );
		move_char( ch, pexit, 0 );

		if ( ( now_in = ch->GetInRoom() ) == was_in )
			continue;
		if ( now_in == NULL )
			break;

		ch->InRoomId = was_in->GetId();
		act( AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );
		ch->InRoomId = now_in->GetId();
		act( AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );

		if ( !IS_NPC(ch) && !in_arena(ch) )
		{
			CHAR_DATA *wf = ch->GetVictim();
			int level_ratio;
#if 0
			los = (exp_level(ch, ch->level+1)
					- exp_level(ch, ch->level)) * 0.03;
			los += ch->accumulated_exp;
#else
			los = (ch->exp * 2)/1000;  /* 0.2% = 1/10th of dying penalty */
			if(los>=100000)
				los=100000;
#endif
			sprintf(buf, "You flee from combat!  You lose %d experience.", los );
			act( AT_FLEE, buf, ch, NULL, NULL, TO_CHAR );
			ch->accumulated_exp = 0;
			gain_exp( ch, 0 - los, FALSE);
			if ( wf ) level_ratio = URANGE( 1, wf->level / ch->level, 50 );
			if ( ch->pcdata->deity )
			{
				if ( wf && wf->race == ch->pcdata->deity->npcrace )
					adjust_favor( ch, 1, level_ratio );
				else
					if ( wf && wf->race == ch->pcdata->deity->npcfoe )
						adjust_favor( ch, 16, level_ratio );
					else
						adjust_favor( ch, 0, level_ratio );
			}
		}

		if ( ch->GetVictim() && !IS_NPC(ch->GetVictim()) ) {
			CHAR_DATA * vch = ch->GetVictim();
			CHAR_DATA * gch;

			for ( gch = vch->GetInRoom()->first_person; gch; gch = gch->next_in_room ) {
				if ( !is_same_group(gch, vch) ) {
					continue;
				} else if ( !gch->IsFighting()) {
					continue;
				}

				send_to_char("Your victim has fled!\r\n", gch);

				gch->GainAccumulatedXp();
			}
		}

		if ( in_arena(ch) ) {
			char buf[MAX_STRING_LENGTH];

			sprintf(buf, "%s proves %s true cowardice as %s runs screaming from %s.", NAME(ch), SEX_HIS(ch), SEX_HE(ch), NAME(ch->GetVictim()));
			talk_channel(ch, buf, CHANNEL_ARENA, "");
		}
		//stop_fighting( ch, TRUE );
		ch->StopAllFights();
		return;
	}

	if ( !in_arena(ch) ) {
		los = (int) ((exp_level(ch, ch->level+1) - exp_level(ch, ch->level)) * 0.01);
		los += ch->accumulated_exp;
		sprintf(buf, "You attempt to flee from combat!  You lose %d experience.\n\r", los );
		send_to_char( buf, ch );
		ch->accumulated_exp = 0;
		gain_exp( ch, 0 - los, FALSE);
	} else {
		send_to_char("It didn't work!\r\n", ch);
	}

	return;
}


/* show all ongoing fights - good for deciding when to copyover/reboot safely */
void do_fights(CHAR_DATA *ch, const char* argument)
{
CHAR_DATA *vch;
int count;

   (void)argument; /* for now */
   count=0;
   for(vch=first_char; vch; vch=vch->next)
   {
	   if(!IS_NPC(vch) && vch->IsFighting() )
	   {/* show details */
		   ch_printf(ch,"%12s %11s\n",
				   NAME(vch),
				   vnum_to_dotted(vch->GetInRoom()->vnum)
				   );
		   count++;
	   }
   }
   if(count==0)
     ch_printf(ch,"There are no fights presently.\n");
}

void do_sla(CHAR_DATA *ch, const char* argument)
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA *victim;
   SOCIALTYPE *social;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "immolate" ) )
    {
      act( AT_FIRE, "Your fireball turns $N into a blazing inferno.",  ch, NULL, victim, TO_CHAR    );
      act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT    );
      act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "shatter" ) )
    {
      act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
      act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
      act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "demon" ) )
    {
      act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_NOTVICT );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "pounce" ))
    {
      act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "slit" ) )
    {
      act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
    }
   else if (( social=find_social(arg2)) != NULL )
     {
	    color_social(social->others_found, ch, NULL, victim, TO_NOTVICT);
    	color_social(social->char_found, ch, NULL, victim, TO_CHAR);
    	color_social(social->vict_found, ch, NULL, victim, TO_VICT);
     }
    else
    {
      act( AT_IMMORT, "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
      act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
      act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    }

    set_cur_char(victim);
    raw_kill( ch, victim );
    return;
}

/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */
ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
               OBJ_DATA *projectile, sh_int dist )
{
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int plusris;
	int dam;
	int diceroll;
	int prof_bonus;
	int prof_gsn = -1;
	int dt; /* Damage type */
	ch_ret retcode;
	int dam_msg;

	if ( !projectile ) {
		return rNONE;
	}

	dt = TYPE_HIT;
	if ( projectile->item_type == ITEM_PROJECTILE )
	{
		dt += projectile->value[OBJECT_PROJECTILE_DAMAGETYPE];
		dam_msg = projectile->value[OBJECT_PROJECTILE_DAMAGEMESSAGE];
	}
	else
	{
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "Object %s (%s) is not a projectile but is being put through projectile_hit!!",
				projectile->name_.c_str(), vnum_to_dotted(projectile->pIndexData->vnum) );
		bug(buf);
		return rERROR;
	}

	/*
	 * Can't beat a dead char!
	 */
	if ( victim->position == POS_DEAD || char_died(victim) ) {
		extract_obj(projectile, TRUE);
		return rVICT_DIED;
	}

	if ( wield ) {
		prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
	} else {
		prof_bonus = 0;
	}

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) ) {
        thac0_00 = ch->mobthac0;
        thac0_32 =  0;
    } else {
        thac0_00 = class_table[ch->Class]->thac0_00;
        thac0_32 = class_table[ch->Class]->thac0_32;
    }

    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - ch->getHitRoll() - (ch->getDex() - 12)
	            - ( (GetRangedProficiency(ch, prof_gsn) / 10) - 7 ) + dist/3;
    victim_ac = UMAX( -19, (int) (victim->getAC() / 10) );

	if ( ch->getName() == "Tobi" )
	{
		ch_printf(ch, "Your thac0: %3d   Victim AC before changes: %3d\n\r", thac0, victim_ac);
		ch_printf(ch, "Your hitroll: %3d   Dex-12: %3d  Proficiencymod: %3d  Dist/4 %3d\n\r",
		          ch->getHitRoll(), (ch->getDex() - 12), ( (GetRangedProficiency(ch, prof_gsn) / 10) - 7 ),
				  dist/3);
	}

    /* if you can't see what's coming... */
    if ( !can_see_obj( victim, projectile) ) {
        victim_ac += 1;
    }
    if ( !can_see( ch, victim ) ) {
        victim_ac -= 4;
    }

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
        ;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
        /* Miss. */
        if ( prof_gsn != -1 ) {
            learn_from_failure( ch, prof_gsn );
        }

		learn_from_failure(ch, skill_lookup("fire"));

        /* Do something with the projectile */
		ProjectileDecreaseCondition(projectile);
		bNoShields = TRUE;
        damage( ch, victim, 0, dt, dam_msg );
		bNoShields = FALSE;
        tail_chain( );
        return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

	if (wield)
		dam = (number_range(projectile->value[OBJECT_PROJECTILE_MINDAMAGE], projectile->value[OBJECT_PROJECTILE_MAXDAMAGE]) )
				* (wield->value[OBJECT_WEAPON_POWER] / 100);
	else
		dam = number_range(projectile->value[OBJECT_PROJECTILE_MINDAMAGE], projectile->value[OBJECT_PROJECTILE_MAXDAMAGE]);


    /*
     * Bonuses.
     */
    dam += ch->getDamRoll();

    if ( prof_bonus ) {
        dam += prof_bonus / 4;
    }

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 ) {
        dam += (int) (dam * LEARNED(ch, gsn_enhanced_damage) / 120);
        learn_from_success( ch, gsn_enhanced_damage );
    }

    if ( !IS_AWAKE(victim) ) {
        dam *= 2;
    }

    if ( dam <= 0 ) {
        dam = 1;
    }

    plusris = 0;

    if ( IS_OBJ_STAT(projectile, ITEM_MAGIC) ) {
        dam = ris_damage( victim, dam, RIS_MAGIC );
    } else {
        dam = ris_damage( victim, dam, RIS_NONMAGIC );
    }

    /*
     * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll -Thoric
     */
    if ( wield ) {
        plusris = obj_hitroll( wield );
    }

    /* check for RIS_PLUSx                  -Thoric */
    if ( dam ) {
        int x, res, imm, sus, mod;

        if ( plusris )
            plusris = RIS_PLUS1 << UMIN(plusris, 7);

        /* initialize values to handle a zero plusris */
        imm = res = -1;  sus = 1;

        /* find high ris */
        for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 ) {
            if ( IS_SET( victim->immune, x ) ) {
                imm = x;
            }
            if ( IS_SET( victim->resistant, x ) ) {
                res = x;
            }
            if ( IS_SET( victim->susceptible, x ) ) {
                sus = x;
            }
        }
        mod = 10;
        if ( imm >= plusris ) {
            mod -= 10;
        }
        if ( res >= plusris ) {
            mod -= 2;
        }
        if ( sus <= plusris ) {
            mod += 2;
        }

        /* check if immune */
        if ( mod <= 0 ) {
            dam = -1;
        }
        if ( mod != 10 ) {
            dam = (dam * mod) / 10;
        }
    }

    if ( prof_gsn != -1 ) {
        if ( dam > 0 ) {
            learn_from_success( ch, prof_gsn );
        } else {
            learn_from_failure( ch, prof_gsn );
        }
    }
	if (dam > 0)
		learn_from_success(ch, skill_lookup("fire"));
	else
		learn_from_failure(ch, skill_lookup("fire"));
    /* immune to damage */
    if ( dam == -1 ) {
        if ( dt >= 0 && dt < top_sn ) {
            SkillType *skill = skill_table[dt];
            bool found = FALSE;

            if ( skill->imm_char && skill->imm_char[0] != '\0' ) {
                act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
                found = TRUE;
            }

            if ( skill->imm_vict && skill->imm_vict[0] != '\0' ) {
                act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
                found = TRUE;
            }

            if ( skill->imm_room && skill->imm_room[0] != '\0' ) {
                act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
                found = TRUE;
            }

            if ( found ) {
				ProjectileDecreaseCondition(projectile);
                return rNONE;
            }
        }
        dam = 0;
    }

	bNoShields = TRUE;
	retcode = damage( ch, victim, dam, dt, dam_msg);
	bNoShields = FALSE;
    if ( retcode != rNONE ) {
		ProjectileDecreaseCondition(projectile);
        return retcode;
    }
    if ( char_died(ch) ) {
		ProjectileDecreaseCondition(projectile);
        return rCHAR_DIED;
    }
    if ( char_died(victim) ) {
        ProjectileDecreaseCondition(projectile);
		return rVICT_DIED;
    }

    retcode = rNONE;
    if ( dam == 0 ) {
		ProjectileDecreaseCondition(projectile);
        return retcode;
    }

    /* weapon spells    -Thoric */
    if ( projectile
    &&  !IS_SET(victim->immune, RIS_MAGIC)
    &&  !IS_SET(victim->GetInRoom()->room_flags, ROOM_NO_MAGIC) )
    {
        AFFECT_DATA *aff;

        for ( aff = projectile->pIndexData->first_affect; aff; aff = aff->next ) {
            if ( aff->location == APPLY_WEAPONSPELL
            &&   IS_VALID_SN(aff->modifier)
            &&   skill_table[aff->modifier]->spell_fun )
            {
                retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, ch->level, ch, victim );
            }
        }

        if ( retcode != rNONE || char_died(ch) || char_died(victim) ) {
			ProjectileDecreaseCondition(projectile);
            return retcode;
        }

        for ( aff = projectile->first_affect; aff; aff = aff->next ) {
            if ( aff->location == APPLY_WEAPONSPELL
            &&   IS_VALID_SN(aff->modifier)
            &&   skill_table[aff->modifier]->spell_fun ) {
                retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (ch->level)/3, ch, victim );
            }
        }

        if ( retcode != rNONE || char_died(ch) || char_died(victim) ) {
			ProjectileDecreaseCondition(projectile);
            return retcode;
        }
    }

	ProjectileDecreaseCondition(projectile);

    tail_chain( );
    return retcode;
}

