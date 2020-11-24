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
 *														  Regular update module													*
 ****************************************************************************/

#include "globals.h"
#include <sys/types.h>
#ifdef unix
	#include <sys/time.h>
	#include <sys/times.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "connection.h"
#include "World.h"

#include "commands.h"

#include "paths.const.h"

/*
 * Local functions.
 */
int	hit_gain	     (CHAR_DATA *ch);
int	mana_gain	     (CHAR_DATA *ch);
int   	move_gain	     (CHAR_DATA *ch);
void	mobile_update        (void);
void	weather_update       (void);
void	char_update	     (void);
void    rprog_update         (void);
void	obj_update	     (void);
void	aggr_update          (void);
void	room_act_update      (void);
void	obj_act_update       (void);
void	char_check           (void);
void    drunk_randoms        (CHAR_DATA *ch);
void    halucinations        (CHAR_DATA *ch);
void	subtract_times       (struct timeval *etime, struct timeval *stime);
int     char_in_auctionroom  (CHAR_DATA *ch);


/*
 * Global Variables
 */

CHAR_DATA *				gch_prev;
OBJ_DATA *				gobj_prev;

CHAR_DATA *				timechar;

const char * behead_corpse_descs[] =
   { 
     "The beheaded corpse of %s is in the last stages of decay.", 
     "The beheaded corpse of %s is crawling with vermin.",
     "The beheaded corpse of %s fills the air with a foul stench.",
     "The beheaded corpse of %s is buzzing with flies.",
     "The beheaded corpse of %s lies here."
   };



const char * corpse_descs[] =
   { 
     "The corpse of %s is in the last stages of decay.", 
     "The corpse of %s is crawling with vermin.",
     "The corpse of %s fills the air with a foul stench.",
     "The corpse of %s is buzzing with flies.",
     "The corpse of %s lies here."
   };

extern int      top_exit;


/*
 * Advancement stuff.
 */
void deadvance_level( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac;

	sprintf( buf, "the %s",
			title_table [ch->Class] [ch->level-1] [ch->sex == SEX_FEMALE ? 1 : 0] );

	if( ch->pcdata->title_.c_str()[0] !='\0' && ch->pcdata->title_.c_str()[1] == ':')
		;
	else
		set_title( ch, buf );


	add_hp = ch->GetLevelGainHP();
	add_move = ch->GetLevelGainMove();
	add_mana = ch->GetLevelGainMana();

	add_prac	= wis_app[ch->getWis()].practice;

	add_hp		= UMAX(  1, add_hp   );
	add_mana	= UMAX(  0, add_mana );
	add_move	= UMAX( 10, add_move );

	ch->BaseMaxHp -= add_hp;
	ch->BaseMaxMana -= add_mana;
	ch->BaseMaxMove -= add_move;

	ch->max_hit					-= add_hp;
	ch->max_mana				-= add_mana;
	ch->max_move				-= add_move;
	ch->practice				-= add_prac;

	ch->practice = MAX(ch->practice, 0);
	ch->max_hit  = MAX(ch->max_hit,  0);
	ch->max_mana = MAX(ch->max_mana, 0);
	ch->max_move = MAX(ch->max_move, 0);

	if ( !IS_NPC(ch) )
		REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

	if ( IS_VAMPIRE(ch) )
		sprintf( buf,
				"Your gain is: -%d/%d hp, -%d/%d bp, -%d/%d mv -%d/%d prac.\n\r",
				add_hp,				ch->max_hit,
				1,					ch->level + 10,
				add_move,				ch->max_move,
				add_prac,				ch->practice
			   );
	else
		sprintf( buf,
				"Your gain is: -%d/%d hp, -%d/%d mana, -%d/%d mv -%d/%d prac.\n\r",
				add_hp,				ch->max_hit,
				add_mana,				ch->max_mana,
				add_move,				ch->max_move,
				add_prac,				ch->practice
			   );
	set_char_color( AT_WHITE, ch );
	send_to_char( buf, ch );
	return;
}
const char *hoodname[4]=
{
   "Avatar"
  ,"Avatarhood"
  ,"Fiendhood"
  ,"Sagehood"
};

int hoodcolor[4]=
{
  AT_LBLUE
 ,AT_LBLUE
 ,AT_RED
 ,AT_GREEN
};

void advance_level( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac;
	
	/*				save_char_obj( ch );*/
	sprintf( buf, "the %s",
		title_table [ch->Class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );
	if(ch->pcdata->title_.c_str()[0]!='\0' && ch->pcdata->title_.c_str()[1]==':')
		;
	else
		set_title( ch, buf );
	
	add_prac	= wis_app[ch->getWis()].practice;
	
	add_hp      = ch->GetLevelGainHP();
	add_mana    = ch->GetLevelGainMana();
	add_move    = ch->GetLevelGainMove();

	ch->BaseMaxHp += add_hp;
	ch->BaseMaxMana += add_mana;
	ch->BaseMaxMove += add_move;
	
	ch->max_hit += add_hp;
	ch->max_mana				+= add_mana;
	ch->max_move				+= add_move;
	ch->practice				+= add_prac;
	
	if ( !IS_NPC(ch) )
		REMOVE_BIT( ch->act, PLR_BOUGHT_PET );
	
	if ( ch->level == LEVEL_HERO_MIN )
	{
		
		sprintf( buf, "%s has just achieved %s!",
			ch->getShort().c_str(),hoodname[ch->pcdata->ethos] );
		itorSocketId itor;
		for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
		{
			// we know that the world's player connection list only holds player connections IDs,
			// so we can safely cast it to PlayerConnection*
			PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

			if ( d->ConnectedState == CON_PLAYING && d->GetCharacter() != ch )
			{
				set_char_color( hoodcolor[ch->pcdata->ethos], d->GetCharacter() );
				send_to_char( buf,	d->GetCharacter() );
				send_to_char( "\n\r",	d->GetCharacter() );
			}
		}

		set_char_color( AT_WHITE, ch );
		do_help( ch, "M_ADVHERO_" );
	}
	if ( ch->level < LEVEL_IMMORTAL )
	{
		if ( IS_VAMPIRE(ch) )
			sprintf( buf,
			"Your gain is: %d/%d hp, %d/%d bp, %d/%d mv %d/%d prac.\n\r",
			add_hp, 			ch->max_hit,
			1,					ch->level + 10,
			add_move,				ch->max_move,
			add_prac,				ch->practice
			);
		else
			sprintf( buf,
			"Your gain is: %d/%d hp, %d/%d mana, %d/%d mv %d/%d prac.\n\r",
			add_hp, 			ch->max_hit,
			add_mana,				ch->max_mana,
			add_move,				ch->max_move,
			add_prac,				ch->practice
			);
		set_char_color( AT_WHITE, ch );
		send_to_char( buf, ch );
	}
	return;
}	

void gain_exp( CHAR_DATA *ch, int gain, bool bAccumulate )
{
	int modgain;
	int oldexp;

	if ( IS_NPC(ch) || ch->level >= LEVEL_HERO_MAX )
		return;


	if ( in_arena(ch) ) {
		return;
	}
    
	/* Bonus for deadly lowbies */
	/* what the heck is this? -Ksilyan 
	modgain = gain;*/
	modgain = gain;

	oldexp = ch->exp;
    
	if ( bAccumulate )
	{
		ch->accumulated_exp = UMAX( 0, ch->accumulated_exp + modgain );
		return;
	}
	else
	{
		ch->exp =  UMAX( 0, ch->exp + modgain );
		ch->exp += ch->accumulated_exp;
		ch->accumulated_exp = 0;
	}

	if ( ch->exp < exp_level(ch, ch->level) ) {
#if 0 /* never lose levels anymore */
		int count = 0;
		char buf[MAX_STRING_LENGTH];
		while ( ch->level > 2 && ch->exp < exp_level(ch, ch->level) )
		{
			if ( ch->level >= LEVEL_HERO_MIN ) { return; }
				set_char_color( AT_WHITE + AT_BLINK, ch );
			ch_printf( ch, "You feel your power slipping away from you.\r\n");
			deadvance_level( ch );
			ch->level--;
			count++;
		}
	
		if ( count > 0 ) {
			sprintf(buf, "%s lost %d levels due to experience loss.", ch->name, count);
			to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
		}
#endif
	} else {
		// Changed by Ksilyan: levels are now gained instantly
		if ( ch->exp >= exp_level(ch, ch->level+1) 
			&& oldexp < exp_level(ch, ch->level+1))
		{
			ch_printf(ch, "You have obtained enough experience to level!\r\n");
			//ch_printf(ch, "Seek out your guildmaster and 'gain'.\r\n");
		}
    
		//#if 0
		while ( ch->level+1 < LEVEL_HERO_MIN && ch->exp >= exp_level(ch, ch->level+1))
		{
			set_char_color( AT_WHITE + AT_BLINK, ch );
			ch_printf( ch, "You have now obtained experience level %d!\r\n", ++ch->level );
			advance_level( ch );
		}
		//#endif        
}

return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int hung_flag = 0;
    int thir_flag = 0;

    if ( ch->position == POS_MEDITATING )
        return 0;
    
    if ( IS_NPC(ch) )
    {
		if (!IS_SET(ch->act, ACT_NOREGEN))
			gain = ch->level * 3 / 2;
    }
    else
    {
				/* replaced old hard coded regen base - fiz */
				gain = HP_REGEN_BASE;

				switch ( ch->position )
				{
						case POS_DEAD:     return 0;
						case POS_MORTAL:   return -1;
						case POS_INCAP:    return -1;
						case POS_STUNNED:  return 1;
				}

        if  ( IS_VAMPIRE(ch) ) 
				{
            if  ( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )
								gain /= 2;
						else
						if  ( ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level) )
                gain *= 2;
            if  ( ch->isOutside() )
						{
								switch(weather_info.sunlight)
								{
										case SUN_RISE:
										case SUN_SET:
												gain /= 2;
												break;
										case SUN_LIGHT:
												gain /= 4;
												break;
								}
						}
        }

				/* race hp regen modifiers - fiz */
				switch ( ch->race )
				{
						case RACE_HUMAN:		  gain += HP_REGEN_HUMAN;				break;
						case RACE_ELF:			  gain += HP_REGEN_ELF;					break;
						case RACE_DWARF:		  gain += HP_REGEN_DWARF;				break;
						case RACE_HALFLING:		  gain += HP_REGEN_HALFLING;	break;
						case RACE_HALF_OGRE:  gain += HP_REGEN_HALF_OGRE;		break;
						case RACE_HALF_ORC:   gain += HP_REGEN_HALF_ORC;		break; 
						case RACE_HALF_TROLL: gain += HP_REGEN_HALF_TROLL;  break;
						case RACE_HALF_ELF:   gain += HP_REGEN_HALF_ELF;		break;
						case RACE_GITH:			  gain += HP_REGEN_GITH;				break;
						case RACE_GLANDAR:		  gain += HP_REGEN_GLANDAR;				break;
						case RACE_GNOME:		  gain += HP_REGEN_GNOME;				break;
						case RACE_KATRIN:		  gain += HP_REGEN_KATRIN;				break;
						case RACE_ARAYAN:		  gain += HP_REGEN_ARAYAN;				break;
						case RACE_HARAN:		  gain += HP_REGEN_HARAN;				break;
						case RACE_EYAN:			  gain += HP_REGEN_EYAN;				break;
						case RACE_MINOTAUR:		  gain += HP_REGEN_MINOTAUR;	break;
						case RACE_ELDARI:		  gain += HP_REGEN_ELDARI;				break;
						case RACE_PIXIE:		  gain += HP_REGEN_PIXIE;				break;
						default:						  gain += HP_REGEN_ZERO;				break;
				}

				if  ( ch->pcdata->condition[COND_FULL] <= 2 && ch->Class != CLASS_VAMPIRE)
				{
						hung_flag = 1;
				}

				if  ( ch->pcdata->condition[COND_THIRST] <= 2 && ch->Class != CLASS_VAMPIRE)
				{
						thir_flag = 1;
				}

				/* mod to regen when hungry or thirsty - fiz */
				if ( ( hung_flag + thir_flag ) == 0 )
				{
						if  ( ch->position == POS_SLEEPING )
								gain += ch->getCon();				
						else if  ( ch->position == POS_RESTING )
								gain += ( ch->getCon()/2 );					
				}
				else if ( ( hung_flag + thir_flag ) == 1 )
				{
						if  ( ch->position == POS_SLEEPING )
								gain += ( ch->getCon()/2 );
						else if  ( ch->position == POS_RESTING )
								gain += ( ch->getCon()/4 );					
				}
				else
				{
						if  ( ch->position == POS_SLEEPING )
								gain += 0;
						else if  ( ch->position == POS_RESTING )
								gain -= 1;
						else
								gain -= 2;
				}
    }

    if  ( IS_AFFECTED(ch, AFF_POISON) )
				gain /= 4;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int flag = 0;

    if ( IS_NPC(ch) )
    {
				gain = ch->level;
    }
    else
    {
				/* replaced hard coded regen base - fiz */
				gain = MN_REGEN_BASE;

				/* no regen if dying */
				if  ( ch->position < POS_SLEEPING )
						return 0;

				/* race mana regen modifiers - fiz */
				switch ( ch->race )
				{
						case RACE_HUMAN:		  gain += MN_REGEN_HUMAN;				break;
						case RACE_ELF:			  gain += MN_REGEN_ELF;					break;
						case RACE_DWARF:		  gain += MN_REGEN_DWARF;				break;
						case RACE_HALFLING:		  gain += MN_REGEN_HALFLING;	break;
						case RACE_HALF_OGRE:  gain += MN_REGEN_HALF_OGRE;		break;
						case RACE_HALF_ORC:   gain += MN_REGEN_HALF_ORC;		break; 
						case RACE_HALF_TROLL: gain += MN_REGEN_HALF_TROLL;  break;
						case RACE_HALF_ELF:   gain += MN_REGEN_HALF_ELF;		break;
						case RACE_GITH:			  gain += MN_REGEN_GITH;				break;
						case RACE_GLANDAR:		  gain += MN_REGEN_GLANDAR;				break;
						case RACE_GNOME:		  gain += MN_REGEN_GNOME;				break;
						case RACE_KATRIN:		  gain += MN_REGEN_KATRIN;				break;
						case RACE_ARAYAN:		  gain += MN_REGEN_ARAYAN;				break;
						case RACE_HARAN:		  gain += MN_REGEN_HARAN;				break;
						case RACE_EYAN:			  gain += MN_REGEN_EYAN;				break;
						case RACE_MINOTAUR:		  gain += MN_REGEN_MINOTAUR;	break;
						case RACE_ELDARI:		  gain += MN_REGEN_ELDARI;				break;
						case RACE_PIXIE:		  gain += MN_REGEN_PIXIE;				break;
						default:						  gain += MN_REGEN_ZERO;				break;
				}

				/* mods to hunger thirst effects on regen - fiz */
				if  ( ch->pcdata->condition[COND_FULL] <= 2 && ch->Class != CLASS_VAMPIRE )
						flag += 1;

				if  ( ch->pcdata->condition[COND_THIRST] <= 2 && ch->Class != CLASS_VAMPIRE )
						flag += 1;

				if ( flag == 0 )
				{
						if  ( ch->position == POS_SLEEPING )
						{
								gain += ch->getInt() * 2;
						}
						else if  ( ch->position == POS_RESTING )
						{
								gain += ch->getInt();
						}
						else
						{
								gain += ( ch->getInt()/2 );
						}
				}

    }

    if  ( IS_AFFECTED( ch, AFF_POISON ) )
    {
        if ( ch->position == POS_MEDITATING ) {
          send_to_char("The poison is making it impossible to concentrate!\r\n",
                    ch);
          act(AT_ACTION, "$n unties $mself from $s knot, and stands up.",
                  ch, NULL, NULL, TO_ROOM);
          gain = 0;
          ch->position = POS_STANDING;
        }
	    gain /= 4;
    }

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;
    int flag = 0;

    if ( ch->position == POS_MEDITATING )
        return 0;
    
    if ( IS_NPC(ch) )
    {
				gain = ch->level;
    }
    else
    {
				gain = UMAX( 15, 2 * ch->level );

				switch ( ch->position )
				{
						case POS_DEAD:			   return 0;
						case POS_MORTAL:   return -1;
						case POS_INCAP:    return -1;
						case POS_STUNNED:  return 1;
				}

        if ( IS_VAMPIRE(ch) ) 
				{
            if ( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )
								gain /= 2;
						else
						if  ( ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level) )
                gain *= 2;
            if  ( ch->isOutside() )
						{
								switch(weather_info.sunlight)
								{
										case SUN_RISE:
										case SUN_SET:
												gain /= 2;
												break;
										case SUN_LIGHT:
												gain /= 4;
										break;
								}
						}
        }

				if  ( ch->pcdata->condition[COND_FULL]   == 0 && ch->Class != CLASS_VAMPIRE)
						flag += 1;

				if  ( ch->pcdata->condition[COND_THIRST] == 0 && ch->Class != CLASS_VAMPIRE)
						flag += 1;

				if  ( flag == 0 )
				{
						if  ( ch->position == POS_SLEEPING )
								gain += ch->getDex() * 2;
						if  ( ch->position == POS_RESTING )
								gain += ch->getDex();
				}
				else
				{
						if  ( ch->position == POS_SLEEPING )
								gain += ch->getDex();
						if  ( ch->position == POS_RESTING )
								gain += ( ch->getDex()/2 );
				}
    }

    if  ( IS_AFFECTED(ch, AFF_POISON) )
				gain /= 4;

    return UMIN(gain, ch->max_move - ch->move);
}


