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
 * 			Table load/save Module				    *
 ****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "commands.h"

#include "paths.const.h"



ClassType::ClassType()
{
	attr_prime = 0;
	weapon = 0;
	guild = 0;
	skill_adept = 0;
	thac0_00 = 0;
	thac0_32 = 0;
	hp_min = 0;
	hp_max = 0;
	fMana = false;
	exp_base = 0;

}


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


/* global variables */
int top_sn;
int top_herb;

SkillType *		skill_table	[MAX_SKILL];
ClassType *	class_table	[MAX_CLASS];
char *			title_table	[MAX_CLASS]
					[MAX_LEVEL+1]
					[2];
SkillType *		herb_table	[MAX_HERB];


const char * const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb" };

SPELL_FUN *spell_function( const char *name )
{
    if ( !str_cmp( name, "spell_smaug" ))	     return spell_smaug;
    if ( !str_cmp( name, "spell_acid_blast" ))	     return spell_acid_blast;
    if ( !str_cmp( name, "spell_animate_dead" ))     return spell_animate_dead;
    if ( !str_cmp( name, "spell_astral_walk" ))	     return spell_astral_walk;
    if ( !str_cmp( name, "spell_blindness" ))	     return spell_blindness;
    if ( !str_cmp( name, "spell_burning_hands" ))    return spell_burning_hands;
    if ( !str_cmp( name, "spell_call_lightning" ))   return spell_call_lightning;
    if ( !str_cmp( name, "spell_cause_critical" ))   return spell_cause_critical;
    if ( !str_cmp( name, "spell_cause_light" ))	     return spell_cause_light;
    if ( !str_cmp( name, "spell_cause_serious" ))    return spell_cause_serious;
    if ( !str_cmp( name, "spell_change_sex" ))	     return spell_change_sex;
    if ( !str_cmp( name, "spell_charm_person" ))     return spell_charm_person;
    if ( !str_cmp( name, "spell_chill_touch" ))	     return spell_chill_touch;
    if ( !str_cmp( name, "spell_colour_spray" ))     return spell_colour_spray;
    if ( !str_cmp( name, "spell_control_weather" ))  return spell_control_weather;
    if ( !str_cmp( name, "spell_create_food" ))	     return spell_create_food;
    if ( !str_cmp( name, "spell_create_water" ))     return spell_create_water;
    if ( !str_cmp( name, "spell_cure_blindness" ))   return spell_cure_blindness;
    if ( !str_cmp( name, "spell_cure_poison" ))	     return spell_cure_poison;
    if ( !str_cmp( name, "spell_curse" ))	     return spell_curse;
    if ( !str_cmp( name, "spell_detect_poison" ))    return spell_detect_poison;
    if ( !str_cmp( name, "spell_dispel_evil" ))	     return spell_dispel_evil;
    if ( !str_cmp( name, "spell_dispel_magic" ))     return spell_dispel_magic;
	if ( !str_cmp( name, "spell_divine_invocation" )) return spell_divine_invocation;
    if ( !str_cmp( name, "spell_dream" ))	     return spell_dream;
    if ( !str_cmp( name, "spell_earthquake" ))	     return spell_earthquake;
    if ( !str_cmp( name, "spell_enchant_weapon" ))   return spell_enchant_weapon;
    if ( !str_cmp( name, "spell_energy_drain" ))     return spell_energy_drain;
    if ( !str_cmp( name, "spell_faerie_fire" ))	     return spell_faerie_fire;
    if ( !str_cmp( name, "spell_faerie_fog" ))	     return spell_faerie_fog;
    if ( !str_cmp( name, "spell_farsight" ))	     return spell_farsight;
    if ( !str_cmp( name, "spell_fireball" ))	     return spell_fireball;
    if ( !str_cmp( name, "spell_flamestrike" ))	     return spell_flamestrike;
    if ( !str_cmp( name, "spell_gate" ))	     return spell_gate;
    if ( !str_cmp( name, "spell_knock" ))	     return spell_knock;
    if ( !str_cmp( name, "spell_harm" ))	     return spell_harm;
	if ( !str_cmp( name, "spell_holy_sanctity" )) return spell_holy_sanctity;
	if ( !str_cmp( name, "spell_holy_might" )) return spell_holy_might;
    if ( !str_cmp( name, "spell_identify" ))	     return spell_identify;
    if ( !str_cmp( name, "spell_invis" ))	     return spell_invis;
    if ( !str_cmp( name, "spell_know_alignment" ))   return spell_know_alignment;
    if ( !str_cmp( name, "spell_lightning_bolt" ))   return spell_lightning_bolt;
    if ( !str_cmp( name, "spell_locate_object" ))    return spell_locate_object;
    if ( !str_cmp( name, "spell_magic_missile" ))    return spell_magic_missile;
    if ( !str_cmp( name, "spell_mist_walk" ))	     return spell_mist_walk;
    if ( !str_cmp( name, "spell_pass_door" ))	     return spell_pass_door;
    if ( !str_cmp( name, "spell_plant_pass" ))	     return spell_plant_pass;
    if ( !str_cmp( name, "spell_poison" ))	     return spell_poison;
    if ( !str_cmp( name, "spell_polymorph" ))	     return spell_polymorph;
    if ( !str_cmp( name, "spell_possess" ))	     return spell_possess;
    if ( !str_cmp( name, "spell_recharge" ))	     return spell_recharge;
    if ( !str_cmp( name, "spell_remove_curse" ))     return spell_remove_curse;
    if ( !str_cmp( name, "spell_remove_invis" ))     return spell_remove_invis;
    if ( !str_cmp( name, "spell_remove_trap" ))	     return spell_remove_trap;
    if ( !str_cmp( name, "spell_shocking_grasp" ))   return spell_shocking_grasp;
    if ( !str_cmp( name, "spell_sleep" ))	     return spell_sleep;
    if ( !str_cmp( name, "spell_solar_flight" ))     return spell_solar_flight;
    if ( !str_cmp( name, "spell_summon" ))	     return spell_summon;
    if ( !str_cmp( name, "spell_teleport" ))	     return spell_teleport;
    if ( !str_cmp( name, "spell_ventriloquate" ))    return spell_ventriloquate;
    if ( !str_cmp( name, "spell_weaken" ))	     return spell_weaken;
    if ( !str_cmp( name, "spell_word_of_recall" ))   return spell_word_of_recall;
    if ( !str_cmp( name, "spell_acid_breath" ))	     return spell_acid_breath;
    if ( !str_cmp( name, "spell_fire_breath" ))	     return spell_fire_breath;
    if ( !str_cmp( name, "spell_frost_breath" ))     return spell_frost_breath;
    if ( !str_cmp( name, "spell_gas_breath" ))	     return spell_gas_breath;
    if ( !str_cmp( name, "spell_lightning_breath" )) return spell_lightning_breath;
    if ( !str_cmp( name, "spell_spiral_blast" ))     return spell_spiral_blast;
    if ( !str_cmp( name, "spell_scorching_surge" ))  return spell_scorching_surge;
    if ( !str_cmp( name, "spell_helical_flow" ))     return spell_helical_flow;
    if ( !str_cmp( name, "spell_transport" ))	     return spell_transport;
    if ( !str_cmp( name, "spell_portal" ))	     return spell_portal;

    if ( !str_cmp( name, "spell_ethereal_fist" ))    return spell_ethereal_fist;
    if ( !str_cmp( name, "spell_spectral_furor" ))   return spell_spectral_furor;
    if ( !str_cmp( name, "spell_hand_of_chaos" ))    return spell_hand_of_chaos;
    if ( !str_cmp( name, "spell_disruption" ))	     return spell_disruption;
    if ( !str_cmp( name, "spell_sonic_resonance" ))  return spell_sonic_resonance;
    if ( !str_cmp( name, "spell_mind_wrack" ))	     return spell_mind_wrack;
    if ( !str_cmp( name, "spell_mind_wrench" ))	     return spell_mind_wrench;
    if ( !str_cmp( name, "spell_revive" ))	     return spell_revive;
    if ( !str_cmp( name, "spell_sulfurous_spray" ))  return spell_sulfurous_spray;
    if ( !str_cmp( name, "spell_caustic_fount" ))    return spell_caustic_fount;
    if ( !str_cmp( name, "spell_acetum_primus" ))    return spell_acetum_primus;
    if ( !str_cmp( name, "spell_galvanic_whip" ))    return spell_galvanic_whip;
    if ( !str_cmp( name, "spell_magnetic_thrust" ))  return spell_magnetic_thrust;
    if ( !str_cmp( name, "spell_quantum_spike" ))    return spell_quantum_spike;
    if ( !str_cmp( name, "spell_black_hand" ))	     return spell_black_hand;
    if ( !str_cmp( name, "spell_black_fist" ))	     return spell_black_fist;
    if ( !str_cmp( name, "spell_black_lightning" ))  return spell_black_lightning;
    if ( !str_cmp( name, "spell_midas_touch" ))      return spell_midas_touch;
    if ( !str_cmp( name, "spell_pixie_scents" ))     return spell_pixie_scents;

    if ( !str_cmp( name, "reserved" ))		     return NULL;
    if ( !str_cmp( name, "spell_null" ))	     return spell_null;
    return spell_notfound;
}