void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
	int condition;
	ch_ret retcode = rNONE;

	if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL || NOT_AUTHED(ch))
		return;

	condition													= ch->pcdata->condition[iCond];
	if ( iCond == COND_BLOODTHIRST )
		ch->pcdata->condition[iCond]    = URANGE( 0, condition + value,
				10 + ch->level );
	else
		ch->pcdata->condition[iCond]    = URANGE( 0, condition + value, 48 );

	if ( ch->pcdata->condition[iCond] == 0 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_HUNGRY, ch );
					send_to_char( "You are STARVING!\n\r",  ch );
					/*            act( AT_HUNGRY, "$n is starved half to death!", ch, NULL, NULL, TO_ROOM); */
					if  ( number_bits(1) == 0 )
						worsen_mental_state( ch, 1 );
					retcode = damage(ch, ch, 2, TYPE_UNDEFINED, 0);
				}
				break;

			case COND_THIRST:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_THIRSTY, ch );
					send_to_char( "You are DYING of THIRST!\n\r", ch );
					/*            act( AT_THIRSTY, "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM); */
					worsen_mental_state( ch, 1 );
					retcode = damage(ch, ch, 2, TYPE_UNDEFINED, 0);
				}
				break;

			case COND_BLOODTHIRST:
				if ( ch->level < LEVEL_IMMORTAL )
				{
					set_char_color( AT_BLOOD, ch );
					send_to_char( "You are starved to feast on blood!\n\r", ch );
					/*            act( AT_BLOOD, "$n is suffering from lack of blood!", ch,
								  NULL, NULL, TO_ROOM);*/
					worsen_mental_state( ch, 2 );
					if  ( ch->hit >= 20 )
					{
						retcode = damage(ch, ch, ch->max_hit / 20, TYPE_UNDEFINED, 0);
					}
				}
				break;
			case COND_DRUNK:
				if ( condition != 0 ) {
					set_char_color( AT_SOBER, ch );
					send_to_char( "You are sober.\n\r", ch );
				}
				retcode = rNONE;
				break;
			default:
				bug( "Gain_condition: invalid condition type %d", iCond );
				retcode = rNONE;
				break;
		}
	}

	if ( retcode != rNONE )
		return;

	if ( ch->pcdata->condition[iCond] == 1 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_HUNGRY, ch );
					send_to_char( "You are really hungry.\n\r",  ch );
					/*            act( AT_HUNGRY, "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM);*/
					if ( number_bits(1) == 0 )
						worsen_mental_state( ch, 1 );
				} 
				break;

			case COND_THIRST:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_THIRSTY, ch );
					send_to_char( "You are really thirsty.\n\r", ch );
					worsen_mental_state( ch, 1 );
					/*						act( AT_THIRSTY, "$n looks a little parched.", ch, NULL, NULL, TO_ROOM);*/
				} 
				break;

			case COND_BLOODTHIRST:
				if ( ch->level < LEVEL_IMMORTAL )
				{
					set_char_color( AT_BLOOD, ch );
					send_to_char( "You have a growing need to feast on blood!\n\r", ch );
					/*            act( AT_BLOOD, "$n gets a strange look in $s eyes...", ch,
								  NULL, NULL, TO_ROOM);*/
					worsen_mental_state( ch, 1 );
				}
				break;
			case COND_DRUNK:
				if ( condition != 0 ) {
					set_char_color( AT_SOBER, ch );
					send_to_char( "You are feeling a little less light headed.\n\r", ch );
				}
				break;
		}
	}


	if ( ch->pcdata->condition[iCond] == 2 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_HUNGRY, ch );
					send_to_char( "You are hungry.\n\r",  ch );
				} 
				break;

			case COND_THIRST:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_THIRSTY, ch );
					send_to_char( "You are thirsty.\n\r", ch );
				} 
				break;

			case COND_BLOODTHIRST:
				if ( ch->level < LEVEL_IMMORTAL )
				{
					set_char_color( AT_BLOOD, ch );
					send_to_char( "You feel an urgent need for blood.\n\r", ch );
				}  
				break;
		}
	}

	if ( ch->pcdata->condition[iCond] == 3 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_HUNGRY, ch );
					send_to_char( "You are a mite peckish.\n\r",  ch );
				} 
				break;

			case COND_THIRST:
				if ( ch->Class == CLASS_VAMPIRE ) break;                       
				if ( ch->level < LEVEL_IMMORTAL && ch->Class != CLASS_VAMPIRE )
				{
					set_char_color( AT_THIRSTY, ch );
					send_to_char( "You could use a sip of something refreshing.\n\r", ch );
				} 
				break;

			case COND_BLOODTHIRST:
				if ( ch->level < LEVEL_IMMORTAL )
				{
					set_char_color( AT_BLOOD, ch );
					send_to_char( "You feel an aching in your fangs.\n\r", ch );
				}
				break;
		}
	}
	return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update( void )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	ExitData *pexit;
	int door;
	ch_ret     retcode;

	retcode = rNONE;

	/* Examine all mobs. */
	for ( ch = last_char; ch; ch = gch_prev )
	{
		set_cur_char( ch );
		if ( ch == first_char && ch->prev )
		{
			bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}

		gch_prev = ch->prev;

		if ( gch_prev && gch_prev->next != ch )
		{
			sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
					ch->getName().c_str() );
			bug( buf, 0 );					
			bug( "Short-cutting here", 0 );
			gch_prev = NULL;
			ch->prev = NULL;
			do_shout( ch, "Tarin says, 'Prepare for the worst!'" );
		}

		if ( !IS_NPC(ch) )
		{
			drunk_randoms(ch);
			halucinations(ch);
			continue;
		}

		/////////////////////////////////////////
		// Everything from here on is NPC only //
		/////////////////////////////////////////

		if ( !ch->GetInRoom()
				||   IS_AFFECTED(ch, AFF_CHARM)
				||   IS_AFFECTED(ch, AFF_PARALYSIS) )
			continue;

		/* Clean up 'animated corpses' that are not charmed' - Scryn */

		if ( ch->pIndexData->vnum == 5 && !IS_AFFECTED(ch, AFF_CHARM) )
		{
			if(ch->GetInRoom()->first_person)
				act(AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM);

			if(IS_NPC(ch)) /* Guard against purging switched? */
				extract_char(ch, TRUE);
			continue;
		}

		if ( !IS_SET( ch->act, ACT_RUNNING )
				&&   !IS_SET( ch->act, ACT_SENTINEL )
				&&   !ch->IsFighting() && ch->hunting )
		{
			ch->AddWait( 2 * PULSE_VIOLENCE );
			hunt_victim( ch );
			continue;
		}  

		if ( ch->vnum_destination || ch->GetDestinationChar() )  
		{  
			auto_walk(ch);  
			continue;  
		}

		/* Examine call for special procedure */
		if ( !IS_SET( ch->act, ACT_RUNNING )
				&&    ch->spec_fun )
		{
			if ( (*ch->spec_fun) ( ch ) )
				continue;
			if ( char_died(ch) )
				continue;
		}

		/* Check for mudprogram script on mob */
		if ( IS_SET( ch->pIndexData->progtypes, SCRIPT_PROG ) )
		{
			mprog_script_trigger( ch );
			continue;
		}

		if ( ch != cur_char )
		{
			bug( "Mobile_update: ch != cur_char after spec_fun", 0 );
			continue;
		}

		/* That's all for sleeping / busy monster */
		if ( ch->position != POS_STANDING )
			continue;

		if ( IS_SET(ch->act, ACT_MOUNTED ) )
		{
			if ( IS_SET(ch->act, ACT_AGGRESSIVE) )
				do_emote( ch, "snarls and growls." );
			continue;
		}

		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_SAFE )
				&&   IS_SET(ch->act, ACT_AGGRESSIVE) )
			do_emote( ch, "glares around and snarls." );


		/* MOBprogram random trigger */


		// Commented out area player count check:
		// some areas need their random actions
		// to continue even if there are no players
		//   -Ksilyan

		//if ( ch->GetInRoom()->area->nplayer > 0 )
		//{
		mprog_random_trigger( ch );
		if ( char_died(ch) )
			continue;
		if ( ch->position < POS_STANDING )
			continue;
		//}

		/* MOBprogram hour trigger: do something for an hour */
		mprog_hour_trigger(ch);

		if ( char_died(ch) )
			continue;

		rprog_hour_trigger(ch);
		if ( char_died(ch) )
			continue;

		if ( ch->position < POS_STANDING )
			continue;

		/* Scavenge */
		if ( IS_SET(ch->act, ACT_SCAVENGER)
				&&   ch->GetInRoom()->first_content
				&&   ch->carry_number < can_carry_n( ch )
				&&   number_bits( 2 ) == 0 )
		{
			OBJ_DATA *obj;
			OBJ_DATA *obj_best;
			int max;

			max         = 1;
			obj_best    = NULL;
			for ( obj = ch->GetInRoom()->first_content; obj; obj = obj->next_content )
			{
				if ( CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max 
						&& !IS_OBJ_STAT( obj, ITEM_BURIED ) )
				{
					obj_best    = obj;
					max         = obj->cost;
				}
			}

			if ( obj_best )
			{
				obj_from_room( obj_best );
				obj_to_char( obj_best, ch );
				act( AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
			}
		}

		/* Ksilyan
		 * Before wandering, check to see if
		 * we're a mob with a home.
		 * If we're not home, then go home.
		 */
		if ( ch->HomeVnum != 0 )
		{
			ch->vnum_destination = ch->HomeVnum;
			auto_walk(ch);
			continue;
		}

		/* Wander */
		if ( !IS_SET(ch->act, ACT_RUNNING)
				&& !ch->IsFighting() // Not sure why this is necessary, but *shrug*
				&&   !IS_SET(ch->act, ACT_SENTINEL)
				&&   !IS_SET(ch->act, ACT_PROTOTYPE)
				&& ( door = number_bits( 5 ) ) <= 9
				&& ( pexit = get_exit(ch->GetInRoom(), door) ) != NULL
				&&   pexit->to_room
				&&   !IS_SET(pexit->exit_info, EX_CLOSED)
				&&	 !IS_SET(pexit->exit_info, EX_NOMOBWANDER)
				&&   !( IS_SET(ch->act, ACT_STAYSECTOR) && ch->GetInRoom()->sector_type != pexit->to_room->sector_type )
				&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
				&&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)
				/* Ksilyan: can't wander if paralyzed!
				 * Not sure why it didn't already trigger above though..
				 * need to check indentation... *shrug*
				 */
				&&   !IS_AFFECTED(ch, AFF_PARALYSIS)
				&& ( !IS_SET(ch->act, ACT_STAY_AREA)
					||   pexit->to_room->area == ch->GetInRoom()->area ) )
		{
			retcode = move_char( ch, pexit, 0 );
			/* If ch changes position due
			   to it's or someother mob's
			   movement via MOBProgs,
			   continue - Kahn */
			if ( char_died(ch) )
				continue;
			if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
					||    ch->position < POS_STANDING )
				continue;
		}

		/* Flee */
		if ( ch->hit < ch->max_hit / 2
				&& ( door = number_bits( 4 ) ) <= 9
				&& ( pexit = get_exit(ch->GetInRoom(),door) ) != NULL
				&&   pexit->to_room
				&&   !IS_SET(pexit->exit_info, EX_CLOSED)
				&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) )
		{
			CHAR_DATA *rch;
			bool found;

			found = FALSE;
			for ( rch  = ch->GetInRoom()->first_person;
					rch;
					rch  = rch->next_in_room )
			{
				if ( is_fearing(ch, rch) )
				{
					switch( number_bits(2) )
					{
						case 0:
							sprintf( buf, "Get away from me, %s!", rch->getShort().c_str() );
							break;
						case 1:
							sprintf( buf, "Leave me be, %s!", rch->getShort().c_str() );
							break;
						case 2:
							sprintf( buf, "%s is trying to kill me!  Help!", rch->getShort().c_str() );
							break;
						case 3:
							sprintf( buf, "Someone save me from %s!", rch->getShort().c_str() );
							break;
					}
					do_yell( ch, buf );
					found = TRUE;
					break;
				}
			}
			if ( found )
				retcode = move_char( ch, pexit, 0 );
		}
	}

	return;
}

#define SUNLESS_MSGS 21
const char *sunless_msg[SUNLESS_MSGS]=
{
 "A small bird looks eastward, chirps hopefully, but gives up."
,"The flowers droop a little more."
,"The chirping of crickets slows down."
,"The blades of grass turn paler."
,"The leaves weaken and fade."
,"Fruit shrivels and darkens on the branches."
,"Patches of frost appear on the fields."
,"An early robin hunts for worms, but finds they've all been taken."
,"A rabbit returns to its hole with only a few limp carrots."
,"A groundhog peers out of its hole, finds no shadow, and returns."
,"A hungry squirrel can't remember where his acorns were hidden."
,"A very confused rooster looks around impatiently."
,"A sleepy owl yawns, wondering when it can finally rest."
,"A stuffed bat, too heavy to fly, belches in discomfort."
,"Everything seems COLDER."
,"A few stars are obscured by the lightless moon."
,"A sunflower collapses face down in the soil."
,"The road is littered with motionless reptiles."
,"Your teeth begin to chatter."
,"You shiver and try to keep warm."
,"A lost bee cannot find its way back to the hive."
};

int sun_broken(void)
{
/* return (qvar[QVAR_SUNWORKS].value==0); */
 return(FALSE);
}


static int season_sun[17]=
{
  -2,-2,-2,-1,-1,0,1,1,2,2,2,1,1,0,-1,-1,-2
};

int sunrise_hour(void)
{
  return(6 - season_sun[time_info.month]);
}

int sunset_hour(void)
{
  return(18 + season_sun[time_info.month]);
}

/*
 * Update the weather.
 */