DO_FUN *skill_function( const char *name )
{
	switch( name[3] )
	{
		case 'a':
			if ( !str_cmp(name, "do_assist" ) ) return do_assist;
			if ( !str_cmp( name, "do_aassign" ))		return do_aassign;
			if ( !str_cmp( name, "do_advance" ))		return do_advance;
			if ( !str_cmp( name, "do_affected" ))		return do_affected;
			if ( !str_cmp( name, "do_afk" ))		return do_afk;
			if ( !str_cmp( name, "do_aid" ))		return do_aid;
			if ( !str_cmp( name, "do_allow" ))		return do_allow;
			if ( !str_cmp( name, "do_ansi" ))		return do_ansi;
			if ( !str_cmp( name, "do_answer" ))		return do_answer;
			if ( !str_cmp( name, "do_apply" ))		return do_apply;
			if ( !str_cmp( name, "do_appraise" ))		return do_appraise;
			if ( !str_cmp( name, "do_arena" ) )    return do_arena;
			if ( !str_cmp( name, "do_arenachat") ) return do_arenachat;
			if ( !str_cmp( name, "do_areas" ))		return do_areas;
			if ( !str_cmp( name, "do_aset" ))		return do_aset;
			if ( !str_cmp( name, "do_ask" ))		return do_ask;
			if ( !str_cmp( name, "do_astat" ))		return do_astat;
			if ( !str_cmp( name, "do_at" ))			return do_at;
			if ( !str_cmp( name, "do_auction" ))		return do_auction;
			if ( !str_cmp( name, "do_authorize" ))		return do_authorize;
			if ( !str_cmp( name, "do_autologin" ))  return do_autologin;
			if ( !str_cmp( name, "do_avclose" ))            return do_avclose;
			if ( !str_cmp( name, "do_avopen" ))             return do_avopen;
			if ( !str_cmp( name, "do_avstat" ))             return do_avstat;
			if ( !str_cmp( name, "do_avtalk" ))		return do_avtalk;
			if ( !str_cmp( name, "do_accounts"))    return do_accounts;
			break;
		case 'b':
			if ( !str_cmp( name, "do_behead"   ))   return do_behead;
			if ( !str_cmp( name, "do_backstab" ))		return do_backstab;
			if ( !str_cmp( name, "do_balzhur" ))		return do_balzhur;
			if ( !str_cmp( name, "do_bamfin" ))		return do_bamfin;
			if ( !str_cmp( name, "do_bamfout" ))		return do_bamfout;
			if ( !str_cmp( name, "do_ban" ))		return do_ban;
			if ( !str_cmp( name, "do_bank" ))		return do_bank;
			if ( !str_cmp( name, "do_barricade"))   return do_barricade;
			if ( !str_cmp( name, "do_bash" ))		return do_bash;
			if ( !str_cmp( name, "do_bashdoor" ))		return do_bashdoor;
			if ( !str_cmp( name, "do_beep" ))               return do_beep;
			if ( !str_cmp( name, "do_berserk" ))		return do_berserk;
			if ( !str_cmp( name, "do_bestow" ))		return do_bestow;
			if ( !str_cmp( name, "do_bestowarea" ))		return do_bestowarea;
			if ( !str_cmp( name, "do_bio" ))		return do_bio;
			if ( !str_cmp( name, "do_boards" ))		return do_boards;
			if ( !str_cmp( name, "do_bodybag" ))		return do_bodybag;
			if ( !str_cmp( name, "do_brandish" ))		return do_brandish;
			if ( !str_cmp( name, "do_brew" ))		return do_brew;
			if ( !str_cmp( name, "do_bset" ))		return do_bset;
			if ( !str_cmp( name, "do_bstat" ))		return do_bstat;
			if ( !str_cmp( name, "do_bug" ))		return do_bug;
			if ( !str_cmp( name, "do_bury" ))		return do_bury;
			if ( !str_cmp( name, "do_buy" ))		return do_buy;
			if ( !str_cmp( name, "do_bellow" ) )             return do_bellow;
			if ( !str_cmp( name, "do_bite" ) )              return do_bite;
			break;
		case 'c':
			if ( !str_cmp(name, "do_cook")) return do_cook;
			if ( !str_cmp( name, "do_cast" ))		return do_cast;
			if ( !str_cmp( name, "do_cedit" ))		return do_cedit;
			if ( !str_cmp( name, "do_channels" ))		return do_channels;
			if ( !str_cmp( name, "do_chat" ))		return do_chat;
			if ( !str_cmp( name, "do_check_vnums" ))	return do_check_vnums;
			if ( !str_cmp( name, "do_circle" ))		return do_circle;
			if ( !str_cmp( name, "do_clans" ))		return do_clans;
			if ( !str_cmp( name, "do_clantalk" ))		return do_clantalk;
			//if ( !str_cmp( name, "do_clear" ))		return do_clear;
			if ( !str_cmp( name, "do_climb" ))		return do_climb;
			if ( !str_cmp( name, "do_close" ))		return do_close;
			if ( !str_cmp( name, "do_color" ))              return do_color;
			if ( !str_cmp( name, "do_cmdtable" ))		return do_cmdtable;
			if ( !str_cmp( name, "do_cmenu" ))		return do_cmenu;
			if ( !str_cmp( name, "do_commands" ))		return do_commands;
			if ( !str_cmp( name, "do_comment" ))		return do_comment;
			if ( !str_cmp( name, "do_compare" ))		return do_compare;
			if ( !str_cmp( name, "do_connections" ))    return do_connections;
			if ( !str_cmp( name, "do_config" ))		return do_config;
			if ( !str_cmp( name, "do_consider" ))		return do_consider;
			if ( !str_cmp( name, "do_council_induct" ))	return do_council_induct;
			if ( !str_cmp( name, "do_council_outcast" ))	return do_council_outcast;
			if ( !str_cmp( name, "do_councils" ))		return do_councils;
			if ( !str_cmp( name, "do_counciltalk" ))	return do_counciltalk;
			if ( !str_cmp( name, "do_copyover" ))		return do_copyover;
			if ( !str_cmp( name, "do_credits" ))		return do_credits;
			if ( !str_cmp( name, "do_cset" ))		return do_cset;
			break;
		case 'd':
			if ( !str_cmp( name, "do_donate" )) return do_donate;
			if ( !str_cmp( name, "do_doas"    ))    return do_doas;
			if ( !str_cmp( name, "do_deities" ))		return do_deities;
			if ( !str_cmp( name, "do_deny" ))		return do_deny;
			if ( !str_cmp( name, "do_delete" ) )    return do_delete;
			if ( !str_cmp( name, "do_description" ))	return do_description;
			if ( !str_cmp( name, "do_destro" ))		return do_destro;
			if ( !str_cmp( name, "do_destroy" ))		return do_destroy;
			if ( !str_cmp( name, "do_detrap" ))		return do_detrap;
			if ( !str_cmp( name, "do_diagnose" ))		return do_diagnose;
			if ( !str_cmp( name, "do_devote" ))		return do_devote;
			if ( !str_cmp( name, "do_dig" ))		return do_dig;
			if ( !str_cmp( name, "do_disarm" ))		return do_disarm;
			if ( !str_cmp( name, "do_disconnect" ))		return do_disconnect;
			if ( !str_cmp( name, "do_dismount" ))		return do_dismount;
			if ( !str_cmp( name, "do_dmesg" ))		return do_dmesg;
			if ( !str_cmp( name, "do_dotted_to_vnum")) return do_dotted_to_vnum;
			if ( !str_cmp( name, "do_down" ))		return do_down;
			if ( !str_cmp( name, "do_drag" ))		return do_drag;
			if ( !str_cmp( name, "do_draw" ) )      return do_draw;
			if ( !str_cmp( name, "do_drink" ))		return do_drink;
			if ( !str_cmp( name, "do_drop" ))		return do_drop;
			break;
		case 'e':
			if ( !str_cmp( name, "do_east" ))    			return do_east;
			if ( !str_cmp( name, "do_eat" ))    			return do_eat;
			if ( !str_cmp( name, "do_echo" ))    			return do_echo;
			if ( !str_cmp( name, "do_editmovementmessage" ))return do_editmovementmessage;
			if ( !str_cmp( name, "do_editquest" )) 			return do_editquest;
			if ( !str_cmp( name, "do_emote" ))    			return do_emote;
			if ( !str_cmp( name, "do_empty" ))    			return do_empty;
			if ( !str_cmp( name, "do_enter" ))    			return do_enter;
			if ( !str_cmp( name, "do_equipment" ))      	return do_equipment;
			if ( !str_cmp( name, "do_evtalk" ))         	return do_evtalk;
			if ( !str_cmp( name, "do_examine" ))        	return do_examine;
			if ( !str_cmp( name, "do_excommunicate"))   	return do_excommunicate;
			if ( !str_cmp( name, "do_exits" ))    			return do_exits;
			break;
		case 'f':
			if ( !str_cmp( name, "do_fire" ) )      		return do_fire;
			if ( !str_cmp( name, "do_fireworks" ) )    		return do_fireworks;
			if ( !str_cmp( name, "do_feed" ))    			return do_feed;
			if ( !str_cmp( name, "do_fights" ))         	return do_fights;
			if ( !str_cmp( name, "do_fill" ))    			return do_fill;
			if ( !str_cmp( name, "do_fixchar" ))    		return do_fixchar;
			if ( !str_cmp( name, "do_fixme" ))      		return do_fixme;
			if ( !str_cmp( name, "do_flee" ))    			return do_flee;
			if ( !str_cmp( name, "do_foldarea" ))    		return do_foldarea;
			if ( !str_cmp( name, "do_follow" ))    			return do_follow;
			if ( !str_cmp( name, "do_for" ))    			return do_for;
			if ( !str_cmp( name, "do_forage" ))    			return do_forage;
			if ( !str_cmp( name, "do_force" ))    			return do_force;
			if ( !str_cmp( name, "do_forceclose" ))    		return do_forceclose;
			if ( !str_cmp( name, "do_forge" ))    			return do_forge;
			if ( !str_cmp( name, "do_form_password" ))  	return do_form_password;
			if ( !str_cmp( name, "do_forum_password" )) 	return do_forum_password;
			if ( !str_cmp( name, "do_fquit" ))    			return do_fquit;
			if ( !str_cmp( name, "do_freeze" ))    			return do_freeze;
			break;
		case 'g':
			if ( !str_cmp( name, "do_gain" ) )      	return do_gain;
			if ( !str_cmp( name, "do_get" ))			return do_get;
			if ( !str_cmp( name, "do_give" ))			return do_give;
			if ( !str_cmp( name, "do_glance" ))			return do_glance;
			if ( !str_cmp( name, "do_glory" ))			return do_glory;
			if ( !str_cmp( name, "do_gold" ))           return do_gold;
			if ( !str_cmp( name, "do_goto" ))			return do_goto;
			if ( !str_cmp( name, "do_gouge" ))			return do_gouge;
			if ( !str_cmp( name, "do_group" ))			return do_group;
			if ( !str_cmp( name, "do_grub" ))			return do_grub;
			if ( !str_cmp( name, "do_gtell" ))			return do_gtell;
			if ( !str_cmp( name, "do_guilds" ))			return do_guilds;
			if ( !str_cmp( name, "do_guildtalk" ))		return do_guildtalk;
			break;
		case 'h':
			if ( !str_cmp( name, "do_hedit" ))		return do_hedit;
			if ( !str_cmp( name, "do_hell" ))		return do_hell;
			if ( !str_cmp( name, "do_help" ))		return do_help;
			if ( !str_cmp( name, "do_hide" ))		return do_hide;
			if ( !str_cmp( name, "do_hitall" ))		return do_hitall;
			if ( !str_cmp( name, "do_hlist" ))		return do_hlist;
			if ( !str_cmp( name, "do_holylight" ))		return do_holylight;
			if ( !str_cmp( name, "do_homepage" ))		return do_homepage;
			if ( !str_cmp( name, "do_hset" ))		return do_hset;
			break;
		case 'i':
			if ( !str_cmp( name, "do_ide" ))		return do_ide;
			if ( !str_cmp( name, "do_idea" ))		return do_idea;
			if ( !str_cmp( name, "do_immortalize" ))	return do_immortalize;
			if ( !str_cmp( name, "do_immtalk" ))		return do_immtalk;
			if ( !str_cmp( name, "do_induct" ))		return do_induct;
			if ( !str_cmp( name, "do_installarea" ))	return do_installarea;
			if ( !str_cmp( name, "do_instaroom" ))		return do_instaroom;
			if ( !str_cmp( name, "do_instazone" ))		return do_instazone;
			if ( !str_cmp( name, "do_inventory" ))		return do_inventory;
			if ( !str_cmp( name, "do_invis" ))		return do_invis;
			break;
		case 'k':
			if ( !str_cmp( name, "do_kick" ))		return do_kick;
			if ( !str_cmp( name, "do_kill" ))		return do_kill;
			if ( !str_cmp( name, "do_kindle_fire" )) return do_kindle_fire;
			if ( !str_cmp( name, "do_knock" ))          return do_knock;
			if ( !str_cmp( name, "do_ksilyantoy" ))		return do_ksilyantoy;
			break;
		case 'l':
			if ( !str_cmp( name, "do_languages" ))		return do_languages;
			if ( !str_cmp( name, "do_last" ))		return do_last;
			if ( !str_cmp( name, "do_leave" ))		return do_leave;
			if ( !str_cmp( name, "do_level" ))		return do_level;
			if ( !str_cmp( name, "do_light" ))		return do_light;
			if ( !str_cmp( name, "do_list" ))		return do_list;
			if ( !str_cmp( name, "do_litterbug" ))		return do_litterbug;
			if ( !str_cmp( name, "do_loadarea" ))		return do_loadarea;
			if ( !str_cmp( name, "do_loadup" ))		return do_loadup;
			if ( !str_cmp( name, "do_lock" ))		return do_lock;
			if ( !str_cmp( name, "do_log" ))		return do_log;
			if ( !str_cmp( name, "do_look" ))		return do_look;
			if ( !str_cmp( name, "do_lookmap" ))		return do_lookmap;
			if ( !str_cmp( name, "do_low_purge" ))		return do_low_purge;
			break;
		case 'm':
			if ( !str_cmp( name, "do_meditate") ) return do_meditate;
			if ( !str_cmp( name, "do_mudconnector" ))  return do_mudconnector;
			if ( !str_cmp( name, "do_mailroom" ))		return do_mailroom;
			if ( !str_cmp( name, "do_make" ))		return do_make;

			/* matthew */
			if ( !str_cmp( name, "do_makestable" )) return do_makestable;

			if ( !str_cmp( name, "do_makeboard" ))		return do_makeboard;
			if ( !str_cmp( name, "do_makeclan" ))		return do_makeclan;
			if ( !str_cmp( name, "do_makecouncil" ))	return do_makecouncil;
			if ( !str_cmp( name, "do_makedeity" ))		return do_makedeity;
			if ( !str_cmp( name, "do_makerepair" ))		return do_makerepair;
			if ( !str_cmp( name, "do_makeshop" ))		return do_makeshop;
			if ( !str_cmp( name, "do_makewizlist" ))	return do_makewizlist;
			if ( !str_cmp( name, "do_mapout" ))		return do_mapout;
			if ( !str_cmp( name, "do_massign" ))		return do_massign;
			if ( !str_cmp( name, "do_mcreate" ))		return do_mcreate;
			if ( !str_cmp( name, "do_mdelete" ))		return do_mdelete;
			if ( !str_cmp( name, "do_memory" ))		return do_memory;
			if ( !str_cmp( name, "do_mfind" ))		return do_mfind;
			if ( !str_cmp( name, "do_mine" ))		return do_mine;
			if ( !str_cmp( name, "do_mload" ))		return do_mload;
			if ( !str_cmp( name, "do_mlist" ))		return do_mlist;
			if ( !str_cmp( name, "do_movementmessage" ))   return do_movementmessage;
			//if ( !str_cmp( name, "do_mmenu" ))		return do_mmenu;
			if ( !str_cmp( name, "do_mount" ))		return do_mount;
			if ( !str_cmp( name, "do_mpcontinue" )) return do_mpcontinue;
			if ( !str_cmp( name, "do_mpforcelook" ))       return do_mpforcelook;
			if ( !str_cmp( name, "do_mpwalk" ))     return do_mpwalk;
			if ( !str_cmp( name, "do_mp_close_passage" ))	return do_mp_close_passage;
			if ( !str_cmp( name, "do_mp_damage" ))		return do_mp_damage;
			if ( !str_cmp( name, "do_mp_deposit" ))		return do_mp_deposit;
			if ( !str_cmp( name, "do_mp_open_passage" ))	return do_mp_open_passage;
			if ( !str_cmp( name, "do_mp_practice" ))	return do_mp_practice;
			if ( !str_cmp( name, "do_mp_restore" ))		return do_mp_restore;
			if ( !str_cmp( name, "do_mp_setthief" ))    return do_mp_setthief;
			if ( !str_cmp( name, "do_mp_setwanted" ))	return do_mp_setwanted;
			if ( !str_cmp( name, "do_mp_slay" ))		return do_mp_slay;
			if ( !str_cmp( name, "do_mp_withdraw" ))	return do_mp_withdraw;
			if ( !str_cmp( name, "do_mpadvance" ))		return do_mpadvance;
			if ( !str_cmp( name, "do_mpapply" ))		return do_mpapply;
			if ( !str_cmp( name, "do_mpapplyb" ))		return do_mpapplyb;
			if ( !str_cmp( name, "do_mpasound" ))		return do_mpasound;
			if ( !str_cmp( name, "do_mpat" ))			return do_mpat;
			if ( !str_cmp( name, "do_mpaward" ))		return do_mpaward;
			if ( !str_cmp( name, "do_mpbecome" )) 		return do_mpbecome;
			if ( !str_cmp( name, "do_mpdream" ))		return do_mpdream;
			if ( !str_cmp( name, "do_mpareaecho" ) )    return do_mpareaecho;
			if ( !str_cmp( name, "do_mpecho" ))			return do_mpecho;
			if ( !str_cmp( name, "do_mpechoaround" ))	return do_mpechoaround;
			if ( !str_cmp( name, "do_mpechoat" ))		return do_mpechoat;
			if ( !str_cmp( name, "do_mpedit" ))			return do_mpedit;
			if ( !str_cmp( name, "do_mpfavor" ))		return do_mpfavor;
            if ( !str_cmp( name, "do_mpflag" ))			return do_mpflag;
			if ( !str_cmp( name, "do_mpforce" ))		return do_mpforce;
			if ( !str_cmp( name, "do_mpglory" ))		return do_mpglory;
			if ( !str_cmp( name, "do_mpgoto" ))			return do_mpgoto;
			if ( !str_cmp( name, "do_mpinvis" ))		return do_mpinvis;
			if ( !str_cmp( name, "do_mpjunk" ))			return do_mpjunk;
			if ( !str_cmp( name, "do_mpkill" ))			return do_mpkill;
			if ( !str_cmp( name, "do_mpmload" ))		return do_mpmload;
			if ( !str_cmp( name, "do_mpmset" ))			return do_mpmset;
			if ( !str_cmp( name, "do_mpnothing" ))		return do_mpnothing;
			if ( !str_cmp( name, "do_mpoload" ))		return do_mpoload;
			if ( !str_cmp( name, "do_mposet" ))			return do_mposet;
			if ( !str_cmp( name, "do_mppurge" ))		return do_mppurge;
			if ( !str_cmp( name, "do_mpstat" ))			return do_mpstat;
			if ( !str_cmp( name, "do_mptransfer" ))		return do_mptransfer;
			if ( !str_cmp( name, "do_mrange" ))			return do_mrange;
			if ( !str_cmp( name, "do_mscriptilyan" )) 	return do_mscriptilyan;
			if ( !str_cmp( name, "do_mset" ))			return do_mset;
			if ( !str_cmp( name, "do_mstat" ))			return do_mstat;
			if ( !str_cmp( name, "do_murde" ))			return do_murde;
			if ( !str_cmp( name, "do_murder" ))			return do_murder;
			if ( !str_cmp( name, "do_muse" ))			return do_muse;
			if ( !str_cmp( name, "do_music" ))			return do_music;
			if ( !str_cmp( name, "do_mwhere" ))			return do_mwhere;
			if ( !str_cmp( name, "do_mpdispel") ) 		return do_mpdispel;
			if ( !str_cmp( name, "do_mphunt" ) ) 		return do_mphunt;
			break;
		case 'n':
			if ( !str_cmp( name, "do_namechange" ))		return do_namechange;
			if ( !str_cmp( name, "do_new_dump" ))       return do_new_dump;
			if ( !str_cmp( name, "do_newbiechat" ))		return do_newbiechat;
			if ( !str_cmp( name, "do_newbielock" ))         return do_newbielock;
			if ( !str_cmp( name, "do_newbieset" ))		return do_newbieset;
			if ( !str_cmp( name, "do_new_inv" ) )           return do_new_inv;
			if ( !str_cmp( name, "do_newwho" ))         return do_newwho;
			if ( !str_cmp( name, "do_newzones" ))		return do_newzones;
			if ( !str_cmp( name, "do_noemote" ))		return do_noemote;
			if ( !str_cmp( name, "do_nohelps" ))		return do_nohelps;
			if ( !str_cmp( name, "do_noresolve" ))		return do_noresolve;
			if ( !str_cmp( name, "do_north" ))		return do_north;
			if ( !str_cmp( name, "do_northeast" ))		return do_northeast;
			if ( !str_cmp( name, "do_northwest" ))		return do_northwest;
			if ( !str_cmp( name, "do_notell" ))		return do_notell;
			if ( !str_cmp( name, "do_notitle" ))            return do_notitle;
			if ( !str_cmp( name, "do_noteroom" ))		return do_noteroom;
			break;
		case 'o':
			if ( !str_cmp( name, "do_ooc" ) )           return do_ooc;
			if ( !str_cmp( name, "do_oassign" ))		return do_oassign;
			if ( !str_cmp( name, "do_object_csv_dump" )) return do_object_csv_dump;
			if ( !str_cmp( name, "do_object_csv_read" )) return do_object_csv_read;
			if ( !str_cmp( name, "do_object_backup" ) ) return do_object_backup;
			if ( !str_cmp( name, "do_ocreate" ))		return do_ocreate;
			if ( !str_cmp( name, "do_odelete" ))		return do_odelete;
			if ( !str_cmp( name, "do_ofind" ))		return do_ofind;
			if ( !str_cmp( name, "do_ogrub" ))		return do_ogrub;
			if ( !str_cmp( name, "do_oload" ))		return do_oload;
			if ( !str_cmp( name, "do_oldscore" ))		return do_oldscore;
			if ( !str_cmp( name, "do_olist" ))		return do_olist;
			//if ( !str_cmp( name, "do_omenu" ))		return do_omenu;
			if ( !str_cmp( name, "do_opedit" ))		return do_opedit;
			if ( !str_cmp( name, "do_open" ))		return do_open;
			/*	if ( !str_cmp( name, "do_opentourney" ))	return do_opentourney; */
			if ( !str_cmp( name, "do_opstat" ))		return do_opstat;
			if ( !str_cmp( name, "do_orange" ))		return do_orange;
			if ( !str_cmp( name, "do_order" ))		return do_order;
			if ( !str_cmp( name, "do_orders" ))		return do_orders;
			if ( !str_cmp( name, "do_ordertalk" ))		return do_ordertalk;
			if ( !str_cmp( name, "do_oreport" ))    return do_oreport;
			if ( !str_cmp( name, "do_osay" ))		return do_osay;
			if ( !str_cmp( name, "do_oscriptilyan" )) return do_oscriptilyan;
			if ( !str_cmp( name, "do_oset" ))		return do_oset;
			if ( !str_cmp( name, "do_ostat" ))		return do_ostat;
			if ( !str_cmp( name, "do_outcast" ))		return do_outcast;
			if ( !str_cmp( name, "do_owhere" ))		return do_owhere;
			break;
		case 'p':
			//if ( !str_cmp( name, "do_pagelen" ))		return do_pagelen;
			if ( !str_cmp( name, "do_pager" ))		return do_pager;
			if ( !str_cmp( name, "do_pardon" ))		return do_pardon;
			if ( !str_cmp( name, "do_password" ))		return do_password;
			if ( !str_cmp( name, "do_peace" ))		return do_peace;
			if ( !str_cmp( name, "do_pick" ))		return do_pick;
			if ( !str_cmp( name, "do_playbuild" ))          return do_playbuild;
			if ( !str_cmp( name, "do_poison_weapon" ))	return do_poison_weapon;
			if ( !str_cmp( name, "do_pray" ) ) return do_pray;
			if ( !str_cmp( name, "do_practice" ))		return do_practice;
			if ( !str_cmp( name, "do_prompt" ))		return do_prompt;
			if ( !str_cmp( name, "do_pull" ))		return do_pull;
			if ( !str_cmp( name, "do_punch" ))		return do_punch;
			if ( !str_cmp( name, "do_purge" ))		return do_purge;
			if ( !str_cmp( name, "do_push" ))		return do_push;
			if ( !str_cmp( name, "do_put" ))		return do_put;
			break;
		case 'q':
			if ( !str_cmp( name, "do_qpset" ))		return do_qpset;
			if ( !str_cmp( name, "do_qset" ))               return do_qset;
			if ( !str_cmp( name, "do_qstat" ))              return do_qstat;
			if ( !str_cmp( name, "do_quaff" ))		return do_quaff;
			if ( !str_cmp( name, "do_qedit" ))              return do_qedit;
			if ( !str_cmp( name, "do_quest" ))		return do_quest;
			if ( !str_cmp( name, "do_qui" ))		return do_qui;
			if ( !str_cmp( name, "do_quit" ))		return do_quit;
			break;
		case 'r':
			if ( !str_cmp( name, "do_rarelist") )   return do_rarelist;
			if ( !str_cmp( name, "do_rareset") )   return do_rareset;
			if ( !str_cmp( name, "do_rscriptilyan") ) return do_rscriptilyan;
			if ( !str_cmp( name, "do_rfind" ) )     return do_rfind;
			if ( !str_cmp( name, "do_rank" ))		return do_rank;
			if ( !str_cmp( name, "do_rassign" ))		return do_rassign;
			if ( !str_cmp( name, "do_rat" ))		return do_rat;
			if ( !str_cmp( name, "do_rdelete" ))		return do_rdelete;
			if ( !str_cmp( name, "do_reboo" ))		return do_reboo;
			if ( !str_cmp( name, "do_renumber" ))   return do_renumber;
			if ( !str_cmp( name, "do_reboot" ))		return do_reboot;
			if ( !str_cmp( name, "do_recho" ))		return do_recho;
			if ( !str_cmp( name, "do_recite" ))		return do_recite;
			if ( !str_cmp( name, "do_redit" ))		return do_redit;
			//if ( !str_cmp( name, "do_redraw_page" ))	return do_redraw_page;
			//if ( !str_cmp( name, "do_refresh_page" ))	return do_refresh_page;
			if ( !str_cmp( name, "do_regoto" ))		return do_regoto;
			if ( !str_cmp( name, "do_remove" ))		return do_remove;
			if ( !str_cmp( name, "do_rent" ))		return do_rent;
			if ( !str_cmp( name, "do_repair" ))		return do_repair;
			if ( !str_cmp( name, "do_repairset" ))		return do_repairset;
			if ( !str_cmp( name, "do_repairshops" ))	return do_repairshops;
			if ( !str_cmp( name, "do_repairstat" ))		return do_repairstat;
			if ( !str_cmp( name, "do_reply" ))		return do_reply;
			if ( !str_cmp( name, "do_report" ))		return do_report;
			if ( !str_cmp( name, "do_respond" ) )  return do_respond;
			if ( !str_cmp( name, "do_rescue" ))		return do_rescue;
			if ( !str_cmp( name, "do_reset" ))		return do_reset;
			if ( !str_cmp( name, "do_rest" ))		return do_rest;
			if ( !str_cmp( name, "do_restore" ))		return do_restore;
			if ( !str_cmp( name, "do_restoretime" ))	return do_restoretime;
			if ( !str_cmp( name, "do_restrict" ))		return do_restrict;
			if ( !str_cmp( name, "do_retell" ))		return do_retell;
			if ( !str_cmp( name, "do_retire" ))		return do_retire;
			if ( !str_cmp( name, "do_retran" ))		return do_retran;
			if ( !str_cmp( name, "do_return" ))		return do_return;
			if ( !str_cmp( name, "do_revert" ))		return do_revert;
			if ( !str_cmp( name, "do_rip" ))		return do_rip;
			if ( !str_cmp( name, "do_rlist" ))		return do_rlist;
			//if ( !str_cmp( name, "do_rmenu" ))		return do_rmenu;
			if ( !str_cmp( name, "do_rpedit" ))		return do_rpedit;
			if ( !str_cmp( name, "do_rpstat" ))		return do_rpstat;
			if ( !str_cmp( name, "do_rreset" ))		return do_rreset;
			if ( !str_cmp( name, "do_rstat" ))		return do_rstat;
			break;
		case 's':
			if ( !str_cmp( name, "do_spam" )) return do_spam;
			if ( !str_cmp( name, "do_sacrifice" ))		return do_sacrifice;
			if ( !str_cmp( name, "do_save" ))		return do_save;
			if ( !str_cmp( name, "do_savearea" ))		return do_savearea;
			if ( !str_cmp( name, "do_say" ))		return do_say;
			if ( !str_cmp( name, "do_sayto" ))		return do_sayto;
			if ( !str_cmp( name, "do_scan" ))		return do_scan;
			if ( !str_cmp( name, "do_scentedit" ))		return do_scentedit;
			if ( !str_cmp( name, "do_score" ))		return do_score;
			if ( !str_cmp( name, "do_scribe" ))		return do_scribe;
			if ( !str_cmp( name, "do_search" ))		return do_search;
			if ( !str_cmp( name, "do_sedit" ))		return do_sedit;
			if ( !str_cmp( name, "do_sell" ))		return do_sell;
			if ( !str_cmp( name, "do_set_boot_time" ))	return do_set_boot_time;
			if ( !str_cmp( name, "do_setclan" ))		return do_setclan;
			if ( !str_cmp( name, "do_setclass" ))		return do_setclass;
			if ( !str_cmp( name, "do_setcouncil" ))		return do_setcouncil;
			if ( !str_cmp( name, "do_setdeity" ))		return do_setdeity;
			if ( !str_cmp( name, "do_setgembase" ))     return do_setgembase;
			if ( !str_cmp( name, "do_sgtalk" ))             return do_sgtalk;
			if ( !str_cmp( name, "do_sheathe" ) )   return do_sheathe;
			if ( !str_cmp( name, "do_shops" ))		return do_shops;
			if ( !str_cmp( name, "do_shopset" ))		return do_shopset;
			if ( !str_cmp( name, "do_shopstat" ))		return do_shopstat;
			if ( !str_cmp( name, "do_shout" ))		return do_shout;
			if ( !str_cmp( name, "do_shove" ))		return do_shove;
			if ( !str_cmp( name, "do_showclan" ))		return do_showclan;
			if ( !str_cmp( name, "do_showclass" ))		return do_showclass;
			if ( !str_cmp( name, "do_showcouncil" ))	return do_showcouncil;
			if ( !str_cmp( name, "do_showdeity" ))		return do_showdeity;
			if ( !str_cmp( name, "do_showquest" ))		return do_showquest;
			if ( !str_cmp( name, "do_showvault" ))      return do_showvault;
			if ( !str_cmp( name, "do_shutdow" ))		return do_shutdow;
			if ( !str_cmp( name, "do_shutdown" ))		return do_shutdown;
			if ( !str_cmp( name, "do_silence" ))		return do_silence;
			if ( !str_cmp( name, "do_sit" ))		return do_sit;
			if ( !str_cmp( name, "do_sla" ))		return do_sla;
			if ( !str_cmp( name, "do_slay" ))		return do_slay;
			if ( !str_cmp( name, "do_sleep" ))		return do_sleep;
			if ( !str_cmp( name, "do_slice" ))		return do_slice;
			if ( !str_cmp( name, "do_slist" ))		return do_slist;
			if ( !str_cmp( name, "do_slookup" ))		return do_slookup;
			if ( !str_cmp( name, "do_smell" ))		return do_smell;
			if ( !str_cmp( name, "do_smelt" ))		return do_smelt;
			if ( !str_cmp( name, "do_smoke" ))		return do_smoke;
			if ( !str_cmp( name, "do_snoop" ))		return do_snoop;
			if ( !str_cmp( name, "do_sober" ))		return do_sober;
			if ( !str_cmp( name, "do_socials" ))		return do_socials;
			if ( !str_cmp( name, "do_south" ))		return do_south;
			if ( !str_cmp( name, "do_southeast" ))		return do_southeast;
			if ( !str_cmp( name, "do_southwest" ))		return do_southwest;
			if ( !str_cmp( name, "do_speak" ))		return do_speak;
			if ( !str_cmp( name, "do_spells" ))     return do_spells;
			if ( !str_cmp( name, "do_split" ))		return do_split;
			if ( !str_cmp( name, "do_sset" ))		return do_sset;

			if ( !str_cmp( name, "do_stand" ))		return do_stand;
			if ( !str_cmp( name, "do_stable" ))		return do_stable;
			if ( !str_cmp( name, "do_stableset" ))  return do_stableset;
			if ( !str_cmp( name, "do_stables"))     return do_stables;
			if ( !str_cmp( name, "do_stablestat"))  return do_stablestat;
			if ( !str_cmp( name, "do_stagename"))   return do_stagename;

			if ( !str_cmp( name, "do_steal" ))		return do_steal;
			if ( !str_cmp( name, "do_stun" ))		return do_stun;
			if ( !str_cmp( name, "do_supplicate" ))		return do_supplicate;
			if ( !str_cmp( name, "do_switch" ))		return do_switch;
			break;
		case 't':
			if ( !str_cmp( name, "do_throw" ) )     return do_throw;
			if ( !str_cmp( name, "do_test" ) )              return do_test;
			if ( !str_cmp( name, "do_trainset" ) ) return do_trainset;
			if ( !str_cmp( name, "do_trainstat" ) ) return do_trainstat;
			if ( !str_cmp( name, "do_tamp" ))		return do_tamp;
			if ( !str_cmp( name, "do_tell" ))		return do_tell;
			if ( !str_cmp( name, "do_think" ))		return do_think;
			if ( !str_cmp( name, "do_time" ))		return do_time;
			if ( !str_cmp( name, "do_timecmd" ))		return do_timecmd;
			if ( !str_cmp( name, "do_title" ))		return do_title;
			if ( !str_cmp( name, "do_titlechange" ))        return do_titlechange;
			if ( !str_cmp( name, "do_titlelock" ))  return do_titlelock;
			if ( !str_cmp( name, "do_track" ))		return do_track;
			if ( !str_cmp( name, "do_transfer" ))		return do_transfer;
			if ( !str_cmp( name, "do_trust" ))		return do_trust;
			if ( !str_cmp( name, "do_typo" ))		return do_typo;
			if ( !str_cmp( name, "do_television_set" )) return do_television_set;
			break;
		case 'u':
			if ( !str_cmp( name, "do_unfoldarea" ))		return do_unfoldarea;
			if ( !str_cmp( name, "do_unhell" ))		return do_unhell;
			if ( !str_cmp( name, "do_unlock" ))		return do_unlock;
			if ( !str_cmp( name, "do_unsilence" ))          return do_unsilence;
			if ( !str_cmp( name, "do_up" ))			return do_up;
			if ( !str_cmp( name, "do_use" ))		return do_use;
			if ( !str_cmp( name, "do_users" ))		return do_users;
			break;
		case 'v':
			if ( !str_cmp( name, "do_value" ))		return do_value;
			if ( !str_cmp( name, "do_vault" ))      return do_vault;
			if ( !str_cmp( name, "do_visible" ))		return do_visible;
			if ( !str_cmp( name, "do_vnums" ))		return do_vnums;
			if ( !str_cmp( name, "do_vnum_to_dotted" ))		return do_vnum_to_dotted;
			if ( !str_cmp( name, "do_vsearch" ))		return do_vsearch;
			break;
		case 'w':
			if ( !str_cmp( name, "do_wwho") )     return do_wwho;
			if ( !str_cmp( name, "do_wake" ))		return do_wake;
			if ( !str_cmp( name, "do_wartalk" ))		return do_wartalk;
			if ( !str_cmp( name, "do_wear" ))		return do_wear;
			if ( !str_cmp( name, "do_weather" ))		return do_weather;
			if ( !str_cmp( name, "do_west" ))		return do_west;
			if ( !str_cmp( name, "do_where" ))		return do_where;
			if ( !str_cmp( name, "do_whisper" ))	return do_whisper;
			if ( !str_cmp( name, "do_who" ))		return do_who;
			if ( !str_cmp( name, "do_whois" ))		return do_whois;
			if ( !str_cmp( name, "do_wimpy" ))		return do_wimpy;
			if ( !str_cmp( name, "do_wizhelp" ))		return do_wizhelp;
			if ( !str_cmp( name, "do_wizlist" ))		return do_wizlist;
			if ( !str_cmp( name, "do_wizlock" ))		return do_wizlock;
			if ( !str_cmp( name, "do_worth" ))          return do_worth;
			break;
		case 'y':
			if ( !str_cmp( name, "do_yell" ))		return do_yell;
			break;
		case 'z':
			if ( !str_cmp( name, "do_zap" ))		return do_zap;
			if ( !str_cmp( name, "do_zones" ))		return do_zones;
	}
	return skill_notfound;
}