void weather_update( void )
{
	char buf[MAX_STRING_LENGTH];
	int diff;
	sh_int AT_TEMP = AT_PLAIN;
	
	buf[0] = '\0';
	
	switch ( ++time_info.hour )
	{	
		case 12:
			weather_info.sunlight = SUN_LIGHT;
			strcat( buf, "It's noon." ); 
			AT_TEMP = AT_YELLOW;
			break;
			
		case 23:	   
			strcat( buf,"Soon it will be midnight.");
			AT_TEMP = AT_DGREY;
			break;
			
		case 24:
			time_info.hour = 0;
			time_info.day++;
			strcat( buf, "Midnight falls upon the land." );
			AT_TEMP = AT_DGREY;
			break;
			
			
		default:
		{
			int sunrise,sunset;
			
			sunrise = sunrise_hour();
			sunset = 18 + season_sun[time_info.month];
			
			if(time_info.hour==sunrise-1)
			{
				AT_TEMP = AT_DGREY;
				strcat(buf,"The eastern sky brightens.");
			}
			if(time_info.hour==sunrise)
			{
				weather_info.sunlight=SUN_RISE;
				strcat(buf,"The sun rises in the east.");
				AT_TEMP = AT_ORANGE;
			}
			else if(time_info.hour==sunrise+1)
			{
				weather_info.sunlight=SUN_LIGHT;
				strcat(buf,"The day has begun.");
				AT_TEMP = AT_YELLOW;
			}
			else if(time_info.hour==sunset-1)
			{
				weather_info.sunlight=SUN_SET;
				strcat(buf, "The sun slowly lowers in the west.");
				AT_TEMP = AT_ORANGE;
			}
			else if(time_info.hour==sunset)
			{
				weather_info.sunlight=SUN_SET;
				strcat(buf,"The sky slowly fades to red.");
				AT_TEMP = AT_BLOOD;
			}
			else if(time_info.hour==sunset+1)
			{
				weather_info.sunlight=SUN_DARK;
				strcat(buf,"The night has begun.");
				AT_TEMP = AT_DGREY;
			}
		}
	}
	
	if(sun_broken())
	{
		weather_info.sunlight=SUN_DARK;
		buf[0]='\0'; /* ignore everything previous */
		AT_TEMP = AT_DGREY;
		if(time_info.hour==0)
		{
			strcat(buf,"The date changes during this endless midnight.");
		}
		if(time_info.hour==12)
		{
			int i;
			
			while((i=number_bits(5))>=SUNLESS_MSGS)
				;
			strcat(buf,sunless_msg[i]);
		}
	}
	
	if ( time_info.day	 >= 30 )
	{
		time_info.day = 0;
		time_info.month++;
	}
	
	if ( time_info.month >= 17 )
	{
		time_info.month = 0;
		time_info.year++;
	}
	
	if ( buf[0] != '\0' )
	{
		itorSocketId itor;
		for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
		{
			// we know that the world's player connection list only holds player connections IDs,
			// so we can safely cast it to PlayerConnection*
			PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

			if ( d->ConnectedState == CON_PLAYING
				&&	 d->GetCharacter()->isOutside()
				&&	 IS_AWAKE(d->GetCharacter()) )
				act( AT_TEMP, buf, d->GetCharacter(), 0, 0, TO_CHAR );
		}
		buf[0] = '\0';
	}
	/*
	* Weather change.
	*/
	if ( time_info.month >= 9 && time_info.month <= 16 )
		diff = weather_info.mmhg >	985 ? -2 : 2;
	else
		diff = weather_info.mmhg > 1015 ? -2 : 2;
	
	weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
	weather_info.change    = UMAX(weather_info.change, -12);
	weather_info.change    = UMIN(weather_info.change,	12);
	
	weather_info.mmhg += weather_info.change;
	weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
	weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);
	
	AT_TEMP = AT_GREY;
	
	if(sun_broken())
	{
		weather_info.sky=SKY_CLOUDLESS;
		weather_info.mmhg=1010;
		weather_info.change=0;
	}
	
	
	switch ( weather_info.sky )
	{
		default: 
			bug( "Weather_update: bad sky %d.", weather_info.sky );
			weather_info.sky = SKY_CLOUDLESS;
			break;
			
		case SKY_CLOUDLESS:
			if ( weather_info.mmhg <  990
				|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
			{
				strcat( buf, "The sky is getting cloudy." );
				weather_info.sky = SKY_CLOUDY;
				AT_TEMP = AT_GREY;
			}
			break;
			
		case SKY_CLOUDY:
			if ( weather_info.mmhg <  970
				|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
			{
				strcat( buf, "Raindrops start to beat down on the ground." );
				weather_info.sky = SKY_RAINING;
				AT_TEMP = AT_BLUE;
			}
			
			if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
			{
				strcat( buf, "The clouds disappear." );
				weather_info.sky = SKY_CLOUDLESS;
				AT_TEMP = AT_WHITE;
			}
			break;
			
		case SKY_RAINING:
			if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
			{
				strcat( buf, "Thunder rolls and lightning flashes in the sky." );
				weather_info.sky = SKY_LIGHTNING;
				AT_TEMP = AT_YELLOW;
			}
			
			if ( weather_info.mmhg > 1030
				|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
			{
				strcat( buf, "The rain stopped." );
				weather_info.sky = SKY_CLOUDY;
				AT_TEMP = AT_WHITE;
			}
			break;
			
		case SKY_LIGHTNING:
			if ( weather_info.mmhg > 1010
				|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
			{
				strcat( buf, "The lightning has stopped." );
				weather_info.sky = SKY_RAINING;
				AT_TEMP = AT_GREY;
				break;
			}
			break;
	}
	
	if ( buf[0] != '\0' )
	{
		itorSocketId itor;
		for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
		{
			// we know that the world's player connection list only holds player connections IDs,
			// so we can safely cast it to PlayerConnection*
			PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

			if ( d->ConnectedState == CON_PLAYING
				&&	 d->GetCharacter()->isOutside()
				&&	 IS_AWAKE(d->GetCharacter()) )
			{
				act( AT_TEMP, buf, d->GetCharacter(), 0, 0, TO_CHAR );
			}
		}
	}
	
	return;
}

void rprog_update( void )
{
	ROOM_INDEX_DATA *room;
	int count;
	
	for ( count = 0; count < 32768; ++count )
	{
		if ( (room = get_room_index(count)) == NULL ) {
			continue;
		}
		if ( !room->first_person ) {
			continue;
		}
		
		rprog_random_trigger(room->first_person);
	}
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */

/* Testaur: check for screwed up follow/groups */
#define MAX_LEADERS 32 /* will never happen, right? */

void char_update( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_save = NULL;
	sh_int save_count = 0;


	/* This leader/master fix should no longer be necessary,
	 * with the ID checks -Ksilyan

	 int i=0;
	 int ldrcnt=0;
	 CHAR_DATA *ldr[MAX_LEADERS];
	 int       ldrflag[MAX_LEADERS];
	 char      buf[MAX_STRING_LENGTH];

	// 1) scan for alleged master/leaders
	for ( ch=first_char; ch; ch=ch->next )
	{
	if(ch->master)
	{
	for(i=0;i<ldrcnt;i++)
	if(ch->master==ldr[i])
	break;
	if(i==ldrcnt && ldrcnt<MAX_LEADERS)
	{
	ldrflag[ldrcnt]=0;
	ldr[ldrcnt++]=ch->master;
	}
	}
	if(ch->leader)
	{
	for(i=0;i<ldrcnt;i++)
	if(ch->leader==ldr[i])
	break;
	if(i==ldrcnt && ldrcnt<MAX_LEADERS)
	{
	ldrflag[ldrcnt]=0;
	ldr[ldrcnt++]=ch->leader;
	}
	}
	}
	// 2) flag all ldr entries which are valid
	for ( ch = first_char; ch; ch=ch->next )
	{
	for(i=0;i<ldrcnt;i++)
	{
	if(ch==ldr[i])
	{
	ldrflag[i]=( IS_NPC(ch) ? 1 : 3);
	break;         
	}
	}
	}
	// 3) finally, scan through all chars and complain if leader/master is bad
	for ( ch = first_char; ch; ch=ch->next )
	{
	if(ch->master)
	{
	for(i=0;i<ldrcnt;i++)
	{
	if(ldr[i]==ch->master)
	{
	if(ldrflag[i]==0)
	{
	sprintf(buf,"%s is following something unknown -- fixed.",NAME(ch));
	bug(buf,0);
	ch->master=NULL;
	}
	else
	break;
	}
	}
	}
	if(ch->leader)
	{
	for(i=0;i<ldrcnt;i++)
	{
	if(ldr[i]==ch->leader)
	{
		if(ldrflag[i]==0)
		{
			sprintf(buf,"%s is in some unknown group -- fixed.",NAME(ch));
			bug(buf,0);
			ch->LeaderId = 0;
		}
		else
			break;
	}  
}
}
}
// end of Testaur's follow/group check
- follow/group check removed by Ksilyan
*/

for ( ch = last_char; ch; ch = gch_prev )
{
	if ( ch == first_char && ch->prev )
	{
		bug( "char_update: first_char->prev != NULL... fixed", 0 );
		ch->prev = NULL;
	}
	gch_prev = ch->prev;
	set_cur_char( ch );
	if ( gch_prev && gch_prev->next != ch )
	{
		bug( "char_update: ch->prev->next != ch", 0 );
		return;
	}

	/*
	 *  Do a room_prog rand check right off the bat
	 *   if ch disappears (rprog might wax npc's), continue
	 */
	/*if(!IS_NPC(ch))
	  rprog_random_trigger( ch );
	 */
	if( char_died(ch) )
		continue;

	if(IS_NPC(ch))
		mprog_time_trigger(ch);   

	if( char_died(ch) )
		continue;

	rprog_time_trigger(ch);

	if( char_died(ch) )
		continue;

	/*
	 * See if player should be auto-saved.
	 */
	if ( !IS_NPC(ch)
			&& ( !ch->GetConnection() || ch->GetConnection()->ConnectedState == CON_PLAYING )
			&&    ch->level >= 1
			&&    secCurrentTime - ch->secSaveTime > (sysdata.save_frequency*60) )
		ch_save			= ch;
	else
		ch_save			= NULL;

	if ( ch->hit  < ch->max_hit)
		ch->hit  += hit_gain(ch);

	if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);

	if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);

	//       if ( ch->position <= POS_STUNNED )
	//				 ch->hit = 1;
	update_pos( ch );

	switch( ch->position )
	{
		case POS_MORTAL:
			act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
					ch, NULL, NULL, TO_ROOM );
			set_char_color(AT_DANGER, ch);
			send_to_char("You are mortally wounded, and will die soon if not aided.\r\n", ch);
			break;

		case POS_INCAP:
			act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
					ch, NULL, NULL, TO_ROOM );
			set_char_color(AT_DANGER, ch);
			send_to_char("You are incapacitated and will slowly die, if not aided.\r\n", ch);
			break;

		case POS_STUNNED:
			if ( !IS_AFFECTED( ch, AFF_PARALYSIS ) )
			{
				act( AT_ACTION, "$n is stunned, but will probably recover.",
						ch, NULL, NULL, TO_ROOM );
				set_char_color(AT_HURT, ch);
				send_to_char("You are stunned, but will probably recover.\r\n", ch);
			}
			break;

		case POS_DEAD:
			act( AT_DEAD, "$n is DEAD!", ch, 0, 0, TO_ROOM );
			set_char_color(AT_DEAD, ch);
			send_to_char("You have been KILLED!\r\n", ch);
			set_cur_char(ch);
			raw_kill( ch, ch );
			continue;
	}


	++ch->timer;    

	if ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_AFK)
			&& ch->GetConnection() && ch->GetConnection()->GetIdleSeconds() > 300  )
	{
		act(AT_ACTION, "$n starts to doze off.", ch, NULL, NULL, TO_ROOM);
		send_to_char("Idle for a long time -- afking you.\n", ch);
		SET_BIT(ch->act, PLR_AFK);
	}


	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
		OBJ_DATA *obj;

		if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
				&&   obj->item_type == ITEM_LIGHT
				&&   obj->value[2] > 0 )
		{
			if ( --obj->value[2] == 0 && ch->GetInRoom() )
			{
				/* ch->GetInRoom()->light -= obj->count; Change by Ksilyan */
				ch->GetInRoom()->light--;
				act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
				if ( obj->serial == cur_obj )
					global_objcode = rOBJ_EXPIRED;
				extract_obj( obj, TRUE );
			}
		}

		if ( ch->timer >= 12 )
		{
			if ( !ch->GetWasInRoom() && ch->GetInRoom() )
			{
				ch->WasInRoomId = ch->GetInRoom()->GetId();
				if ( ch->IsFighting() )
					ch->StopAllFights();//stop_fighting( ch, TRUE );
				act( AT_ACTION, "$n disappears into the void.",
						ch, NULL, NULL, TO_ROOM );
				send_to_char( "You disappear into the void.\n\r", ch );
				if ( IS_SET( sysdata.save_flags, SV_IDLE ) )
					save_char_obj( ch );
				char_from_room( ch );
				char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
			}
		}

		if ( ch->pcdata->condition[COND_DRUNK] > 8 )
			worsen_mental_state( ch, ch->pcdata->condition[COND_DRUNK]/8 );
		if ( ch->pcdata->condition[COND_FULL] > 1 || ch->Class == CLASS_VAMPIRE)
		{
			switch( ch->position )
			{
				case POS_SLEEPING:  better_mental_state( ch, 4 );		break;
				case POS_RESTING:   better_mental_state( ch, 3 );		break;
				case POS_SITTING:
				case POS_MOUNTED:   better_mental_state( ch, 2 );		break;
				case POS_STANDING:  better_mental_state( ch, 1 );		break;
				case POS_FIGHTING:
									if ( number_bits(2) == 0 )
										better_mental_state( ch, 1 );
									break;
			}
		}
		if ( ch->pcdata->condition[COND_THIRST] > 1 || ch->Class == CLASS_VAMPIRE)
		{
			switch( ch->position )
			{
				case POS_SLEEPING:  better_mental_state( ch, 5 );		break;
				case POS_RESTING:   better_mental_state( ch, 3 );		break;
				case POS_SITTING:
				case POS_MOUNTED:   better_mental_state( ch, 2 );		break;
				case POS_STANDING:  better_mental_state( ch, 1 );		break;
				case POS_FIGHTING:
									if ( number_bits(2) == 0 )
										better_mental_state( ch, 1 );
									break;
			}
		}

		if ( ch->Class == CLASS_VAMPIRE && ch->level >= 10 )
		{
			if ( time_info.hour < 21 && time_info.hour > 5 )
				gain_condition( ch, COND_BLOODTHIRST, -1 );
		}
		else {
			//gain_forage(ch);
			gain_condition(ch, COND_FULL, -1);
			gain_condition( ch, COND_DRUNK,  -1 );
		}
		if ( ch->GetInRoom() && ch->Class != CLASS_VAMPIRE)
			switch( ch->GetInRoom()->sector_type )
			{
				default:
					gain_condition( ch, COND_THIRST, -1 );  break;
				case SECT_DESERT:
					gain_condition( ch, COND_THIRST, -2 );  break;
				case SECT_UNDERWATER:
				case SECT_OCEANFLOOR:
					if ( number_bits(1) == 0 )
						gain_condition( ch, COND_THIRST, -1 );  break;
			}

	}

	if ( !char_died(ch) )
	{
		/*
		 * Careful with the damages here,
		 *   MUST NOT refer to ch after damage taken,
		 *   as it may be lethal damage (on NPC).
		 */
		if ( IS_AFFECTED(ch, AFF_POISON) )
		{
			act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
			act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
			ch->mental_state = URANGE( 20, ch->mental_state
					+ (3), 100 );
			damage( ch, ch, 6, gsn_poison , 0);
		}
		else
			if ( ch->position == POS_INCAP )
				damage( ch, ch, 1, TYPE_UNDEFINED ,0);
			else
				if ( ch->position == POS_MORTAL )
					damage( ch, ch, 4, TYPE_UNDEFINED , 0);
		if ( char_died(ch) )
			continue;
		if ( ch->mental_state >= 30 )
			switch( (ch->mental_state+5) / 10 )
			{
				case  3:
					send_to_char( "You feel feverish.\n\r", ch );
					act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
					break;
				case  4:
					send_to_char( "You do not feel well at all.\n\r", ch );
					act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
					break;
				case  5:
					send_to_char( "You need help!\n\r", ch );
					act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
					break;
				case  6:
					send_to_char( "Seekest thou a cleric.\n\r", ch );
					act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
					break;
				case  7:
					send_to_char( "You feel reality slipping away...\n\r", ch );
					act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
					break;
				case  8:
					send_to_char( "You begin to understand... everything.\n\r", ch );
					act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
					break;
				case  9:
					send_to_char( "You are ONE with the universe.\n\r", ch );
					act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL, TO_ROOM );
					break;
				case 10:
					send_to_char( "You feel the end is near.\n\r", ch );
					act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
					break;
			}
		if ( ch->mental_state <= -30 )
			switch( (abs(ch->mental_state)+5) / 10 )
			{
				case  10:
					if ( ch->position > POS_SLEEPING )
					{
						if ( (ch->position == POS_STANDING
									||    ch->position < POS_FIGHTING)
								&&    number_percent()+10 < abs(ch->mental_state) )
							do_sleep( ch, "" );
						else
							send_to_char( "You're barely conscious.\n\r", ch );
					}
					break;
				case   9:
					if ( ch->position > POS_SLEEPING )
					{
						if ( (ch->position == POS_STANDING
									||    ch->position < POS_FIGHTING)
								&&   (number_percent()+20) < abs(ch->mental_state) )
							do_sleep( ch, "" );
						else
							send_to_char( "You can barely keep your eyes open.\n\r", ch );
					}
					break;
				case   8:
					if ( ch->position > POS_SLEEPING )
					{
						if ( ch->position < POS_SITTING
								&&  (number_percent()+30) < abs(ch->mental_state) )
							do_sleep( ch, "" );
						else
							send_to_char( "You're extremely drowsy.\n\r", ch );
					}
					break;
				case   7:
					if ( ch->position > POS_RESTING )
						send_to_char( "You feel very unmotivated.\n\r", ch );
					break;
				case   6:
					if ( ch->position > POS_RESTING )
						send_to_char( "You feel sedated.\n\r", ch );
					break;
				case   5:
					if ( ch->position > POS_RESTING )
						send_to_char( "You feel sleepy.\n\r", ch );
					break;
				case   4:
					if ( ch->position > POS_RESTING )
						send_to_char( "You feel tired.\n\r", ch );
					break;
				case   3:
					if ( ch->position > POS_RESTING )
						send_to_char( "You could use a rest.\n\r", ch );
					break;
			}
		if ( ch->timer > 24 && !IS_IMMORTAL(ch) )
			do_quit( ch, "" );
		else
			if ( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO )
					&&   ++save_count < 10 )		/* save max of 10 per tick */
				save_char_obj( ch );
	}
}

return;
}



void expire_object(Object * obj)
{
	short AT_TEMP = AT_PLAIN;
	const char *message;
	Character *rch;

	switch ( obj->item_type )
	{
		default:
			message = "$p mysteriously vanishes.";
			AT_TEMP = AT_PLAIN;
			break;
		case ITEM_PORTAL:
			message = "$p winks out of existence.";
			remove_portal(obj);
			obj->item_type = ITEM_TRASH;					/* so extract_obj				 */
			AT_TEMP = AT_MAGIC;									/* doesn't remove_portal */
			break;
		case ITEM_FOUNTAIN:
			message = "$p dries up.";
			AT_TEMP = AT_BLUE;
			break;
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			message = "$p decays into dust and blows away.";
			AT_TEMP = AT_OBJECT;
			break;
			//AT_TEMP = AT_MAGIC;
			//break;
		case ITEM_FOOD:
			message = "$p is devoured by a swarm of maggots.";
			AT_TEMP = AT_HUNGRY;
			break;
		case ITEM_BLOOD:
			message = "$p slowly seeps into the ground.";
			AT_TEMP = AT_BLOOD;
			break;
		case ITEM_BLOODSTAIN:
			message = "$p dries up into flakes and blows away.";
			AT_TEMP = AT_BLOOD;
			break;
		case ITEM_SCRAPS:
			message = "$p crumbles and decays into nothing.";
			AT_TEMP = AT_OBJECT;
			break;
		case ITEM_FIRE:
			/* Ksilyan - removed because this is handled in extract_obj -> obj_from_room */
			/*if (obj->GetInRoom())
			  --obj->GetInRoom()->light;*/
			message = "$p burns out.";
			AT_TEMP = AT_FIRE;
	}

	if ( obj->GetCarriedBy() )
	{
		act( AT_TEMP, message, obj->GetCarriedBy(), obj, NULL, TO_CHAR );
	}
	else if ( obj->GetInRoom()
			  && ( rch = obj->GetInRoom()->first_person ) != NULL
			  && !IS_OBJ_STAT( obj, ITEM_BURIED ) )
	{
		act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
		act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
	}

	if ( obj->serial == cur_obj )
		global_objcode = rOBJ_EXPIRED;

	// Remove contents to the ground, unless it's an NPC corpse
	if (obj->item_type != ITEM_CORPSE_NPC)
	{
		Object * pObj;
		while ( (pObj = obj->first_content) )
		{
			obj_from_obj(pObj);

			if ( obj->GetCarriedBy() ) {
				obj_to_char(pObj, obj->GetCarriedBy());
			}
			else if ( obj->GetInRoom() ) {
				if ( IS_OBJ_STAT(obj, ITEM_BURIED) ) {
					SET_BIT(pObj->extra_flags, ITEM_BURIED);
				}
				obj_to_room(pObj, obj->GetInRoom());  
			}
			else if ( obj->GetInObj() ) {
				obj_to_obj(pObj, obj->GetInObj());
			}
		}
	}

	// Remove the object
	extract_obj( obj, TRUE );
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
	OBJ_DATA *obj;

	for ( obj = last_object; obj; obj = gobj_prev )
	{
		if ( obj == first_object && obj->prev )
		{
			bug( "obj_update: first_object->prev != NULL... fixed", 0 );
			obj->prev = NULL;
		}
		gobj_prev = obj->prev;
		if ( gobj_prev && gobj_prev->next != obj )
		{
			bug( "obj_update: obj->prev->next != obj", 0 );
			return;
		}
		set_cur_obj( obj );
		if ( obj->GetCarriedBy() )
			oprog_random_trigger( obj ); 
		else
			if( obj->GetInRoom() && obj->GetInRoom()->area->nplayer > 0 )
				oprog_random_trigger( obj ); 

		if( obj_extracted(obj) )
			continue;

		if ( obj->item_type == ITEM_PIPE )
		{
			if ( IS_SET( obj->value[3], PIPE_LIT ) )
			{
				if ( --obj->value[1] <= 0 )
				{
					obj->value[1] = 0;
					REMOVE_BIT( obj->value[3], PIPE_LIT );
				}
				else if ( IS_SET( obj->value[3], PIPE_HOT ) )
					REMOVE_BIT( obj->value[3], PIPE_HOT );
				else
				{
					if ( IS_SET( obj->value[3], PIPE_GOINGOUT ) )
					{
						REMOVE_BIT( obj->value[3], PIPE_LIT );
						REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
					}
					else
						SET_BIT( obj->value[3], PIPE_GOINGOUT );
				}

				if ( !IS_SET( obj->value[3], PIPE_LIT ) )
					SET_BIT( obj->value[3], PIPE_FULLOFASH );
			}
			else
				REMOVE_BIT( obj->value[3], PIPE_HOT );
		}


		/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

		if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
		{
			bool beheaded = FALSE;
			sh_int timerfrac = UMAX(1, obj->timer - 1);
			if ( obj->item_type == ITEM_CORPSE_PC )
				timerfrac = (int)(obj->timer / 8 + 1);

			if ( obj->timer > 0 && obj->value[2] > timerfrac )
			{
				char buf[MAX_STRING_LENGTH];
				char name[MAX_STRING_LENGTH];
				char *bufptr;
				bufptr = one_argument( obj->shortDesc_.c_str(), name ); 
				bufptr = one_argument( bufptr, name ); 

				if ( is_name("beheaded", obj->name_.c_str()) ){
					beheaded = TRUE;
				}

				bufptr = one_argument( bufptr, name ); 

				separate_obj(obj);
				obj->value[2] = timerfrac; 

				if ( beheaded ) {
					sprintf(buf, behead_corpse_descs[UMIN(timerfrac -1, 4) ],
							capitalize(strip_the(bufptr)));
				} else {
					sprintf( buf, corpse_descs[ UMIN( timerfrac - 1, 4 ) ], 
							 capitalize( bufptr ) ); 
				}

				obj->longDesc_ = buf; 
			}  
		}


		// Check to see if the object's timer has expired. Don't
		// let inventory decay.
		if ( !IS_OBJ_STAT(obj, ITEM_INVENTORY) && obj->timer > 0 && --obj->timer == 0 )
			expire_object(obj);

	}
	return;
}


/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check( void )
{
	CHAR_DATA *ch, *ch_next;
	OBJ_DATA *obj;
	ExitData *pexit;
	static int cnt = 0;
	int door, retcode;

	cnt = (cnt+1) % 2;

	for ( ch = first_char; ch; ch = ch_next )
	{
		set_cur_char(ch);
		ch_next = ch->next;
		will_fall(ch, 0);

		if ( char_died( ch ))
			continue;

		if ( ch->position == POS_MEDITATING ) {
			if ( number_percent() <= ch->pcdata->learned[gsn_meditate] ) {
				ch->mana += mana_gain(ch)/4;
			}
		}

		if ( IS_NPC( ch ) )
		{
			if ( cnt != 0 )
				continue;

			/* running mobs			-Thoric */
			if ( IS_SET(ch->act, ACT_RUNNING) )
			{
				if ( ch->vnum_destination || ch->GetDestinationChar() )  
				{  
					auto_walk(ch);  
					continue;  
				}

				if ( !IS_SET( ch->act, ACT_SENTINEL )
						&&   !ch->IsFighting() && ch->hunting )
				{
					ch->AddWait( 2 * PULSE_VIOLENCE );
					hunt_victim( ch );
					continue;
				}

				if ( ch->spec_fun )
				{
					if ( (*ch->spec_fun) ( ch ) )
						continue;
					if ( char_died(ch) )
						continue;
				}

				if ( !IS_SET(ch->act, ACT_SENTINEL)
						&&   !IS_SET(ch->act, ACT_PROTOTYPE)
						&& ( door = number_bits( 4 ) ) <= 9
						&& ( pexit = get_exit(ch->GetInRoom(), door) ) != NULL
						&&   pexit->to_room
						&&   !IS_SET(pexit->exit_info, EX_CLOSED)
						&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
						&&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)
						&& ( !IS_SET(ch->act, ACT_STAY_AREA)
							||   pexit->to_room->area == ch->GetInRoom()->area ) )
				{
					retcode = move_char( ch, pexit, 0 );
					if ( char_died(ch) )
						continue;
					if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
							||    ch->position < POS_STANDING )
						continue;
				}
			}
			continue;
		}
		else
		{ /* ch is PC */
			
			if ( ch->GetMount()
					&&   ch->GetInRoom() != ch->GetMount()->GetInRoom() )
			{
				REMOVE_BIT( ch->GetMount()->act, ACT_MOUNTED );
				ch->MountId = 0;
				ch->position = POS_STANDING;

				send_to_char( "No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r", ch );
			}

			if ( ( ( ch->GetInRoom() && ch->GetInRoom()->sector_type == SECT_UNDERWATER )
						||( ch->GetInRoom() && ch->GetInRoom()->sector_type == SECT_OCEANFLOOR))
					&& ch->Class != CLASS_VAMPIRE)
			{
				if ( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
				{
					if ( ch->level < LEVEL_IMMORTAL )
					{
						int dam;

						/* Changed level of damage at Brittany's request. -- Narn */	
						if( number_bits(3) == 0)
						{
							dam = number_range( ch->max_hit / 100, ch->max_hit / 50);
							dam = UMAX( 1, dam );
							send_to_char( "You cough and choke as you try to breathe water!\n\r", ch );
							damage( ch, ch, dam, TYPE_UNDEFINED, 0 );
						}
					}
				}
			}

			if (ch->Class == CLASS_VAMPIRE
					&& ch->level < LEVEL_IMMORTAL
					&& (weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT )
					&& ch->isOutside())
			{
				if( number_bits(3) == 0 )
				{
					int dam;

					dam = number_range( ch->max_hit/100, ch->max_hit/60 );
					if (dam < 1)
						dam = 1; //dam = UMAX(2,dam); // what's up with umax?

					if( number_bits(1)==0)
						send_to_char( "You scream in pain as your flesh burns in the sunlight!\r\n", ch);
					else
						send_to_char( "You watch in horror as smoke pours from your shrivelling hands!\r\n", ch);
					damage( ch, ch, dam, TYPE_UNDEFINED,0 );
				}
			}

			if ( char_died( ch ) )
				continue; 

			if ( ch->GetInRoom()
					&& (( ch->GetInRoom()->sector_type == SECT_WATER_NOSWIM )
						||  ( ch->GetInRoom()->sector_type == SECT_WATER_SWIM ) ) )
			{
				if ( !IS_AFFECTED( ch, AFF_FLYING )
						&& !IS_AFFECTED( ch, AFF_FLOATING ) 
						&& !IS_AFFECTED( ch, AFF_AQUA_BREATH )
						&& !ch->GetMount() )
				{
					for ( obj = ch->first_carrying; obj; obj = obj->next_content )
						if ( obj->item_type == ITEM_BOAT )
							break;

					if ( !obj )
					{
						if ( ch->level < LEVEL_IMMORTAL )
						{
							int mov;
							int dam;

							if ( ch->move > 0 )
							{
								mov = number_range( ch->max_move / 20, ch->max_move / 5 );
								mov = UMAX( 1, mov );

								if ( ch->move - mov < 0 )
									ch->move = 0;
								else
									ch->move -= mov;
							}
							else
							{
								if( number_bits(3) == 0 )
								{
									dam = number_range( ch->max_hit / 20, ch->max_hit / 5 );
									dam = UMAX( 1, dam );
									send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\n\r", ch );
									damage( ch, ch, dam, TYPE_UNDEFINED,0 );
								}
							}
						}
					}
				}
			}

			/* beat up on link dead players */
			if ( !ch->GetConnection() )
			{
				CHAR_DATA *wch, *wch_next;

				for ( wch = ch->GetInRoom()->first_person; wch; wch = wch_next )
				{
					wch_next = wch->next_in_room;

					if (!IS_NPC(wch)
							||   wch->IsFighting()
							||   IS_AFFECTED(wch, AFF_CHARM)
							||   !IS_AWAKE(wch)
							|| ( IS_SET(wch->act, ACT_WIMPY) && IS_AWAKE(ch) )
							||   !can_see( wch, ch ) )
						continue;

					if ( is_hating( wch, ch ) )
					{
						found_prey( wch, ch );
						continue;
					}

					if ( !IS_SET(wch->act, ACT_AGGRESSIVE)
							||    IS_SET(wch->act, ACT_MOUNTED)
							||    IS_SET(wch->GetInRoom()->room_flags, ROOM_SAFE ) )
						continue;
					global_retcode = multi_hit( wch, ch, TYPE_UNDEFINED );
				}
			}
		}
	}
}