const char *spell_name( SPELL_FUN *spell )
{
    if ( spell == spell_smaug )		    return "spell_smaug";
    if ( spell == spell_acid_blast )	    return "spell_acid_blast";
    if ( spell == spell_animate_dead )	    return "spell_animate_dead";
    if ( spell == spell_astral_walk )	    return "spell_astral_walk";
    if ( spell == spell_blindness )	    return "spell_blindness";
    if ( spell == spell_burning_hands )	    return "spell_burning_hands";
    if ( spell == spell_call_lightning )    return "spell_call_lightning";
    if ( spell == spell_cause_critical )    return "spell_cause_critical";
    if ( spell == spell_cause_light )	    return "spell_cause_light";
    if ( spell == spell_cause_serious )	    return "spell_cause_serious";
    if ( spell == spell_change_sex )	    return "spell_change_sex";
    if ( spell == spell_charm_person )	    return "spell_charm_person";
    if ( spell == spell_chill_touch )	    return "spell_chill_touch";
    if ( spell == spell_colour_spray )	    return "spell_colour_spray";
    if ( spell == spell_control_weather )   return "spell_control_weather";
    if ( spell == spell_create_food )	    return "spell_create_food";
    if ( spell == spell_create_water )	    return "spell_create_water";
    if ( spell == spell_cure_blindness )    return "spell_cure_blindness";
    if ( spell == spell_cure_poison )	    return "spell_cure_poison";
    if ( spell == spell_curse )		    return "spell_curse";
    if ( spell == spell_detect_poison )	    return "spell_detect_poison";
    if ( spell == spell_dispel_evil )	    return "spell_dispel_evil";
    if ( spell == spell_dispel_magic )	    return "spell_dispel_magic";
	if ( spell == spell_divine_invocation ) return "spell_divine_invocation";
    if ( spell == spell_dream )		    return "spell_dream";
    if ( spell == spell_earthquake )	    return "spell_earthquake";
    if ( spell == spell_enchant_weapon )    return "spell_enchant_weapon";
    if ( spell == spell_energy_drain )	    return "spell_energy_drain";
    if ( spell == spell_faerie_fire )	    return "spell_faerie_fire";
    if ( spell == spell_faerie_fog )	    return "spell_faerie_fog";
    if ( spell == spell_farsight )	    return "spell_farsight";
    if ( spell == spell_fireball )	    return "spell_fireball";
    if ( spell == spell_flamestrike )	    return "spell_flamestrike";
    if ( spell == spell_gate )		    return "spell_gate";
    if ( spell == spell_knock )		    return "spell_knock";
    if ( spell == spell_harm )		    return "spell_harm";
	if ( spell == spell_holy_might ) return "spell_holy_might";
	if ( spell == spell_holy_sanctity ) return "spell_holy_sanctity";
    if ( spell == spell_identify )	    return "spell_identify";
    if ( spell == spell_invis )		    return "spell_invis";
    if ( spell == spell_know_alignment )    return "spell_know_alignment";
    if ( spell == spell_lightning_bolt )    return "spell_lightning_bolt";
    if ( spell == spell_locate_object )	    return "spell_locate_object";
    if ( spell == spell_magic_missile )	    return "spell_magic_missile";
    if ( spell == spell_mist_walk )	    return "spell_mist_walk";
    if ( spell == spell_pass_door )	    return "spell_pass_door";
    if ( spell == spell_plant_pass )	    return "spell_plant_pass";
    if ( spell == spell_poison )	    return "spell_poison";
    if ( spell == spell_polymorph )	    return "spell_polymorph";
    if ( spell == spell_possess )	    return "spell_possess";
    if ( spell == spell_recharge )	    return "spell_recharge";
    if ( spell == spell_remove_curse )	    return "spell_remove_curse";
    if ( spell == spell_remove_invis )	    return "spell_remove_invis";
    if ( spell == spell_remove_trap )	    return "spell_remove_trap";
    if ( spell == spell_shocking_grasp )    return "spell_shocking_grasp";
    if ( spell == spell_sleep )		    return "spell_sleep";
    if ( spell == spell_solar_flight )	    return "spell_solar_flight";
    if ( spell == spell_summon )	    return "spell_summon";
    if ( spell == spell_teleport )	    return "spell_teleport";
    if ( spell == spell_ventriloquate )	    return "spell_ventriloquate";
    if ( spell == spell_weaken )	    return "spell_weaken";
    if ( spell == spell_word_of_recall )    return "spell_word_of_recall";
    if ( spell == spell_acid_breath )	    return "spell_acid_breath";
    if ( spell == spell_fire_breath )	    return "spell_fire_breath";
    if ( spell == spell_frost_breath )	    return "spell_frost_breath";
    if ( spell == spell_gas_breath )	    return "spell_gas_breath";
    if ( spell == spell_lightning_breath )  return "spell_lightning_breath";
    if ( spell == spell_spiral_blast )	    return "spell_spiral_blast";
    if ( spell == spell_scorching_surge )   return "spell_scorching_surge";
    if ( spell == spell_helical_flow )	    return "spell_helical_flow";
    if ( spell == spell_transport )	    return "spell_transport";
    if ( spell == spell_portal )	    return "spell_portal";

    if ( spell == spell_ethereal_fist )	    return "spell_ethereal_fist";
    if ( spell == spell_spectral_furor )    return "spell_spectral_furor";
    if ( spell == spell_hand_of_chaos )	    return "spell_hand_of_chaos";
    if ( spell == spell_disruption )	    return "spell_disruption";
    if ( spell == spell_sonic_resonance )   return "spell_sonic_resonance";
    if ( spell == spell_mind_wrack )	    return "spell_mind_wrack";
    if ( spell == spell_mind_wrench )	    return "spell_mind_wrench";
    if ( spell == spell_revive )	    return "spell_revive";
    if ( spell == spell_sulfurous_spray )   return "spell_sulfurous_spray";
    if ( spell == spell_caustic_fount )	    return "spell_caustic_fount";
    if ( spell == spell_acetum_primus )	    return "spell_acetum_primus";
    if ( spell == spell_galvanic_whip )	    return "spell_galvanic_whip";
    if ( spell == spell_magnetic_thrust )   return "spell_magnetic_thrust";
    if ( spell == spell_quantum_spike )	    return "spell_quantum_spike";
    if ( spell == spell_black_hand )	    return "spell_black_hand";
    if ( spell == spell_black_fist )	    return "spell_black_fist";
    if ( spell == spell_black_lightning )   return "spell_black_lightning";
    if ( spell == spell_midas_touch )       return "spell_midas_touch";
    if ( spell == spell_pixie_scents )	    return "spell_pixie_scents";

    if ( spell == spell_null )		    return "spell_null";
    return "reserved";
}

const char *skill_name( DO_FUN *skill )
{
    if ( skill == do_accounts ) return "do_accounts";
    if ( skill == do_assist ) return "do_assist";
    if ( skill == do_aassign )		return "do_aassign";
    if ( skill == do_advance )		return "do_advance";
    if ( skill == do_affected )		return "do_affected";
    if ( skill == do_afk )		return "do_afk";
    if ( skill == do_aid )		return "do_aid";
    if ( skill == do_allow )		return "do_allow";
    if ( skill == do_ansi )		return "do_ansi";
    if ( skill == do_answer )		return "do_answer";
    if ( skill == do_apply )		return "do_apply";
    if ( skill == do_appraise )		return "do_appraise";
    if ( skill == do_arena )     return "do_arena";
    if ( skill == do_arenachat ) return "do_arenachat";
    if ( skill == do_areas )		return "do_areas";
    if ( skill == do_aset )		return "do_aset";
    if ( skill == do_ask )		return "do_ask";
    if ( skill == do_astat )		return "do_astat";
    if ( skill == do_at )		return "do_at";
    if ( skill == do_auction )		return "do_auction";
    if ( skill == do_authorize )	return "do_authorize";
	if ( skill == do_autologin )    return "do_autologin";
    if ( skill == do_avclose )          return "do_avclose";
    if ( skill == do_avopen )		return "do_avopen";
    if ( skill == do_avstat )           return "do_avstat";
    if ( skill == do_avtalk )           return "do_avtalk";
    if ( skill == do_behead )           return "do_behead";
    if ( skill == do_backstab )		return "do_backstab";
    if ( skill == do_balzhur )		return "do_balzhur";
    if ( skill == do_bamfin )		return "do_bamfin";
    if ( skill == do_bellow )           return "do_bellow";
    if ( skill == do_bamfout )		return "do_bamfout";
    if ( skill == do_ban )		return "do_ban";
	if ( skill == do_bank )		return "do_bank";
	if ( skill == do_barricade )        return "do_barricade";
    if ( skill == do_bash )		return "do_bash";
    if ( skill == do_bashdoor )		return "do_bashdoor";
    if ( skill == do_beep )            return "do_beep";
    if ( skill == do_berserk )		return "do_berserk";
    if ( skill == do_bestow )		return "do_bestow";
    if ( skill == do_bestowarea )	return "do_bestowarea";
    if ( skill == do_bio )		return "do_bio";
    if ( skill == do_bite )             return "do_bite";
    if ( skill == do_boards )		return "do_boards";
    if ( skill == do_bodybag )		return "do_bodybag";
    if ( skill == do_brandish )		return "do_brandish";
    if ( skill == do_brew )		return "do_brew";
    if ( skill == do_bset )		return "do_bset";
    if ( skill == do_bstat )		return "do_bstat";
    if ( skill == do_bug )		return "do_bug";
    if ( skill == do_bury )		return "do_bury";
    if ( skill == do_buy )		return "do_buy";
    if ( skill ==  do_cook ) return "do_cook";
    if ( skill == do_cast )		return "do_cast";
    if ( skill == do_cedit )		return "do_cedit";
    if ( skill == do_channels )		return "do_channels";
    if ( skill == do_chat )		return "do_chat";
    if ( skill == do_check_vnums )	return "do_check_vnums";
    if ( skill == do_circle )		return "do_circle";
    if ( skill == do_clans )		return "do_clans";
    if ( skill == do_clantalk )		return "do_clantalk";
    //if ( skill == do_clear )		return "do_clear";
    if ( skill == do_climb )		return "do_climb";
    if ( skill == do_close )		return "do_close";
    if ( skill == do_color )            return "do_color";
    if ( skill == do_cmdtable )		return "do_cmdtable";
    if ( skill == do_cmenu )		return "do_cmenu";
    if ( skill == do_commands )		return "do_commands";
    if ( skill == do_comment )		return "do_comment";
    if ( skill == do_compare )		return "do_compare";
	if ( skill == do_connections )  return "do_connections";
    if ( skill == do_config )		return "do_config";
    if ( skill == do_consider )		return "do_consider";
    if ( skill == do_copyover )		return "do_copyover";
    if ( skill == do_council_induct )	return "do_council_induct";
    if ( skill == do_council_outcast )	return "do_council_outcast";
    if ( skill == do_councils )		return "do_councils";
    if ( skill == do_counciltalk )	return "do_counciltalk";
    if ( skill == do_credits )		return "do_credits";
    if ( skill == do_cset )		return "do_cset";
    if ( skill == do_deities )		return "do_deities";
    if ( skill == do_deny )		return "do_deny";
    if ( skill == do_delete ) return "do_delete";
    if ( skill == do_description )	return "do_description";
    if ( skill == do_destro )		return "do_destro";
    if ( skill == do_destroy )		return "do_destroy";
    if ( skill == do_detrap )		return "do_detrap";
    if ( skill == do_devote )		return "do_devote";
    if ( skill == do_dig )		return "do_dig";
    if ( skill == do_disarm )		return "do_disarm";
    if ( skill == do_disconnect )	return "do_disconnect";
    if ( skill == do_dismount )		return "do_dismount";
    if ( skill == do_dmesg )		return "do_dmesg";
    if ( skill == do_doas )  return "do_doas";
    if ( skill == do_donate ) return "do_donate";
	if ( skill == do_dotted_to_vnum ) return "do_dotted_to_vnum";
    if ( skill == do_down )		return "do_down";
    if ( skill == do_drag )		return "do_drag";
	if ( skill == do_draw )     return "do_draw";
    if ( skill == do_drink )		return "do_drink";
    if ( skill == do_drop )		return "do_drop";
    if ( skill == do_diagnose )		return "do_diagnose";
    if ( skill == do_east )		return "do_east";
    if ( skill == do_eat )		return "do_eat";
    if ( skill == do_echo )		return "do_echo";
    if ( skill == do_editmovementmessage ) return "do_editmovementmessage";
    if ( skill == do_editquest)        	return "do_editquest";
    if ( skill == do_emote )			return "do_emote";
    if ( skill == do_empty )			return "do_empty";
    if ( skill == do_enter )			return "do_enter";
    if ( skill == do_equipment )		return "do_equipment";
    if ( skill == do_evtalk )           return "do_evtalk";
    if ( skill == do_examine )			return "do_examine";
    if ( skill == do_excommunicate )    return "do_excommunicate";
    if ( skill == do_exits )			return "do_exits";
    if ( skill == do_fire )     		return "do_fire";
    if ( skill == do_fireworks )		return "do_fireworks";
    if ( skill == do_feed )				return "do_feed";
    if ( skill == do_fights )           return "do_fights";
    if ( skill == do_fill )				return "do_fill";
    if ( skill == do_fixchar )			return "do_fixchar";
	if ( skill == do_fixme )    		return "do_fixme";
    if ( skill == do_flee )				return "do_flee";
    if ( skill == do_foldarea )			return "do_foldarea";
    if ( skill == do_follow )			return "do_follow";
    if ( skill == do_for )				return "do_for";
	if ( skill == do_forage )			return "do_forage";
    if ( skill == do_force )			return "do_force";
    if ( skill == do_forceclose )		return "do_forceclose";
	if ( skill == do_forge )			return "do_forge";
    if ( skill == do_form_password )	return "do_form_password";
	if ( skill == do_forum_password ) 	return "do_forum_password";
    if ( skill == do_fquit )			return "do_fquit";
    if ( skill == do_freeze )			return "do_freeze";
    if ( skill == do_gain )   			return "do_gain";
    if ( skill == do_get )				return "do_get";
    if ( skill == do_give )				return "do_give";
    if ( skill == do_glance )			return "do_glance";
    if ( skill == do_glory )			return "do_glory";
	if ( skill == do_gold )             return "do_gold";
    if ( skill == do_goto )				return "do_goto";
    if ( skill == do_gouge )			return "do_gouge";
    if ( skill == do_group )			return "do_group";
    if ( skill == do_grub )				return "do_grub";
    if ( skill == do_gtell )			return "do_gtell";
    if ( skill == do_guilds )			return "do_guilds";
    if ( skill == do_guildtalk )		return "do_guildtalk";
    if ( skill == do_hedit )			return "do_hedit";
    if ( skill == do_hell )		return "do_hell";
    if ( skill == do_help )		return "do_help";
    if ( skill == do_hide )		return "do_hide";
    if ( skill == do_hitall )		return "do_hitall";
    if ( skill == do_hlist )		return "do_hlist";
    if ( skill == do_holylight )	return "do_holylight";
    if ( skill == do_homepage )		return "do_homepage";
    if ( skill == do_hset )		return "do_hset";
    if ( skill == do_ide )		return "do_ide";
    if ( skill == do_idea )		return "do_idea";
    if ( skill == do_immortalize )	return "do_immortalize";
    if ( skill == do_immtalk )		return "do_immtalk";
    if ( skill == do_induct )		return "do_induct";
    if ( skill == do_installarea )	return "do_installarea";
    if ( skill == do_instaroom )	return "do_instaroom";
    if ( skill == do_instazone )	return "do_instazone";
    if ( skill == do_inventory )	return "do_inventory";
    if ( skill == do_invis )		return "do_invis";
    if ( skill == do_kick )		return "do_kick";
    if ( skill == do_kill )		return "do_kill";
    if ( skill == do_kindle_fire) return "do_kindle_fire";
	if ( skill == do_knock)         return "do_knock";
	if ( skill == do_ksilyantoy)	return "do_ksilyantoy";
    if ( skill == do_languages )	return "do_languages";
    if ( skill == do_last )		return "do_last";
    if ( skill == do_leave )		return "do_leave";
    if ( skill == do_level )		return "do_level";
    if ( skill == do_light )		return "do_light";
    if ( skill == do_list )		return "do_list";
    if ( skill == do_litterbug )	return "do_litterbug";
    if ( skill == do_loadarea )		return "do_loadarea";
    if ( skill == do_loadup )		return "do_loadup";
    if ( skill == do_lock )		return "do_lock";
    if ( skill == do_log )		return "do_log";
    if ( skill == do_look )		return "do_look";
    if ( skill == do_lookmap )		return "do_lookmap";
    if ( skill == do_low_purge )	return "do_low_purge";
    if ( skill == do_mphunt ) return "do_mphunt";
    if ( skill == do_mpdispel ) return "do_mpdispel";
    if ( skill == do_meditate ) return "do_meditate";
    if ( skill == do_mudconnector ) return "do_mudconnector";
    if ( skill == do_mailroom )		return "do_mailroom";
    if ( skill == do_make )		return "do_make";
    if ( skill == do_makeboard )	return "do_makeboard";
    if ( skill == do_makeclan )		return "do_makeclan";
    if ( skill == do_makecouncil )	return "do_makecouncil";
    if ( skill == do_makedeity )	return "do_makedeity";
    if ( skill == do_makerepair )	return "do_makerepair";
    if ( skill == do_makeshop )		return "do_makeshop";
    if ( skill == do_makewizlist )	return "do_makewizlist";
    if ( skill == do_mapout )		return "do_mapout";
    if ( skill == do_massign )		return "do_massign";
    if ( skill == do_mcreate )		return "do_mcreate";
    if ( skill == do_mdelete )		return "do_mdelete";
    if ( skill == do_memory )		return "do_memory";
    if ( skill == do_mfind )		return "do_mfind";
    if ( skill == do_mine )		return "do_mine";
    if ( skill == do_mload )		return "do_mload";
    if ( skill == do_mlist )		return "do_mlist";
    //if ( skill == do_mmenu )		return "do_mmenu";
	if ( skill == do_movementmessage ) return "do_movementmessage";
    if ( skill == do_mount )		return "do_mount";
    if ( skill == do_mpcontinue ) return "do_mpcontinue";
   if ( skill == do_mpforcelook )       return "do_mpforcelook";
	if ( skill == do_mpwalk )       return "do_mpwalk";
    if ( skill == do_mp_close_passage )	return "do_mp_close_passage";
    if ( skill == do_mp_damage )	return "do_mp_damage";
    if ( skill == do_mp_deposit )	return "do_mp_deposit";
    if ( skill == do_mp_open_passage )	return "do_mp_open_passage";
    if ( skill == do_mp_practice )	return "do_mp_practice";
    if ( skill == do_mp_restore )	return "do_mp_restore";
    if ( skill == do_mp_setthief)		return "do_mp_setthief";
	if ( skill == do_mp_setwanted )		return "do_mp_setwanted";
    if ( skill == do_mp_slay )			return "do_mp_slay";
    if ( skill == do_mp_withdraw )		return "do_mp_withdraw";
    if ( skill == do_mpadvance )		return "do_mpadvance";
    if ( skill == do_mpapply )			return "do_mpapply";
    if ( skill == do_mpapplyb )			return "do_mpapplyb";
    if ( skill == do_mpasound )			return "do_mpasound";
    if ( skill == do_mpat )				return "do_mpat";
    if ( skill == do_mpaward )			return "do_mpaward";
    if ( skill == do_mpbecome ) 		return "do_mpbecome";
    if ( skill == do_mpdream )			return "do_mpdream";
    if ( skill == do_mpecho )			return "do_mpecho";
	if ( skill == do_mpareaecho)   		return "do_mpareaecho";
    if ( skill == do_mpechoaround )		return "do_mpechoaround";
    if ( skill == do_mpechoat )			return "do_mpechoat";
    if ( skill == do_mpedit )			return "do_mpedit";
    if ( skill == do_mpfavor )			return "do_mpfavor";
    if ( skill == do_mpflag )			return "do_mpflag";
    if ( skill == do_mpforce )			return "do_mpforce";
    if ( skill == do_mpglory )			return "do_mpglory";
    if ( skill == do_mpgoto )			return "do_mpgoto";
    if ( skill == do_mpinvis )			return "do_mpinvis";
    if ( skill == do_mpjunk )			return "do_mpjunk";
    if ( skill == do_mpkill )			return "do_mpkill";
    if ( skill == do_mpmload )			return "do_mpmload";
    if ( skill == do_mpmset )			return "do_mpmset";
    if ( skill == do_mpnothing )		return "do_mpnothing";
    if ( skill == do_mpoload )			return "do_mpoload";
    if ( skill == do_mposet )			return "do_mposet";
    if ( skill == do_mppurge )			return "do_mppurge";
    if ( skill == do_mpstat )			return "do_mpstat";
    if ( skill == do_mptransfer )		return "do_mptransfer";
    if ( skill == do_mrange )			return "do_mrange";
	if ( skill == do_mscriptilyan ) 	return "do_mscriptilyan";
    if ( skill == do_mset )				return "do_mset";
    if ( skill == do_mstat )			return "do_mstat";
    if ( skill == do_murde )			return "do_murde";
    if ( skill == do_murder )			return "do_murder";
    if ( skill == do_muse )				return "do_muse";
    if ( skill == do_music )			return "do_music";
    if ( skill == do_mwhere )			return "do_mwhere";
    if ( skill == do_wwho )   			return "do_wwho";
    if ( skill == do_namechange )		return "do_namechange";
    if ( skill == do_new_dump )     	return "do_new_dump";
    if ( skill == do_newbiechat )		return "do_newbiechat";
    if ( skill == do_newbielock )       return "do_newbielock";
    if ( skill == do_newbieset )	return "do_newbieset";
    if ( skill == do_new_inv )          return "do_new_inv";
	if ( skill == do_newwho )       return "do_newwho";
    if ( skill == do_newzones )		return "do_newzones";
    if ( skill == do_noemote )		return "do_noemote";
    if ( skill == do_nohelps )		return "do_nohelps";
    if ( skill == do_noresolve )	return "do_noresolve";
    if ( skill == do_north )		return "do_north";
    if ( skill == do_northeast )	return "do_northeast";
    if ( skill == do_northwest )	return "do_northwest";
    if ( skill == do_notell )		return "do_notell";
    if ( skill == do_notitle )          return "do_notitle";
    if ( skill == do_noteroom )		return "do_noteroom";
    if ( skill == do_ooc )          return "do_ooc";
    if ( skill == do_oassign )		return "do_oassign";
	if ( skill == do_object_csv_dump ) return "do_object_csv_dump";
	if ( skill == do_object_csv_read ) return "do_object_csv_read";
    if ( skill == do_object_backup ) return "do_object_backup";
    if ( skill == do_ocreate )		return "do_ocreate";
    if ( skill == do_odelete )		return "do_odelete";
    if ( skill == do_ofind )		return "do_ofind";
    if ( skill == do_ogrub )		return "do_ogrub";
    if ( skill == do_oload )		return "do_oload";
    if ( skill == do_oldscore )		return "do_oldscore";
    if ( skill == do_olist )		return "do_olist";
    //if ( skill == do_omenu )		return "do_omenu";
    if ( skill == do_opedit )		return "do_opedit";
    if ( skill == do_open )		return "do_open";
/*  if ( skill == do_opentourney )	return "do_opentourney"; */
    if ( skill == do_opstat )		return "do_opstat";
    if ( skill == do_orange )		return "do_orange";
    if ( skill == do_order )		return "do_order";
    if ( skill == do_orders )		return "do_orders";
    if ( skill == do_ordertalk )	return "do_ordertalk";
	if ( skill == do_oreport )      return "do_report";
	if ( skill == do_osay )			return "do_osay";
	if ( skill == do_oscriptilyan)  return "do_oscriptilyan";
    if ( skill == do_oset )		return "do_oset";
    if ( skill == do_ostat )		return "do_ostat";
    if ( skill == do_outcast )		return "do_outcast";
    if ( skill == do_owhere )		return "do_owhere";
    //if ( skill == do_pagelen )		return "do_pagelen";
    if ( skill == do_pager )		return "do_pager";
    if ( skill == do_pardon )		return "do_pardon";
    if ( skill == do_password )		return "do_password";
    if ( skill == do_peace )		return "do_peace";
    if ( skill == do_pick )		return "do_pick";
    if ( skill == do_playbuild )        return "do_playbuild";
    if ( skill == do_poison_weapon )	return "do_poison_weapon";
    if ( skill == do_pray )   return "do_pray";
    if ( skill == do_practice )		return "do_practice";
    if ( skill == do_prompt )		return "do_prompt";
    if ( skill == do_pull )		return "do_pull";
    if ( skill == do_punch )		return "do_punch";
    if ( skill == do_purge )		return "do_purge";
    if ( skill == do_push )		return "do_push";
    if ( skill == do_put )		return "do_put";
    if ( skill == do_qpset )		return "do_qpset";
    if ( skill == do_qset )             return "do_qset";
    if ( skill == do_qstat )            return "do_qstat";
    if ( skill == do_quaff )		return "do_quaff";
    if ( skill == do_qedit )            return "do_qedit";
    if ( skill == do_quest )		return "do_quest";
    if ( skill == do_qui )		return "do_qui";
    if ( skill == do_quit )		return "do_quit";
    if ( skill == do_rarelist ) return "do_rarelist";
    if ( skill == do_rareset ) return "do_rareset";
    if ( skill == do_rfind )    return "do_rfind";
    if ( skill == do_rank )		return "do_rank";
    if ( skill == do_rassign )		return "do_rassign";
    if ( skill == do_rat )		return "do_rat";
    if ( skill == do_rdelete )		return "do_rdelete";
    if ( skill == do_reboo )		return "do_reboo";
    if ( skill == do_reboot )		return "do_reboot";
    if ( skill == do_recho )		return "do_recho";
    if ( skill == do_recite )		return "do_recite";
    if ( skill == do_redit )		return "do_redit";
    //if ( skill == do_redraw_page )	return "do_redraw_page";
    //if ( skill == do_refresh_page )	return "do_refresh_page";
    if ( skill == do_regoto )		return "do_regoto";
    if ( skill == do_remove )		return "do_remove";
    if ( skill == do_renumber )     return "do_renumber";
    if ( skill == do_rent )		return "do_rent";
    if ( skill == do_repair )		return "do_repair";
    if ( skill == do_repairset )	return "do_repairset";
    if ( skill == do_repairshops )	return "do_repairshops";
    if ( skill == do_repairstat )	return "do_repairstat";
    if ( skill == do_reply )		return "do_reply";
    if ( skill == do_report )		return "do_report";
    if ( skill == do_respond )   return "do_respond";
    if ( skill == do_rescue )		return "do_rescue";
    if ( skill == do_reset )		return "do_reset";
    if ( skill == do_rest )		return "do_rest";
    if ( skill == do_restore )		return "do_restore";
    if ( skill == do_restoretime )	return "do_restoretime";
    if ( skill == do_restrict )		return "do_restrict";
    if ( skill == do_retell )           return "do_retell";
    if ( skill == do_retire )		return "do_retire";
    if ( skill == do_retran )		return "do_retran";
    if ( skill == do_return )		return "do_return";
    if ( skill == do_revert )		return "do_revert";
    if ( skill == do_rip )		return "do_rip";
    if ( skill == do_rlist )		return "do_rlist";
    //if ( skill == do_rmenu )		return "do_rmenu";
    if ( skill == do_rpedit )		return "do_rpedit";
    if ( skill == do_rpstat )		return "do_rpstat";
    if ( skill == do_rreset )		return "do_rreset";
	if ( skill == do_rscriptilyan)  return "do_rscriptilyan";
    if ( skill == do_rstat )		return "do_rstat";
    if ( skill == do_sacrifice )	return "do_sacrifice";
    if ( skill == do_spam ) return "do_spam";
    if ( skill == do_save )		return "do_save";
    if ( skill == do_savearea )		return "do_savearea";
    if ( skill == do_say )		return "do_say";
    if ( skill == do_sayto )		return "do_sayto";
    if ( skill == do_scan )		return "do_scan";
    if ( skill == do_scentedit )	return "do_scentedit";
    if ( skill == do_score )		return "do_score";
    if ( skill == do_scribe )		return "do_scribe";
    if ( skill == do_search )		return "do_search";
    if ( skill == do_sedit )		return "do_sedit";
    if ( skill == do_sell )		return "do_sell";
    if ( skill == do_set_boot_time )	return "do_set_boot_time";
    if ( skill == do_setclan )		return "do_setclan";
    if ( skill == do_setclass )		return "do_setclass";
    if ( skill == do_setcouncil )	return "do_setcouncil";
    if ( skill == do_setdeity )		return "do_setdeity";
	if ( skill == do_setgembase )   return "do_setgembase";
    if ( skill == do_sgtalk )           return "do_sgtalk";
	if ( skill == do_sheathe )      return "do_sheathe";
    if ( skill == do_shops )		return "do_shops";
    if ( skill == do_shopset )		return "do_shopset";
    if ( skill == do_shopstat )		return "do_shopstat";
    if ( skill == do_shout )		return "do_shout";
    if ( skill == do_shove )		return "do_shove";
    if ( skill == do_showclan )		return "do_showclan";
    if ( skill == do_showclass )	return "do_showclass";
    if ( skill == do_showcouncil )	return "do_showcouncil";
    if ( skill == do_showdeity )	return "do_showdeity";
    if ( skill == do_showquest )	return "do_showquest";
	if ( skill == do_showvault )    return "do_showvault";
    if ( skill == do_shutdow )		return "do_shutdow";
    if ( skill == do_shutdown )		return "do_shutdown";
    if ( skill == do_silence )		return "do_silence";
    if ( skill == do_sit )		return "do_sit";
    if ( skill == do_sla )		return "do_sla";
    if ( skill == do_slay )		return "do_slay";
    if ( skill == do_sleep )		return "do_sleep";
    if ( skill == do_slice )		return "do_slice";
    if ( skill == do_slist )		return "do_slist";
    if ( skill == do_slookup )		return "do_slookup";
    if ( skill == do_smell )		return "do_smell";
    if ( skill == do_smelt )		return "do_smelt";
    if ( skill == do_smoke )		return "do_smoke";
    if ( skill == do_snoop )		return "do_snoop";
    if ( skill == do_sober )		return "do_sober";
    if ( skill == do_socials )		return "do_socials";
    if ( skill == do_south )		return "do_south";
    if ( skill == do_southeast )	return "do_southeast";
    if ( skill == do_southwest )	return "do_southwest";
	if ( skill == do_spells )       return "do_spells";
    if ( skill == do_speak )		return "do_speak";
    if ( skill == do_split )		return "do_split";
    if ( skill == do_sset )		return "do_sset";

	if ( skill == do_stable )		return "do_stable";
	if ( skill == do_stableset)     return "do_stableset";
	if ( skill == do_stagename)     return "do_stagename";
	if ( skill == do_makestable)    return "do_makestable";
	if ( skill == do_stables)       return "do_stables";
	if ( skill == do_stablestat)    return "do_stablestat";

	if ( skill == do_stand )		return "do_stand";
    if ( skill == do_steal )		return "do_steal";
    if ( skill == do_stun )		return "do_stun";
    if ( skill == do_supplicate )	return "do_supplicate";
    if ( skill == do_switch )		return "do_switch";
    if ( skill == do_throw ) return "do_throw";
    if ( skill == do_test )             return "do_test";
    if ( skill == do_television_set )   return "do_television_set";
    if ( skill == do_trainset )    return "do_trainset";
    if ( skill == do_trainstat ) return "do_trainstat";
    if ( skill == do_tamp )		return "do_tamp";
    if ( skill == do_tell )		return "do_tell";
    if ( skill == do_think )		return "do_think";
    if ( skill == do_time )		return "do_time";
    if ( skill == do_timecmd )		return "do_timecmd";
    if ( skill == do_title )		return "do_title";
    if ( skill == do_titlechange )      return "do_titlechange";
	if ( skill == do_titlelock )    return "do_titlelock";
    if ( skill == do_track )		return "do_track";
    if ( skill == do_transfer )		return "do_transfer";
    if ( skill == do_trust )		return "do_trust";
    if ( skill == do_typo )		return "do_typo";
    if ( skill == do_unfoldarea )	return "do_unfoldarea";
    if ( skill == do_unhell )		return "do_unhell";
    if ( skill == do_unlock )		return "do_unlock";
    if ( skill == do_unsilence )        return "do_unsilence";
    if ( skill == do_up )		return "do_up";
    if ( skill == do_use )		return "do_use";
    if ( skill == do_users )		return "do_users";
    if ( skill == do_value )		return "do_value";
	if ( skill == do_vault )        return "do_vault";
    if ( skill == do_visible )		return "do_visible";
    if ( skill == do_vnums )		return "do_vnums";
	if ( skill == do_vnum_to_dotted ) return "do_vnum_to_dotted";
    if ( skill == do_vsearch )		return "do_vsearch";
    if ( skill == do_wake )		return "do_wake";
    if ( skill == do_wartalk )		return "do_wartalk";
    if ( skill == do_wear )		return "do_wear";
    if ( skill == do_weather )		return "do_weather";
    if ( skill == do_west )		return "do_west";
    if ( skill == do_where )		return "do_where";
	if ( skill == do_whisper )		return "do_whisper";
    if ( skill == do_who )		return "do_who";
    if ( skill == do_whois )		return "do_whois";
    if ( skill == do_wimpy )		return "do_wimpy";
    if ( skill == do_wizhelp )		return "do_wizhelp";
    if ( skill == do_wizlist )		return "do_wizlist";
    if ( skill == do_wizlock )		return "do_wizlock";
	if ( skill == do_worth )        return "do_worth";
    if ( skill == do_yell )		return "do_yell";
    if ( skill == do_zap )		return "do_zap";
    if ( skill == do_zones )		return "do_zones";
    return "reserved";
}