/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
 void aggr_update( void )
{
	CHAR_DATA *wch;
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	CHAR_DATA *victim;
	struct act_prog_data *apdtmp;

#ifdef UNDEFD
	/*
	 *	GRUNT!	To do
	 *
	 */
	if ( IS_NPC( wch ) && wch->mpactnum > 0
			&& wch->GetInRoom()->area->nplayer > 0 )
	{
		MPROG_ACT_LIST * tmp_act, *tmp2_act;
		for ( tmp_act = wch->mpact; tmp_act;
				tmp_act = tmp_act->next )
		{
			oprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,
					tmp_act->obj, tmp_act->vo, ACT_PROG );
			DISPOSE( tmp_act->buf );
		}
		for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )
		{
			tmp2_act = tmp_act->next;
			DISPOSE( tmp_act );
		}
		wch->mpactnum = 0;
		wch->mpact    = NULL;
	}
#endif

	/* check mobprog act queue */
	while ( (apdtmp = mob_act_list) != NULL )
	{
		wch = (CHAR_DATA *) mob_act_list->vo;
		if ( !char_died(wch) && wch->mpactnum > 0 )
		{
			MPROG_ACT_LIST * tmp_act;

			while ( (tmp_act = wch->mpact) != NULL )
			{
				if ( tmp_act->obj && obj_extracted(tmp_act->obj) )
					tmp_act->obj = NULL;
				if ( tmp_act->ch && !char_died(tmp_act->ch) )
					mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
							tmp_act->obj, tmp_act->vo, ACT_PROG );
				wch->mpact = tmp_act->next;
				DISPOSE(tmp_act->buf);
				DISPOSE(tmp_act);
			}
			wch->mpactnum = 0;
			wch->mpact    = NULL;
		}
		mob_act_list = apdtmp->next;
		DISPOSE( apdtmp );
	}


	/*
	 * Just check descriptors here for victims to aggressive mobs
	 * We can check for linkdead victims to mobile_update				-Thoric
	 */
	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if ( d->ConnectedState != CON_PLAYING || (wch=d->GetCharacter()) == NULL )
			continue;

		if ( char_died(wch)
				||   IS_NPC(wch)
				||   wch->level >= LEVEL_IMMORTAL
				||  !wch->GetInRoom() )
			continue;

		for ( ch = wch->GetInRoom()->first_person; ch; ch = ch_next )
		{
			int count;

			ch_next			= ch->next_in_room;

			if ( !IS_NPC(ch)
					||   ch->IsFighting()
					||   IS_AFFECTED(ch, AFF_CHARM)
					||   !IS_AWAKE(ch)
					||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
					||   !can_see( ch, wch ) )
				continue;

			if ( is_hating( ch, wch ) )
			{
				found_prey( ch, wch );
				continue;
			}

			if ( !IS_SET(ch->act, ACT_AGGRESSIVE)
					||    IS_SET(ch->act, ACT_MOUNTED)
					||    IS_SET(ch->GetInRoom()->room_flags, ROOM_SAFE ) )
				continue;

			/*
			 * Ok we have a 'wch' player character and a 'ch' npc aggressor.
			 * Now make the aggressor fight a RANDOM pc victim in the room,
			 *	 giving each 'vch' an equal chance of selection.
			 */
			count			= 0;
			victim 		= NULL;
			for ( vch = wch->GetInRoom()->first_person; vch; vch = vch_next )
			{
				vch_next = vch->next_in_room;

				if ( !IS_NPC(vch)
						&&   vch->level < LEVEL_IMMORTAL
						&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
						&&   can_see( ch, vch ) )
				{
					if ( number_range( 0, count ) == 0 )
						victim = vch;
					count++;
				}
			}

			if ( !victim )
			{
				bug( "Aggr_update: null victim.", count );
				continue;
			}

			if ( IS_NPC(ch) && IS_SET(ch->attacks, ATCK_BACKSTAB ) )
			{
				OBJ_DATA *obj;

				if ( !ch->GetMount()
						&& (obj = get_eq_char( ch, WEAR_WIELD )) != NULL
						&& obj->value[3] == 11
						&& !victim->IsFighting()
						&& victim->hit >= victim->max_hit / 2
						//&& victim->hit >= victim->max_hit
				   )
				{
					ch->AddWait( skill_table[gsn_backstab]->beats );
					if ( !IS_AWAKE(victim)
							||   number_percent( )+5 < ch->level )
					{
						global_retcode = multi_hit( ch, victim, gsn_backstab );
						continue;
					}
					else
					{
						global_retcode = damage( ch, victim, 0, gsn_backstab ,0);
						continue;
					}
				}
			}
			global_retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
		}
	}

	return;
}

/* From interp.c */
bool check_social   ( CHAR_DATA *ch, const char *command, const char *argument ) ;

/*
 * drunk randoms				- Tricops
 * (Made part of mobile_update  -Thoric)
 */
void drunk_randoms( CHAR_DATA *ch )
{
	CHAR_DATA *rvch = NULL;
	CHAR_DATA *vch;
	sh_int drunk;
	sh_int position;

	if ( IS_NPC( ch ) || ch->pcdata->condition[COND_DRUNK] <= 0 )
		return;

	if ( number_percent() < 30 )
		return;

	drunk = ch->pcdata->condition[COND_DRUNK];
	position = ch->position;
	ch->position = POS_STANDING;

	if ( number_percent() < (2*drunk / 20) )
		check_social( ch, "burp", "" );
	else if ( number_percent() < (2*drunk / 20) )
		check_social( ch, "hiccup", "" );
	else if ( number_percent() < (2*drunk / 20) )
		check_social( ch, "drool", "" );
	else if ( number_percent() < (2*drunk / 20) )
		check_social( ch, "fart", "" );
	else if ( drunk > (10+(ch->getCon()/5))
			&&   number_percent() < ( 2 * drunk / 18 ) )
	{
		for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next_in_room )
		{
			if ( number_percent() < 10 )
				rvch = vch;
		}
		check_social( ch, "puke", (rvch ? rvch->getName().c_str() : (char *) "") );
	}

	ch->position = position;
	return;
}

void halucinations( CHAR_DATA *ch )
{
    if ( ch->mental_state >= 30 && number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) == 0 )
    {
				const char *t;

				switch( number_range( 1, UMIN(20, (ch->mental_state+5) / 5)) )
				{
						default:
						case  1: t = "You feel very restless... you can't sit still.\n\r";						break;
						case  2: t = "You're tingling all over.\n\r";															break;
						case  3: t = "Your skin is crawling.\n\r";																		break;
						case  4: t = "You suddenly feel that something is terribly wrong.\n\r";			break;
						case  5: t = "Those damn little fairies keep laughing at you!\n\r";						break;
						case  6: t = "A bit of autumn mist floats by...\n\r";											break;
						case  7: t = "Have you been here before, or not?  You're not sure...\n\r";		break;
						case  8: t = "Painful childhood memories flash through your mind.\n\r";			break;
						case  9: t = "You hear someone call your name in the distance...\n\r";			break;
						case 10: t = "Your head is pulsating... you can't think straight.\n\r";			break;
						case 11: t = "The ground... seems to be squirming...\n\r";										break;
						case 12: t = "You're not quite sure what is real anymore.\n\r";							break;
						case 13: t = "It's all a dream... or is it?\n\r";														break;
						case 14: t = "They're coming to get you... coming to take you away...\n\r";		break;
						case 15: t = "You begin to feel all powerful!\n\r";														break;
						case 16: t = "You're light as air... the heavens are yours for the taking.\n\r";		break;
						case 17: t = "Your whole life flashes by... and your future...\n\r";			break;
						case 18: t = "You are everywhere and everything... you know all and are all!\n\r";		break;
						case 19: t = "You feel immortal!\n\r";																			break;
						case 20: t = "Ahh... the power of a Supreme Entity... what to do...\n\r";		break;
				}
				send_to_char( t, ch );
    }
    return;
}

void tele_update( void )
{
    TELEPORT_DATA *tele, *tele_next;

    if ( !first_teleport )
      return;
    
    for ( tele = first_teleport; tele; tele = tele_next )
    {
				tele_next = tele->next;
				if ( --tele->timer <= 0 )
				{
						if ( tele->room->first_person )
						{
								if ( IS_SET( tele->room->room_flags, ROOM_TELESHOWDESC ) )
								  teleport( tele->room->first_person, tele->room->tele_vnum,
												TELE_SHOWDESC | TELE_TRANSALL );
								else
								  teleport( tele->room->first_person, tele->room->tele_vnum,
												TELE_TRANSALL );
						}
						UNLINK( tele, first_teleport, last_teleport, next, prev );
						DISPOSE( tele );
				}
    }
}

#if 0
/* 
 * Write all outstanding authorization requests to Log channel - Gorog
 */ 
void auth_update( void ) 
{ 
  CHAR_DATA *victim; 
  DESCRIPTOR_DATA *d; 
  char log_buf [MAX_INPUT_LENGTH];
  bool first_time = TRUE;         /* so titles are only done once */

  for ( d = first_descriptor; d; d = d->next ) 
      {
      victim = d->character;
      if ( victim && IS_WAITING_FOR_AUTH(victim) )
         {
         if ( first_time )
            {
            first_time = FALSE;
            strcpy (log_buf, "Pending authorizations:" ); 
            log_string( log_buf ); 
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1);
            }
         sprintf( log_buf, " %s@%s new %s %s", victim->name,
            victim->desc->host, race_table[victim->race].race_name, 
            class_table[victim->Class]->who_name ); 
         log_string( log_buf ); 
         to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1);
         }
      }
} 
#endif

void auth_update( void ) 
{ 
	CHAR_DATA *victim; 
	
	char buf [MAX_INPUT_LENGTH], log_buf [MAX_INPUT_LENGTH];
	bool found_hit = FALSE; 		/* was at least one found? */
	
	strcpy (log_buf, "Pending authorizations:\n\r" );
	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if ( (victim = d->GetCharacter()) && IS_WAITING_FOR_AUTH(victim) )
		{
			found_hit = TRUE;
			sprintf( buf, " %s@%s new %s %s\n\r", victim->getName().c_str(),
				victim->GetConnection()->GetHost(), race_table[victim->race].race_name, 
				class_table[victim->Class]->whoName_.c_str() );
			strcat (log_buf, buf);
		}
	}
	if (found_hit)
	{
		log_string( log_buf ); 
		to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1);
	}
} 

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int			pulse_second;
    static  int     pulse_asave = PULSE_ASAVE;
    
    struct timeval stime;
    struct timeval etime;

	extern bool newbielock;

    if ( timechar )
    {
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Starting update timer.\n\r", timechar );
      gettimeofday(&stime, NULL);
    }
   
    if ( --pulse_asave    <= 0 )
    {
        AREA_DATA *tarea;
        pulse_asave = PULSE_ASAVE;

        for ( tarea = first_build; tarea; tarea = tarea->next )
        {
            char buf[MAX_STRING_LENGTH];

            if ( !IS_SET(tarea->status, AREA_LOADED) )
                continue;
   
            sprintf(buf, "%s%s", BUILD_DIR, tarea->filename);
            fold_area(tarea, buf, FALSE);
        }
    }
    
    if ( --pulse_area     <= 0 )
    {
				pulse_area				= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
				area_update				( );
    }

    if ( --pulse_mobile   <= 0 )
    {
				pulse_mobile	= PULSE_MOBILE;
				mobile_update  ( );
    }

    if ( --pulse_violence <= 0 )
    {
				pulse_violence  = PULSE_VIOLENCE;
				violence_update ( );
    }

    if ( --pulse_point    <= 0 )
    {
				pulse_point     = number_range( (int) (PULSE_TICK * 0.75), (int) (PULSE_TICK * 1.25) );

		        auth_update     ( );									/* Gorog */
				weather_update  ( );
				char_update				( );
				rprog_update    ( );
				obj_update				( );
				clear_vrooms	( );									/* remove virtual rooms */
				if ( newbielock )
				{
					log_string("NOTICE: MUD is currently newbielocked.");
				}
    }

    if ( --pulse_second   <= 0 )
    {
				pulse_second	= PULSE_PER_SECOND;
				char_check( );
				/*reboot_check( "" ); Disabled to check if its lagging a lot - Scryn*/
				/* Much faster version enabled by Altrag..
				   although I dunno how it could lag too much, it was just a bunch
				   of comparisons.. */
				reboot_check(0);
    }

    if ( auction->item && --auction->pulse <= 0 )
    {                                                  
				auction->pulse = PULSE_AUCTION;                     
				auction_update( );
    }

    tele_update( );
    aggr_update( );
    obj_act_update ( );
    room_act_update( );
    clean_obj_queue();					/* dispose of extracted objects */
    clean_char_queue();					/* dispose of dead mobs/quitting chars */

	
    if ( timechar )
    {
      gettimeofday(&etime, NULL);
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Update timing complete.\n\r", timechar );
      subtract_times(&etime, &stime);
      ch_printf( timechar, "Timing took %d.%06d seconds.\n\r",
          etime.tv_sec, etime.tv_usec );
      timechar = NULL;
    }
    tail_chain( );
    return;
}