bool load_class_file( const char *fname )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	ClassType *Class;
	int cl = -1;
	int tlev = 0;
	FILE *fp;

	sprintf( buf, "%s%s", CLASS_DIR, fname );
	if ( ( fp = fopen( buf, "r" ) ) == NULL )
	{
		perror( buf );
		return FALSE;
	}

	Class = new ClassType;

	for ( ; ; )
	{
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;

			case 'A':
				KEY( "AttrPrime",	Class->attr_prime,	fread_number( fp )	);
				break;

			case 'C':
				KEY( "Class",	cl,			fread_number( fp )	);
				break;

			case 'E':
				if ( !str_cmp( word, "End" ) )
				{
					fclose( fp );
					if ( cl < 0 || cl >= MAX_CLASS )
					{
						sprintf( buf, "Load_class_file: Class (%s) bad/not found (%d)",
								Class->whoName_.length() > 0 ? Class->whoName_.c_str() : "name not found", cl );
						bug( buf, 0 );
						delete Class;
						return FALSE;
					}
					class_table[cl] = Class;
					return TRUE;
				}
				KEY( "ExpBase",	Class->exp_base,	fread_number( fp )	);
				break;

			case 'G':
				KEY( "Guild",	Class->guild,		fread_number( fp )	);
				break;

			case 'H':
				KEY( "HpMax",	Class->hp_max,		fread_number( fp )	);
				KEY( "HpMin",	Class->hp_min,		fread_number( fp )	);
				break;

			case 'M':
				KEY( "Mana",	Class->fMana,		fread_number( fp )	);
				break;

			case 'N':
				KEY( "Name",	Class->whoName_,	fread_string_noheap( fp )	);
				break;

			case 'S':
				if ( !str_cmp( word, "Skill" ) )
				{
					int sn, lev, adp;

					word = fread_word( fp );
					lev = fread_number( fp );
					adp = fread_number( fp );
					sn = skill_lookup( word );
					if ( cl < 0 || cl >= MAX_CLASS )
					{
						sprintf( buf, "load_class_file: Skill %s -- class bad/not found (%d)", word, cl );
						bug( buf, 0 );
					}
					else
						if ( !IS_VALID_SN(sn) )
						{
							sprintf( buf, "load_class_file: Skill %s unknown", word );
							bug( buf, 0 );
						}
						else
						{
							skill_table[sn]->skill_level[cl] = lev;
							skill_table[sn]->skill_adept[cl] = adp;
						}
					fMatch = TRUE;
					break;
				}
				KEY( "Skilladept",	Class->skill_adept,	fread_number( fp )	);
				break;

			case 'T':
				if ( !str_cmp( word, "Title" ) )
				{
					if ( cl < 0 || cl >= MAX_CLASS )
					{
						char *tmp;

						sprintf( buf, "load_class_file: Title -- class bad/not found (%d)", cl );
						bug( buf, 0 );
						tmp = fread_string_nohash( fp );
						DISPOSE( tmp );
						tmp = fread_string_nohash( fp );
						DISPOSE( tmp );
					}
					else
						if ( tlev < MAX_LEVEL+1 )
						{
							title_table[cl][tlev][0] = fread_string_nohash( fp );
							title_table[cl][tlev][1] = fread_string_nohash( fp );
							++tlev;
						}
						else
							bug( "load_class_file: Too many titles" );
					fMatch = TRUE;
					break;
				}
				KEY( "Thac0",	Class->thac0_00,	fread_number( fp )	);
				KEY( "Thac32",	Class->thac0_32,	fread_number( fp )	);
				break;

			case 'W':
				KEY( "Weapon",	Class->weapon,	fread_number( fp )	);
				break;
		}
		if ( !fMatch )
		{
			sprintf( buf, "load_class_file: no match: %s", word );
			bug( buf, 0 );
		}
	}
	return FALSE;
}

/*
 * Load in all the class files.
 */
void load_classes( )
{
    FILE *fpList;
    const char *filename;
    char classlist[256];
    char buf[MAX_STRING_LENGTH];

    sprintf( classlist, "%s%s", CLASS_DIR, CLASS_LIST );
    if ( ( fpList = fopen( classlist, "r" ) ) == NULL )
    {
	perror( classlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	if ( filename[0] == '$' )
	  break;

	if ( !load_class_file( filename ) )
	{
	  sprintf( buf, "Cannot load class file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    return;
}

void write_class_file( int cl )
{
	FILE *fpout;
	char buf[MAX_STRING_LENGTH];
	char filename[MAX_INPUT_LENGTH];
	ClassType *Class = class_table[cl];
	int x, y;

	sprintf( filename, "%s%s.class", CLASSDIR, Class->whoName_.c_str() );
	if ( (fpout=fopen(filename, "w")) == NULL )
	{
		sprintf( buf, "Cannot open: %s for writing", filename );
		bug( buf, 0 );
		return;
	}
	fprintf( fpout, "Name        %s~\n",	Class->whoName_.c_str()		);
	fprintf( fpout, "Class       %d\n",		cl			);
	fprintf( fpout, "Attrprime   %d\n",		Class->attr_prime	);
	fprintf( fpout, "Weapon      %d\n",		Class->weapon		);
	fprintf( fpout, "Guild       %d\n",		Class->guild		);
	fprintf( fpout, "Skilladept  %d\n",		Class->skill_adept	);
	fprintf( fpout, "Thac0       %d\n",		Class->thac0_00		);
	fprintf( fpout, "Thac32      %d\n",		Class->thac0_32		);
	fprintf( fpout, "Hpmin       %d\n",		Class->hp_min		);
	fprintf( fpout, "Hpmax       %d\n",		Class->hp_max		);
	fprintf( fpout, "Mana        %d\n",		Class->fMana		);
	fprintf( fpout, "Expbase     %d\n",		Class->exp_base		);
	for ( x = 0; x < top_sn; x++ )
	{
		if ( skill_table[x]->name_.length() == 0 )
			break;
		if ( (y=skill_table[x]->skill_level[cl]) < LEVEL_IMMORTAL )
			fprintf( fpout, "Skill '%s' %d %d\n",
					skill_table[x]->name_.c_str(), y, skill_table[x]->skill_adept[cl] );
	}
	for ( x = 0; x <= MAX_LEVEL; x++ )
		fprintf( fpout, "Title\n%s~\n%s~\n",
				title_table[cl][x][0], title_table[cl][x][1] );
	fprintf( fpout, "End\n" );
	fclose( fpout );
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SkillType **sk1, SkillType **sk2 )
{
	SkillType *skill1 = (*sk1);
	SkillType *skill2 = (*sk2);

	if ( !skill1 && skill2 )
		return 1;
	if ( skill1 && !skill2 )
		return -1;
	if ( !skill1 && !skill2 )
		return 0;
	if ( skill1->type < skill2->type )
		return -1;
	if ( skill1->type > skill2->type )
		return 1;
	return strcmp( skill1->name_.c_str(), skill2->name_.c_str() );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table()
{
    log_string( "Sorting skill table..." );
    qsort( &skill_table[1], top_sn-1, sizeof( SkillType * ),
		(int(*)(const void *, const void *)) skill_comp );
}


/*
 * Write skill data to a file
 */
void fwrite_skill( FILE *fpout, SkillType *skill )
{
	SMAUG_AFF *aff;

	fprintf( fpout, "Name         %s~\n",	skill->name_.c_str()	);
	fprintf( fpout, "Type         %s\n",	skill_tname[skill->type]);
	fprintf( fpout, "Flags        %d\n",	skill->flags	);
	if ( skill->target )
		fprintf( fpout, "Target       %d\n",	skill->target	);
	if ( skill->minimum_position )
		fprintf( fpout, "Minpos       %d\n",	skill->minimum_position );
	if ( skill->slot )
		fprintf( fpout, "Slot         %d\n",	skill->slot	);
	if ( skill->min_mana )
		fprintf( fpout, "Mana         %d\n",	skill->min_mana );
	if ( skill->beats )
		fprintf( fpout, "Rounds       %d\n",	skill->beats	);
	if ( skill->guild != -1 )
		fprintf( fpout, "Guild        %d\n",	skill->guild	);

	if ( skill->skill_fun )
		fprintf( fpout, "Code         %s\n",	skill_name(skill->skill_fun) );
	else if ( skill->spell_fun )
			fprintf( fpout, "Code         %s\n",	spell_name(skill->spell_fun) );

	fprintf( fpout, "Dammsg       %s~\n",	skill->nounDamage_.c_str() );
	if ( skill->msgOff_.length() > 0 )
		fprintf( fpout, "Wearoff      %s~\n",	skill->msgOff_.c_str()	);

	if ( skill->hit_char && skill->hit_char[0] != '\0' )
		fprintf( fpout, "Hitchar      %s~\n",	skill->hit_char );
	if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
		fprintf( fpout, "Hitvict      %s~\n",	skill->hit_vict );
	if ( skill->hit_room && skill->hit_room[0] != '\0' )
		fprintf( fpout, "Hitroom      %s~\n",	skill->hit_room );

	if ( skill->miss_char && skill->miss_char[0] != '\0' )
		fprintf( fpout, "Misschar     %s~\n",	skill->miss_char );
	if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
		fprintf( fpout, "Missvict     %s~\n",	skill->miss_vict );
	if ( skill->miss_room && skill->miss_room[0] != '\0' )
		fprintf( fpout, "Missroom     %s~\n",	skill->miss_room );

	if ( skill->die_char && skill->die_char[0] != '\0' )
		fprintf( fpout, "Diechar      %s~\n",	skill->die_char );
	if ( skill->die_vict && skill->die_vict[0] != '\0' )
		fprintf( fpout, "Dievict      %s~\n",	skill->die_vict );
	if ( skill->die_room && skill->die_room[0] != '\0' )
		fprintf( fpout, "Dieroom      %s~\n",	skill->die_room );

	if ( skill->imm_char && skill->imm_char[0] != '\0' )
		fprintf( fpout, "Immchar      %s~\n",	skill->imm_char );
	if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
		fprintf( fpout, "Immvict      %s~\n",	skill->imm_vict );
	if ( skill->imm_room && skill->imm_room[0] != '\0' )
		fprintf( fpout, "Immroom      %s~\n",	skill->imm_room );

	if ( skill->dice && skill->dice[0] != '\0' )
		fprintf( fpout, "Dice         %s~\n",	skill->dice );
	if ( skill->value )
		fprintf( fpout, "Value        %d\n",	skill->value );
	if ( skill->difficulty )
		fprintf( fpout, "Difficulty   %d\n",	skill->difficulty );
	if ( skill->participants )
		fprintf( fpout, "Participants %d\n",	skill->participants );
	if ( skill->Prerequisites )
		fprintf( fpout, "Prerequisites %s~\n", skill->Prerequisites );
	if ( skill->components && skill->components[0] != '\0' )
		fprintf( fpout, "Components   %s~\n",	skill->components );
	if ( skill->teachers && skill->teachers[0] != '\0' )
		fprintf( fpout, "Teachers     %s~\n",	skill->teachers );
	for ( aff = skill->affects; aff; aff = aff->next )
		fprintf( fpout, "Affect       '%s' %d '%s' %d\n",
				aff->duration, aff->location, aff->modifier, aff->bitvector );

	if ( skill->type != SKILL_HERB )
	{
		int y;
		int min = 1000;

		for ( y = 0; y < MAX_CLASS; y++ )
			if ( skill->skill_level[y] < min )
				min = skill->skill_level[y];

		fprintf( fpout, "Minlevel     %d\n",	min		);
	}
	fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table()
{
	int x;
	FILE *fpout;

	if ( (fpout=fopen( SKILL_FILE, "w" )) == NULL )
	{
		bug( "Cannot open skills.dat for writting", 0 );
		perror( SKILL_FILE );
		return;
	}

	for ( x = 0; x < top_sn; x++ )
	{
		if ( skill_table[x]->name_.length() == 0 )
			break;
		fprintf( fpout, "#SKILL\n" );
		fwrite_skill( fpout, skill_table[x] );
	}
	fprintf( fpout, "#END\n" );
	fclose( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table()
{
	int x;
	FILE *fpout;

	if ( (fpout=fopen( HERB_FILE, "w" )) == NULL )
	{
		bug( "Cannot open herbs.dat for writting", 0 );
		perror( HERB_FILE );
		return;
	}

	for ( x = 0; x < top_herb; x++ )
	{
		if ( herb_table[x]->name_.length() == 0 )
			break;
		fprintf( fpout, "#HERB\n" );
		fwrite_skill( fpout, herb_table[x] );
	}
	fprintf( fpout, "#END\n" );
	fclose( fpout );
}

int get_skill( const char *skilltype )
{
    if ( !str_cmp( skilltype, "Spell" ) )
      return SKILL_SPELL;
    if ( !str_cmp( skilltype, "Skill" ) )
      return SKILL_SKILL;
    if ( !str_cmp( skilltype, "Weapon" ) )
      return SKILL_WEAPON;
    if ( !str_cmp( skilltype, "Tongue" ) )
      return SKILL_TONGUE;
    if ( !str_cmp( skilltype, "Herb" ) )
      return SKILL_HERB;
    return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 */
void save_commands()
{
    FILE *fpout;
    CMDTYPE *command;
    int x;

    if ( (fpout=fopen( COMMAND_FILE, "w" )) == NULL )
    {
	bug( "Cannot open commands.dat for writing", 0 );
	perror( COMMAND_FILE );
	return;
    }

    for ( x = 0; x < 126; x++ )
    {
	for ( command = command_hash[x]; command; command = command->next )
	{
	    if ( !command->name || command->name[0] == '\0' )
	    {
		bug( "Save_commands: blank command in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#COMMAND\n" );
	    fprintf( fpout, "Name        %s~\n", command->name		);
	    fprintf( fpout, "Code        %s\n",	 skill_name(command->do_fun) );
	    fprintf( fpout, "Position    %d\n",	 command->position	);
	    fprintf( fpout, "Level       %d\n",	 command->level		);
	    fprintf( fpout, "Log         %d\n",	 command->log		);
	    fprintf( fpout, "End\n\n" );
	}
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}

SkillType *fread_skill( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;
	SkillType *skill;

	skill = new SkillType;

	for ( ; ; )
	{
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;

			case 'A':
				if ( !str_cmp( word, "Affect" ) )
				{
					SMAUG_AFF *aff;

					CREATE( aff, SMAUG_AFF, 1 );
					aff->duration = str_dup( fread_word( fp ) );
					aff->location = fread_number( fp );
					aff->modifier = str_dup( fread_word( fp ) );
					aff->bitvector = fread_number( fp );
					aff->next = skill->affects;
					skill->affects = aff;
					fMatch = TRUE;
					break;
				}
				break;

			case 'C':
				if ( !str_cmp( word, "Class" ) )
				{
					int Class = fread_number( fp );

					skill->skill_level[Class] = fread_number( fp );
					skill->skill_adept[Class] = fread_number( fp );
					fMatch = TRUE;
					break;
				}
				if ( !str_cmp( word, "Code" ) )
				{
					SPELL_FUN *spellfun;
					DO_FUN	  *dofun;
					char	  *w = fread_word(fp);

					fMatch = TRUE;
					if ( (spellfun=spell_function(w)) != spell_notfound )
						skill->spell_fun = spellfun;
					else
						if ( (dofun=skill_function(w)) != skill_notfound )
							skill->skill_fun = dofun;
						else
						{
							sprintf( buf, "fread_skill: unknown skill/spell %s", w );
							bug( buf, 0 );
							skill->spell_fun = spell_null;
						}
					break;
				}
				KEY( "Code",	skill->spell_fun, spell_function(fread_word(fp)) );
				KEY( "Components",	skill->components,	fread_string_nohash( fp ) );
				break;

			case 'D':
				KEY( "Dammsg",	skill->nounDamage_,	fread_string_noheap( fp ) );
				KEY( "Dice",	skill->dice,		fread_string_nohash( fp ) );
				KEY( "Diechar",	skill->die_char,	fread_string_nohash( fp ) );
				KEY( "Dieroom",	skill->die_room,	fread_string_nohash( fp ) );
				KEY( "Dievict",	skill->die_vict,	fread_string_nohash( fp ) );
				KEY( "Difficulty",	skill->difficulty,	fread_number( fp ) );
				break;

			case 'E':
				if ( !str_cmp( word, "End" ) )
					return skill;
				break;

			case 'F':
				KEY( "Flags",	skill->flags,		fread_number( fp ) );
				break;

			case 'G':
				KEY( "Guild",	skill->guild,		fread_number( fp ) );
				break;

			case 'H':
				KEY( "Hitchar",	skill->hit_char,	fread_string_nohash( fp ) );
				KEY( "Hitroom",	skill->hit_room,	fread_string_nohash( fp ) );
				KEY( "Hitvict",	skill->hit_vict,	fread_string_nohash( fp ) );
				break;

			case 'I':
				KEY( "Immchar",	skill->imm_char,	fread_string_nohash( fp ) );
				KEY( "Immroom",	skill->imm_room,	fread_string_nohash( fp ) );
				KEY( "Immvict",	skill->imm_vict,	fread_string_nohash( fp ) );
				break;

			case 'M':
				KEY( "Mana",	skill->min_mana,	fread_number( fp ) );
				if ( !str_cmp( word, "Minlevel" ) )
				{
					fread_to_eol( fp );
					fMatch = TRUE;
					break;
				}
				KEY( "Minpos",	skill->minimum_position, fread_number( fp ) );
				KEY( "Misschar",	skill->miss_char,	fread_string_nohash( fp ) );
				KEY( "Missroom",	skill->miss_room,	fread_string_nohash( fp ) );
				KEY( "Missvict",	skill->miss_vict,	fread_string_nohash( fp ) );
				break;

			case 'N':
				KEY( "Name",	skill->name_,		fread_string_noheap( fp ) );
				break;

			case 'P':
				KEY( "Participants",skill->participants,	fread_number( fp ) );
				KEY( "Prerequisites", skill->Prerequisites, fread_string_nohash( fp ) );
				break;

			case 'R':
				KEY( "Rounds",	skill->beats,		fread_number( fp ) );
				break;

			case 'S':
				KEY( "Slot",	skill->slot,		fread_number( fp ) );
				KEY( "Saves",	skill->saves,		fread_number( fp ) );
				break;

			case 'T':
				KEY( "Target",	skill->target,		fread_number( fp ) );
				KEY( "Teachers",	skill->teachers,	fread_string_nohash( fp ) );
				KEY( "Type",	skill->type,  get_skill(fread_word( fp ))  );
				break;

			case 'V':
				KEY( "Value",	skill->value,		fread_number( fp ) );
				break;

			case 'W':
				KEY( "Wearoff",	skill->msgOff_,		fread_string_noheap( fp ) );
				break;
		}

		if ( !fMatch )
		{
			sprintf( buf, "Fread_skill: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

void load_skill_table()
{
    FILE *fp;

    if ( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
                bug( "Load_skill_table: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "SKILL"      ) )
	    {
		if ( top_sn >= MAX_SKILL )
		{
		    bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
		    fclose( fp );
		    return;
		}
		skill_table[top_sn++] = fread_skill( fp );
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_skill_table: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open skills.dat", 0 );
 	exit(1);
    }
}

void load_herb_table()
{
    FILE *fp;

    if ( ( fp = fopen( HERB_FILE, "r" ) ) != NULL )
    {
	top_herb = 0;
	for ( ;; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_herb_table: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "HERB"      ) )
	    {
		if ( top_herb >= MAX_HERB )
		{
		    bug( "load_herb_table: more herbs than MAX_HERB %d", MAX_HERB );
		    fclose( fp );
		    return;
		}
		herb_table[top_herb++] = fread_skill( fp );
		if ( herb_table[top_herb-1]->slot == 0 )
		    herb_table[top_herb-1]->slot = top_herb-1;
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_herb_table: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open herbs.dat", 0 );
 	exit(1);
    }
}

void fread_command( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    const char *word;
    bool fMatch;
    CMDTYPE *command;

    CREATE( command, CMDTYPE, 1 );

    for ( ;; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    KEY( "Code",	command->do_fun,	skill_function(fread_word(fp)) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !command->name )
		{
		    bug( "Fread_command: Name not found", 0 );
		    free_command( command );
		    return;
		}
		if ( !command->do_fun )
		{
		    bug( "Fread_command: Function not found", 0 );
		    free_command( command );
		    return;
		}
		add_command( command );
		return;
	    }
	    break;

	case 'L':
	    KEY( "Level",	command->level,		fread_number(fp) );
	    KEY( "Log",		command->log,		fread_number(fp) );
	    break;

	case 'N':
	    KEY( "Name",	command->name,		fread_string_nohash(fp) );
	    break;

	case 'P':
	    KEY( "Position",	command->position,	fread_number(fp) );
	    break;
	}

	if ( !fMatch )
	{
            sprintf( buf, "Fread_command: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_commands()
{
    FILE *fp;

    if ( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
                bug( "Load_commands: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "COMMAND"      ) )
	    {
                fread_command( fp );
	    	continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_commands: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open commands.dat", 0 );
 	exit(1);
    }

}

void save_classes()
{
    int x;

    for ( x = 0; x < MAX_CLASS; x++ )
      write_class_file( x );
}

/* Next three functions used by idea/typo/bugs code - Zoie */

void copy_files_contents(FILE *fsource, FILE *fdestination)
{
   int ch;
   int cnt = 1;

   for (;;)
   {
      ch = fgetc( fsource );
      if (!feof(fsource))
      {
          fputc( ch, fdestination);
          if (ch == '\n')
          {
             cnt++;
             if (cnt >= 500) //file can't be longer than 500 lines - Zoie
                break;
          }
      }
      else
          break;
   }
}

void write_last_file(char *entry, int file)
{
   	FILE *fpout;
   	FILE *fptemp;
   	char filename[MAX_INPUT_LENGTH];
   	char tempname[MAX_INPUT_LENGTH];

   	if( file == 0 )
      	sprintf(filename, "%s", LAST_LIST);
   	if( file == 1 )
      	sprintf(filename, "%s", PBUG_FILE);
   	if( file == 2 )
      	sprintf(filename, "%s", IDEA_FILE);
   	if( file == 3 )
      	sprintf(filename, "%s", TYPO_FILE);

   	sprintf(tempname, "%s", LAST_TEMP_LIST);

   	if ((fptemp = fopen(tempname, "w")) == NULL)
   	{
    	bug("Cannot open: %s for writing", tempname);
      	return;
   	}
   	fprintf(fptemp, "%s\n", entry); //adds new entry to top of the file
   	if ((fpout = fopen(filename, "r")) != NULL)
   	{
      	copy_files_contents(fpout, fptemp); //copy the rest to the file
      	fclose(fpout); //close the files since writing is done
   	}
   	fclose(fptemp);

   	if (remove(filename) != 0 && fopen(filename, "r") != NULL)
   	{
      	bug("Do not have permission to delete the %s file", filename);
      	return;
   	}
   	if (rename(tempname, filename) != 0)
   	{
      	bug("Do not have permission to rename the %s file", tempname);
      	return;
   	}
   	return;
}

void read_last_file(CHAR_DATA *ch, int count, char *name, int file)
{
   FILE *fpout;
   char filename[MAX_INPUT_LENGTH];
   char charname[100];
   int cnt = 0;
   int letter = 0;
   char *ln;
   char *c;
   char d, e;
   struct tm *tme;
   time_t now;
   char day[MAX_INPUT_LENGTH];
   char sday[5];
   int fnd = 0;

   if( file == 0 )
      sprintf(filename, "%s", LAST_LIST);
   if( file == 1 )
      sprintf(filename, "%s", PBUG_FILE);
   if( file == 2 )
      sprintf(filename, "%s", IDEA_FILE);
   if( file == 3 )
      sprintf(filename, "%s", TYPO_FILE);

   if ((fpout = fopen(filename, "r")) == NULL)
   {
      send_to_char("There is no file to look at.\n\r", ch);
      return;
   }

   for (;;)
   {
      if (feof(fpout))
      {
         fclose(fpout);
         ch_printf(ch, "---------------------------------------------------------------------------\n\r%d Entries Listed.\n\r", cnt);
         return;
      }
      else
      {
         if (count == -2 || ++cnt <= count || count == -1)
         {
            ln = fread_line(fpout);
            strcpy(charname, "");
            if (name) //looking for a certain name
            {
               c = ln;
               for (;;)
               {
                  if (isalpha(*c) && !isspace(*c))
                  {
                     charname[letter] = *c;
                     letter++;
                     c++;
                  }
                  else
                  {
                     charname[letter] = '\0';
                     if (!str_cmp(charname, name))
                     {
                        ch_printf(ch, "%s", ln);
                        letter = 0;
                        strcpy(charname, "");
                        break;
                     }
                     else
                     {
                        if (!feof(fpout))
                        {
                           fread_line(fpout);
                           c = ln;
                           letter = 0;
                           strcpy(charname, "");
                           continue;
                        }
                        else
                        {
                           cnt--;
                           break;
                        }
                     }
                  }
               }
            }
            else if (count == -2) //only today's entries
            {
               c = ln;
               now = time(0);
               tme = localtime(&now);
               strftime(day, 10, "%d", tme);
               for (;;)
               {
                  if (!isdigit(*c))
                  {
                     c++;
                  }
                  else
                  {
                     d = *c;
                     c++;
                     e = *c;
                     sprintf(sday, "%c%c", d, e);
                     if (!str_cmp(sday, day))
                     {
                         fnd = 1;
                         cnt++;
                         ch_printf(ch, "%s", ln);
                         break;
                     }
                     else
                     {
                        if (fnd == 1)
                        {
                           fclose(fpout);
                           ch_printf(ch, "---------------------------------------------------------------------------\n\r%d Entries Listed.\n\r", cnt);
                           return;
                        }
                        else
                           break;
                     }
                  }
               }
            }
            else
            {
               ch_printf(ch, "%s", ln);
            }

         }
         else
         {
            fclose(fpout);
            ch_printf(ch, "--------------------------------------------------------------------------\n\r%d Entries Listed.\n\r", count);
            return;
         }
      }
   }
}