void remove_portal( OBJ_DATA *portal )
{
    ROOM_INDEX_DATA *fromRoom, *toRoom;
    CHAR_DATA *ch;
    ExitData *pexit;
    bool found;

    if ( !portal )
    {
				bug( "remove_portal: portal is NULL", 0 );
				return;
    }

    fromRoom = portal->GetInRoom();
    found = FALSE;
    if ( !fromRoom )
    {
				bug( "remove_portal: portal->GetInRoom() is NULL", 0 );
				return;
    }

    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
				if ( IS_SET( pexit->exit_info, EX_PORTAL ) )
				{
						found = TRUE;
						break;
				}

    if ( !found )
    {
				bug( "remove_portal: portal not found in room %s!", vnum_to_dotted(fromRoom->vnum) );
				return;
    }

    if ( pexit->vdir != DIR_PORTAL )
				bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );

    if ( ( toRoom = pexit->to_room ) == NULL )
      bug( "remove_portal: toRoom is NULL", 0 );
 
    extract_exit( fromRoom, pexit );
    /* rendunancy */
    /* send a message to fromRoom */
    /* ch = fromRoom->first_person; */
    /* if(ch!=NULL) */
    /* act( AT_PLAIN, "A magical portal below winks from existence.", ch, NULL, NULL, TO_ROOM ); */

    /* send a message to toRoom */
    if ( toRoom && (ch = toRoom->first_person) != NULL )
      act( AT_PLAIN, "A magical portal above winks from existence.", ch, NULL, NULL, TO_ROOM );

    /* remove the portal obj: looks better to let update_obj do this */
    /* extract_obj(portal);  */

    return;
}

void reboot_check( time_t reset )
{
	static const char *tmsg[] =
	{ "You feel the ground shake as the end comes near!",
		"Lightning crackles in the sky above!",
		"Crashes of thunder sound across the land!",
		"The sky has suddenly turned midnight black.",
		"You notice the life forms around you slowly dwindling away.",
		"The seas across the realm have turned frigid.",
		"The aura of magic that surrounds the realms seems slightly unstable.",
		"You sense a change in the magical forces surrounding you."
	};
	static const int times[] = { 60, 120, 180, 240, 300, 600, 900, 1800 };
	static const int timesize =
		UMIN(sizeof(times)/sizeof(*times), sizeof(tmsg)/sizeof(*tmsg));
	char buf[MAX_STRING_LENGTH];
	static int trun;
	static bool init;

	if ( !init || reset >= secCurrentTime)
	{
		for ( trun = timesize-1; trun >= 0; trun-- )
			if ( reset >= secCurrentTime+times[trun] )
				break;
		init = TRUE;
		return;
	}

	if ( (secCurrentTime % 1800) == 0 )
	{
		sprintf(buf, "%.24s: %d players", ctime(&secCurrentTime), gTheWorld->ConnectionCount() );
		append_to_file(USAGE_FILE, buf);
	}

	if ( secNewBoot_time_t - secBootTime < 60*60*18 &&
			!set_boot_time->manual )
		return;

	if ( secNewBoot_time_t <= secCurrentTime )
	{
		CHAR_DATA *vch;

		if ( auction->item )
		{
			sprintf(buf, "Sale of %s has been stopped by mud.",
					auction->item->shortDesc_.c_str());
			talk_auction(buf);
			obj_to_char(auction->item, auction->seller);
			auction->item = NULL;
			if ( auction->buyer && auction->buyer != auction->seller )
			{
				auction->buyer->gold += auction->bet;
				send_to_char("Your money has been returned.\n\r", auction->buyer);
			}
		}
		echo_to_all(AT_YELLOW, "You are forced from these realms by a strong "
				"magical presence\n\ras life here is reconstructed.", ECHOTAR_ALL);

		for ( vch = first_char; vch; vch = vch->next )
			if ( !IS_NPC(vch) )
				save_char_obj(vch);
		gGameRunning = false;
		return;
	}

	if ( trun != -1 && secNewBoot_time_t - secCurrentTime <= times[trun] )
	{
		echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);
		if ( trun <= 5 )
			sysdata.DENY_NEW_PLAYERS = TRUE;
		--trun;
		return;
	}
	return;
}
  
#if 0
void reboot_check( char *arg )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    /*struct tm *timestruct;
    int timecheck;*/
    CHAR_DATA *vch;

    /*Bools to show which pre-boot echoes we've done. */
    static bool thirty  = FALSE;
    static bool fifteen = FALSE;
    static bool ten     = FALSE;
    static bool five    = FALSE;
    static bool four    = FALSE;
    static bool three   = FALSE;
    static bool two     = FALSE;
    static bool one     = FALSE;

    /* This function can be called by do_setboot when the reboot time
       is being manually set to reset all the bools. */
    if ( !str_cmp( arg, "reset" ) )
    {
      thirty  = FALSE;
      fifteen = FALSE;
      ten     = FALSE;
      five    = FALSE;
      four    = FALSE;
      three   = FALSE;
      two     = FALSE;
      one     = FALSE;
      return;
    }

    /* If the mud has been up less than 18 hours and the boot time 
       wasn't set manually, forget it. */ 
/* Usage monitor */

if ((secCurrentTime % 1800) == 0)
{
  sprintf(buf, "%s: %d players", ctime(&secCurrentTime), num_descriptors);  
  append_to_file(USAGE_FILE, buf);
}

/* Change by Scryn - if mud has not been up 18 hours at boot time - still 
 * allow for warnings even if not up 18 hours 
 */
    if ( secNewBoot_time_t - secBootTime < 60*60*18 
         && set_boot_time->manual == 0 )
    {
      return;
    }
/*
    timestruct = localtime( &secCurrentTime);

    if ( timestruct->tm_hour == set_boot_time->hour        
         && timestruct->tm_min  == set_boot_time->min )*/
    if ( new_boot_time_t <= secCurrentTime)
    {
       /* Return auction item to seller */
       if (auction->item != NULL)
       {
        sprintf (buf,"Sale of %s has been stopped by mud.",
                 auction->item->short_descr);
        talk_auction (buf);
        obj_to_char (auction->item, auction->seller);
        auction->item = NULL;
        if (auction->buyer != NULL && auction->seller != auction->buyer) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("Your money has been returned.\n\r",auction->buyer);
        }
       }      

       sprintf( buf, "You are forced from these realms by a strong magical presence" ); 
       echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
       sprintf( buf, "as life here is reconstructed." );
       echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );

       /* Save all characters before booting. */
       for ( vch = first_char; vch; vch = vch->next )
       {
         if ( !IS_NPC( vch ) )
           save_char_obj( vch );
       }
       mud_down = TRUE;
    }

  /* How many minutes to the scheduled boot? */
/*  timecheck = ( set_boot_time->hour * 60 + set_boot_time->min )
              - ( timestruct->tm_hour * 60 + timestruct->tm_min );

  if ( timecheck > 30  || timecheck < 0 ) return;

  if ( timecheck <= 1 ) */
  if ( new_boot_time_t - secCurrentTime <= 60 )
  {
    if ( one == FALSE )
    {
				sprintf( buf, "You feel the ground shake as the end comes near!" );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				one = TRUE;
				sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 2 )*/
  if ( new_boot_time_t - secCurrentTime <= 120 )
  {
    if ( two == FALSE )
    {
				sprintf( buf, "Lightning crackles in the sky above!" );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				two = TRUE;
				sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 3 )*/ 
  if (new_boot_time_t - secCurrentTIme <= 180 )
  {
    if ( three == FALSE )
    {
				sprintf( buf, "Crashes of thunder sound across the land!" );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				three = TRUE;
				sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 4 )*/
  if( new_boot_time_t - secCurrentTime <= 240 )
  {
    if ( four == FALSE )
    {
				sprintf( buf, "The sky has suddenly turned midnight black." );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				four = TRUE;
				sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 5 )*/
  if( new_boot_time_t - secCurrentTime <= 300 )
  {
    if ( five == FALSE )
    {
				sprintf( buf, "You notice the life forms around you slowly dwindling away." );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				five = TRUE;
				sysdata.DENY_NEW_PLAYERS = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 10 )*/
  if( new_boot_time_t - secCurrentTime <= 600 )
  {
    if ( ten == FALSE )
    {
				sprintf( buf, "The seas across the realm have turned frigid." );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				ten = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 15 )*/
  if( new_boot_time_t - secCurrentTime <= 900 )
  {
    if ( fifteen == FALSE )
    {
				sprintf( buf, "The aura of magic which once surrounded the realms seems slightly unstable." );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				fifteen = TRUE;
    }
    return;   
  }

/*  if ( timecheck == 30 )*/
  if( new_boot_time_t - secCurrentTime <= 1800 )
  { 
    if ( thirty == FALSE )
    {
				sprintf( buf, "You sense a change in the magical forces surrounding you." );
				echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
				thirty = TRUE;
    }
    return;   
  }

  return;
}
#endif

/* the auction update*/

void auction_update (void)
{
	int tax, pay;
	char buf[MAX_STRING_LENGTH];

	switch (++auction->going) {/* increase the going state */
		case 1 : /* going once */
		case 2 : /* going twice */
			if (auction->bet > auction->starting)
				sprintf (buf, "%s: going %s for %d.\n\r", auction->item->shortDesc_.c_str(),
						((auction->going == 1) ? "once" : "twice"), auction->bet);
			else
				sprintf (buf, "%s: going %s (bid not received yet).\n\r", auction->item->shortDesc_.c_str(),
						((auction->going == 1) ? "once" : "twice"));  
			talk_auction (buf);

			if(!char_in_auctionroom(auction->buyer)) {
				if(auction->going == 1)
					send_to_char("Aren't you supposed to be somewhere else?\n\r", auction->buyer);
				else
					send_to_char("You get a terrible feeling that you have forgotten something!\n\r", auction->buyer);
			}
			if(!char_in_auctionroom(auction->seller)) {
				if(auction->going == 1)
					send_to_char("Don't you have some unfinished business somewhere else?\n\r", auction->seller);
				else
					send_to_char("Your business opportynity is lost lest you act immediately!\n\r", auction->seller);
			}
			break;

		case 3 : /* SOLD! */
			if (!auction->buyer && auction->bet) {
				bug( "Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet );
				auction->bet = 0;
			}
			if (auction->bet > 0 && auction->buyer != auction->seller) {
				sprintf (buf, "%s sold to %s for %d.\n\r",
						auction->item->shortDesc_.c_str(),
						auction->buyer->getShort().c_str(),
						auction->bet);
				talk_auction(buf);

				if(char_in_auctionroom(auction->buyer)) {
					if(auction->buyer->carry_weight + get_obj_weight(auction->item) >
							can_carry_w(auction->buyer)) {
						act( AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item, NULL, TO_CHAR );
						act( AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", auction->buyer, auction->item, NULL, TO_ROOM );
						obj_to_room( auction->item, auction->buyer->GetInRoom() );
					}
					else {
						obj_to_char( auction->item, auction->buyer );
						act(AT_ACTION, "The auctioneer smiles at you, and hands you $p.",
								auction->buyer, auction->item, NULL, TO_CHAR);
						act(AT_ACTION, "The auctioneer smiles at $n, and hands $m $p.",
								auction->buyer, auction->item, NULL, TO_ROOM);
					}
					if(char_in_auctionroom(auction->seller)) {
						pay = (int) (auction->bet * 0.9);
						tax = (int) (auction->bet * 0.1);
						boost_economy( auction->seller->GetInRoom()->area, tax );
						auction->seller->gold += pay; /* give him the money, tax 10 % */
						sprintf(buf, "The auctioneer pays you %d gold, charging an auction fee of %d.\n\r", pay, tax);
						send_to_char(buf, auction->seller);
						auction->item = NULL; /* reset item */
						if ( IS_SET( sysdata.save_flags, SV_AUCTION ) ) {
							save_char_obj( auction->buyer );
							save_char_obj( auction->seller );
						}
					}
					else {
						sprintf(buf, "The auctioneer peers around, trying to find %s.\n\r",
								auction->seller->getShort().c_str() );
						talk_auction(buf);
						sprintf(buf, "After a while he announces that the price is lost to the Retired \n\r");
						talk_auction(buf);
						sprintf(buf, "Auctioneers' Fund because the seller is not present to collect the deposit.\n\r");
						talk_auction(buf);
					}
				}
				else {
					// Tell in auctionrooms that the buyer has left
					sprintf(buf, "The auctioneer peers around, trying to find %s.\n\r",
							auction->buyer->getShort().c_str() );
					talk_auction(buf);
					sprintf(buf, "After a while he announces that the cash deposit is lost to the Retired\n\r");
					talk_auction(buf);
					sprintf(buf, "Auctioneers' Fund because the buyer is not present to claim the purchase.\n\r");
					talk_auction(buf);
					send_to_char("OH NO! The auction! A sense of loss descends upon you.\n\r", auction->buyer);

					if(char_in_auctionroom(auction->seller)) {
						sprintf(buf, "The auctioneer tells you that since %s was not found, the auction\n\r",
								auction->buyer->getShort().c_str());
						send_to_char(buf, auction->seller);
						send_to_char("is cancelled and your item returned. Sorry.\n\r", auction->seller);

						if(auction->seller->carry_weight + get_obj_weight(auction->item)
								+ get_obj_weight(auction->item) > can_carry_w(auction->seller)) {
							act( AT_PLAIN, "You drop $p as it is just too much to carry"
									" with everything else you're carrying.", auction->seller,
									auction->item, NULL, TO_CHAR );
							act( AT_PLAIN, "$n drops $p as it is too much extra weight"
									" for $m with everything else.", auction->seller,
									auction->item, NULL, TO_ROOM );
							obj_to_room( auction->item, auction->seller->GetInRoom() );
						}
						else
							obj_to_char (auction->item,auction->seller);
					}
					else {
						sprintf(buf, "The auctioneer curses. \"Dammit! The seller is also missing.\"\n\r");
						talk_auction(buf);
						sprintf(buf, "After a while he shrugs. \"Oh well, I guess I'll just donate this...\"\n\r");
						talk_auction(buf);

						donate_auctioned_item();
						send_to_char("OH NO! The auction! But now it's too late...\n\r", auction->seller);
					}
				}
			}
			else {/* not sold */
				sprintf (buf, "No bids received for %s - object has been removed from auction.\n\r",
						auction->item->shortDesc_.c_str() );
				talk_auction(buf);
				if(char_in_auctionroom(auction->seller)) {
					act (AT_ACTION, "The auctioneer returns $p to you.",
							auction->seller, auction->item, NULL, TO_CHAR);
					act (AT_ACTION, "The auctioneer returns $p to $n.",
							auction->seller,auction->item,NULL,TO_ROOM);

					if(auction->seller->carry_weight + get_obj_weight(auction->item)
							+ get_obj_weight(auction->item) > can_carry_w(auction->seller)) {
						act( AT_PLAIN, "You drop $p as it is just too much to carry"
								" with everything else you're carrying.", auction->seller,
								auction->item, NULL, TO_CHAR );
						act( AT_PLAIN, "$n drops $p as it is too much extra weight"
								" for $m with everything else.", auction->seller,
								auction->item, NULL, TO_ROOM );
						obj_to_room( auction->item, auction->seller->GetInRoom() );
					}
					else
						obj_to_char (auction->item,auction->seller);
				}
				else {
					sprintf(buf, "The auctioneer curses. \"Dammit! The seller is has gone missing on me!\"\n\r");
					talk_auction(buf);
					sprintf(buf, "Auctioneer shrugs. \"Since he doesn't care for this anymore, I guess I'll just donate it...\"\n\r");
					talk_auction(buf);

					donate_auctioned_item();
					send_to_char("OH NO! The auction! But now it's too late...\n\r", auction->seller);
				}

				tax = (int) (auction->item->cost * 0.05);
				boost_economy(auction->seller->GetInRoom()->area, tax);
				sprintf(buf, "The auctioneer charges you an auction fee of %d.\n\r", tax );
				send_to_char(buf, auction->seller);
				if ((auction->seller->gold - tax) < 0)
					auction->seller->gold = 0;
				else
					auction->seller->gold -= tax;
				if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
					save_char_obj( auction->seller );
			} /* else */
			auction->item = NULL; /* clear auction */
	} /* switch */
} /* func */

void subtract_times(struct timeval *etime, struct timeval *stime)
{
  etime->tv_sec -= stime->tv_sec;
  etime->tv_usec -= stime->tv_usec;
  while ( etime->tv_usec < 0 )
  {
    etime->tv_usec += 1000000;
    etime->tv_sec--;
  }
  return;
}

/*struct forage_data {
    int   amt;
    char* short_d;
    char* long_d;
    char* action_d;
    char* ofound;
    char* cfound;
};*/

//struct forage_data forage_find[SECT_MAX] = {
//    {0, NULL, NULL, NULL, NULL},  /* SECT_INSIDE */
//    {0, NULL, NULL, NULL, NULL},  /* SECT_CITY   */
//    {3,
//        "a few roots",
//        "A few roots, recently dug out of the ground, lie here.",
//        "%s crunche$q on some roots.",
//        "$n digs a few roots from the ground.",
//        "You dig a few roots from the ground."
//    },  /* SECT_FIELD  */
//    {5, 
//        "some berries", 
//        "Some berries, still attached to a branch, lie here.",
//        "%s pick$q the berries off of the branch and put$q them in $s mouth.",
//        "$n plucks a few berries from a bush.",
//        "You pluck a few berries from a bush."
//    },  /* SECT_FOREST */
//    {3,
//        "a mushroom",
//        "A mushroom has been dropped here.",
//        "%s plop$q the mushroom in $s mouth.",
//        "$n lifts up a rock and plucks a mushroom from underneath.", 
//        "You lift up a rock and pluck a mushroom from underneath."
//    },  /* SECT_HILLS  */
//    {1, 
//        "a patch of fungi", 
//        "A small patch of something green covers the ground.", 
//        "%s chew$q thoughtfully on the fungi.", 
//        "$n scrapes some green stuff from the rocks.", 
//        "You scrape some green stuff from the rocks."
//    },  /* SECT_MOUNTAIN */
//    {0, NULL, NULL, NULL, NULL, NULL},  /* SECT_WATER_SWIM */
//    {0, NULL, NULL, NULL, NULL, NULL},  /* SECT_WATER_NOSWIM */
//    {0, NULL, NULL, NULL, NULL, NULL},  /* SECT_UNDERWATER */
//    {0, NULL, NULL, NULL, NULL, NULL},  /* AIR */
//    {1, 
//        "a piece of cactus", 
//        "A small green plant pokes its thorns at you.", 
//        "%s pluck$q out a few thorns, and then bite$q at the cactus.", 
//        "$n reaches down and grabs a small thorny plant.", 
//        "You reach down and grab a small thorny plant.  OUCH"
//    },  /* DESERT */
//    {0, NULL, NULL, NULL, NULL, NULL},  /* DUNNO */
//    {0, NULL, NULL, NULL, NULL, NULL},  /* OCEANFLOOR */
//    {0, NULL, NULL, NULL, NULL, NULL},  /* UNDERGROUND */
//    {2, 
//        "a piece of discarded food", 
//        "A small piece of discarded food lies in the middle of the road.", 
//        "%s devour$q the piece of discarded food, and then spit$q out a few bugs.", 
//        "$n dives into the bushes and comes out with an unrecognizable piece of food.", 
//        "You dive into the bushes and come out with a half eaten piece of food."
//    },  /* ROADS */
//};


/*void gain_forage(CHAR_DATA *ch) {
    int sn_forage;
    OBJ_DATA *food;
    sn_forage = skill_lookup("forage");

    if ( sn_forage < 0 || ch->level <= skill_table[sn_forage]->skill_level[ch->Class]
         || ch->position != POS_STANDING || ch->pcdata->learned[sn_forage] <= 0
         || forage_find[ch->GetInRoom()->sector_type].amt <= 0) {
        gain_condition(ch, COND_FULL, -1);
        return;
    }

    if ( number_percent() > (ch->pcdata->learned[sn_forage]) ) {
        gain_condition(ch, COND_FULL, -1);
        return;
    }

    food = create_object(get_obj_index(OBJ_VNUM_MUSHROOM), 0);
    food->value[0] = forage_find[ch->GetInRoom()->sector_type].amt + ch->level/10;
    food->timer    = 15;

    act(AT_ACTION, forage_find[ch->GetInRoom()->sector_type].ofound, ch, NULL, NULL, TO_ROOM);
    act(AT_ACTION, forage_find[ch->GetInRoom()->sector_type].cfound, ch, NULL, NULL, TO_CHAR);
  
    STRFREE(food->name       );
    food->name        = STRALLOC(forage_find[ch->GetInRoom()->sector_type].short_d);
    STRFREE(food->short_descr);
    food->short_descr = STRALLOC(forage_find[ch->GetInRoom()->sector_type].short_d);
    STRFREE(food->description);
    food->description = STRALLOC(forage_find[ch->GetInRoom()->sector_type].long_d);
    STRFREE(food->action_desc);
    food->action_desc = STRALLOC(forage_find[ch->GetInRoom()->sector_type].action_d);
    
    obj_to_room(food, ch->GetInRoom());
    
    learn_from_success(ch, sn_forage);
}*/

int char_in_auctionroom(CHAR_DATA* ch)
{
   AUCTION_ROOM *pTmp;
   
   for(pTmp = first_auctionroom; pTmp; pTmp = pTmp->next) {
      if(ch->GetInRoom() == pTmp->room)
	return TRUE;
   }
   
   return FALSE;
}

void donate_auctioned_item() {
   int donationroom;
   ROOM_INDEX_DATA *pNew, *pOld;
   
   switch(auction->item->item_type) {
    case ITEM_WEAPON:
    case ITEM_SPIKE:
    case ITEM_PROJECTILE:
    case ITEM_QUIVER: donationroom = ROOM_VNUM_DONATION_WEAPONS; break;
    case ITEM_ARMOR: donationroom = ROOM_VNUM_DONATION_ARMOR; break;
    case ITEM_SCROLL:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_POTION:
    case ITEM_SALVE: donationroom = ROOM_VNUM_DONATION_MAGICAL; break;
    case ITEM_FOOD:
    case ITEM_DRINK_CON:
    case ITEM_MEAT: donationroom = ROOM_VNUM_DONATION_FOOD; break;
    case ITEM_TRASH: donationroom = ROOM_VNUM_DONATION_TRASH; break;
    default:
      donationroom = ROOM_VNUM_DONATION_MISC;
      break;
   }
   
   if((pNew = get_room_index(donationroom))) {
      pOld = auction->seller->GetInRoom();
      obj_to_room(auction->item, pNew);
      char_from_room(auction->seller);
      char_to_room(auction->seller, pNew);
      act(AT_ACTION, "A hand appears in front of you, drops $p, and disappears.",
	  auction->seller, auction->item, NULL, TO_ROOM);
      char_from_room(auction->seller);
      char_to_room(auction->seller, pOld);
   }
}
