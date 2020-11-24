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
 *		       Online Building and Editing Module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <crypt.h>
#include "mud.h"
#include "connection.h"
#include "World.h"
#include "commands.h"
#include "object.h"

#include "paths.const.h"

#include "ScentController.h"

#define AREA_VERSION 9

extern int	top_affect;
extern int	top_reset;
extern int	top_ed;
extern bool	fBootDb;


const char *  const   ex_flags [] =
{
"isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
"dig", /*"r1"*/"nomobwander", "nopassdoor", "hidden", "passage", "portal",
/*"r2"*/"barricade", "r3",
"can_climb", "can_enter", "can_leave", "auto", "r4", "searchable",
"bashed", "bashproof", "nomob", "window", "can_look" };

const char *	const	r_flags	[] =
{
"dark", "death", "nomob", "indoors", "lawful", "neutral", "chaotic",
"nomagic", "tunnel", "private", "safe", "solitary", "petshop", "norecall",
"donation", "nodropall", "silence", "logspeech", "nodrop", "clanstoreroom",
"nosummon", "noastral", "teleport", "teleshowdesc", "nofloor", "storeitems",
"nuke", "onepconly", "auctionroom", "forge", "prototype", "always_light"
};

const char *	const	o_flags	[] =
{
"glow", "hum", "dark", "loyal", "evil", "invis", "magic", "nodrop", "bless",
"antigood", "antievil", "antineutral", "noremove", "inventory",
"antimage", "antithief", "antiwarrior", "anticleric", "organic", "metal",
"donation", "clanobject", "clancorpse", "antivampire", "antidruid",
"hidden", "poisoned", "covering", "deathrot", "buried", "prototype", "deity",
"invisible_to_players"
};

const char *	const	o_flags_2 [] =
{
"invisible_to_players","component_container","communication_device","never_breaks",
"no_locate","rare","r6","r7","r8","r9","r10",
"r11","r12","r13","r14","r15","r16","r17","r18","r19","r20","r21","r22",
"r23","r24","r25","r26","r27","r28","r29","r30","r31"
};

const char *	const	mag_flags	[] =
{
"returning", "backstabber", "bane", "loyal", "haste", "drain",
"lightning_blade"
};

const char *	const	w_flags	[] =
{
"take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
"shield", "about", "waist", "wrist", "wield", "hold", "_dual_", "ears", "cape",
"_missile_", "shoulders","quiver","scabbard","r4","r5","r6",
"r7","r8","r9","r10","r11","r12","r13"
};

const char *	const	area_flags	[] =
{
"r0", "notinlist", "noplayerloot", "r3", "r4", "r5", "r6", "r7", "r8",
"r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
"r18", "r19","r20","r21","r22","r23","r24",
"r25","r26","r27","r28","r29","r30","r31"
};

/* Ksilyan
	This is obsolete. The names are all in translate.c instead.

char *	const	o_types	[] =
{
"none", "light", "scroll", "wand", "staff", "weapon",
"treasure", "armor", "potion", "_worn", "furniture", "trash", "_oldtrap",
"container", "_note", "drinkcon", "key", "food", "money", "pen", "boat",
"corpse", "corpse_pc", "fountain", "pill", "blood", "bloodstain",
"scraps", "pipe", "herbcon", "herb", "incense", "fire", "book", "switch",
"lever", "pullchain", "button", "dial", "component", "match", "trap",
"map", "portal", "paper", "tinder", "lockpick", "spike", "disease", "oil",
"fuel", "projectile", "quiver", "scabbard", "shovel",
"salve", "meat"
};

*/

const char *	const	a_types	[] =
{
"none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
"sex", "class", "level", "age", "height", "weight", "mana", "hit", "move",
"gold", "experience", "armor", "hitroll", "damroll", "save_poison", "save_rod",
"save_para", "save_breath", "save_spell", "charisma", "affected", "resistant",
"immune", "susceptible", "weaponspell", "luck", "backstab", "pick", "track",
"steal", "sneak", "hide", "palm", "detrap", "dodge", "peek", "scan", "gouge",
"search", "mount", "disarm", "kick", "parry", "bash", "stun", "punch", "climb",
"grip", "scribe", "brew", "wearspell", "removespell", "emotion", "mentalstate",
"stripsn", "remove", "dig", "full", "thirst", "drunk", "blood", "contagion",
};

const char *	const	a_flags [] =
{
"blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
"detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
"_flaming", "poison", "protect", "_paralysis", "sneak", "hide", "sleep",
"charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
"scrying", "fireshield", "shockshield", "r1", "iceshield", "possess",
"berserk", "aqua_breath" };

const char *	const	act_flags [] =
{
"npc", "sentinel", "scavenger", "banker", "staysector", "aggressive", "stayarea",
"wimpy", "pet", "train", "practice", "immortal", "invisible_to_players", "polyself",
"safe", "guardian", "running", "nowander", "mountable", "mounted", "scholar",
"secretive", "polymorphed", "mobinvis", "noassist", "suv", "noastral", "noregen", "gemdealer",
"nogemdrop", "prototype", "inanimate" };

const char *	const	pc_flags [] =
{
"r1", "deadly", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
"r2", "r3", "notitled", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
"r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
"r25"
};

const char *	const	plr_flags [] =
{
"npc", "boughtpet", "shovedrag", "autoexits", "autoloot", "autosac", "blank",
"outcast", "brief", "combine", "prompt", "telnet_ga", "holylight",
"wizinvis", "roomvnum","silence", "noemote", "attacker", "notell", "log",
"deny", "freeze", "tickmsg","killer", "litterbug", "ansi", "rip", "nice",
"flee" ,"autogold", "automap", "afk"
};

const char *	const	trap_flags [] =
{
"room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
"unlock", "north", "south", "east", "r1", "west", "up", "down", "examine",
"r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
"r14", "r15"
};

const char *	const	wear_locs [] =
{
"light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
"feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
"wield", "hold", "dual_wield", "ears", "eyes", "missile_wield", "shoulders",
"r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","r13"
};

const char *	const	ris_flags [] =
{
"fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
"poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
"plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "r1", "r2", "r3",
"r4", "r5", "r6", "r7", "r8", "r9", "r10"
};

const char *	const	trig_flags [] =
{
"up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
"d_down", "door", "container", "open", "close", "passage", "oload", "mload",
"teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
"rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
"showroomdesc", "autoreturn", "r2", "r3"
};

const char *	const	part_flags [] =
{
"head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
"ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
"tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
"sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
"r1", "r2"
};

const char *	const	attack_flags [] =
{
"bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
"gouge", "backstab", "feed", "drain", "firebreath", "frostbreath",
"acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
"blindness", "causeserious", "earthquake", "causecritical", "curse",
"flamestrike", "harm", "fireball", "colorspray", "weaken", "r1"
};

const char *	const	defense_flags [] =
{
"parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
"dispelmagic", "dispelevil", "sanctuary", "fireshield", "shockshield",
"shield", "bless", "stoneskin", "teleport", "monsum1", "monsum2", "monsum3",
"monsum4", "disarm", "iceshield", "grip", "r3", "r4", "r5", "r6", "r7",
"r8", "r9", "r10", "r11", "r12"
};

/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 *
 */
const char *	const	mprog_flags [] =
{
"act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
"allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
"look", "exa", "zap", "get", "drop", "damage", "repair", "randiw",
"speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use",
"command", "greetdir", "yell", "mpwalkfin", "steal", "otherdeath", "search",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", ""
/* didn't feel like counting */
};

const char * const city_names [] =
{
"Windy Bluff", "Phatep", "Aina", "Eria", "Kettin", "Varin", "Elsin"
};


const char *flag_string( int bitvector, const char * const flagarray[] )
{
    static char buf[MAX_STRING_LENGTH];
    int x;

    buf[0] = '\0';
    for ( x = 0; x < 32 ; x++ )
      if ( IS_SET( bitvector, 1 << x ) )
      {
	strcat( buf, flagarray[x] );
	strcat( buf, " " );
      }
    if ( (x=strlen( buf )) > 0 )
      buf[--x] = '\0';

    return buf;
}


bool can_rmodify( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
	int vnum = room->vnum;
	AREA_DATA *pArea;

	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( get_trust( ch ) >= sysdata.level_modify_proto )
	  return TRUE;
    if ( is_name(room->area->filename, ch->pcdata->bestowments_.c_str()) )
        return TRUE;
	if ( !IS_SET( room->room_flags, ROOM_PROTOTYPE) )
	{
	  send_to_char( "You cannot modify this room.\r\n", ch );
	  return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to modify this room.\r\n", ch );
	  return FALSE;
	}
	if ( vnum >= pArea->low_r_vnum
	&&   vnum <= pArea->hi_r_vnum )
	  return TRUE;

	send_to_char( "That room is not in your allocated range.\r\n", ch );
	return FALSE;
}

bool is_bestowed_mob(CHAR_DATA *ch, int vnum )
{
    AREA_DATA *tarea;

    for ( tarea = first_area; tarea; tarea = tarea->next ) {
        if ( vnum >= tarea->low_m_vnum && vnum <= tarea->hi_m_vnum ) {
            if ( is_name(tarea->filename, ch->pcdata->bestowments_.c_str()) ) {
                return TRUE;
            }
        }
    }
    for ( tarea = first_build; tarea; tarea = tarea->next ) {
        if ( vnum >= tarea->low_m_vnum && vnum <= tarea->hi_m_vnum ) {
            if ( is_name(tarea->filename, ch->pcdata->bestowments_.c_str()) )
                return TRUE;
        }
    }

    return FALSE;
}

bool is_bestowed_obj(CHAR_DATA *ch, int vnum )
{
    AREA_DATA *tarea;

    for ( tarea = first_area; tarea; tarea = tarea->next ) {
        if ( vnum >= tarea->low_o_vnum && vnum <= tarea->hi_o_vnum ) {
            if ( is_name(tarea->filename, ch->pcdata->bestowments_.c_str()) )
               return TRUE;
        }
    }
    for ( tarea = first_build; tarea; tarea = tarea->next ) {
        if ( vnum >= tarea->low_o_vnum && vnum <= tarea->hi_o_vnum ) {
            if ( is_name(tarea->filename, ch->pcdata->bestowments_.c_str()) )
                return TRUE;
        }
    }

    return FALSE;
}


bool can_omodify( CHAR_DATA *ch, OBJ_DATA *obj )
{
	int vnum = obj->pIndexData->vnum;
	AREA_DATA *pArea;

	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( get_trust( ch ) >= sysdata.level_modify_proto )
	  return TRUE;
/*
    if ( is_bestowed_obj(ch, obj->pIndexData->vnum) )
        return TRUE;
*/
	if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
	{
	  send_to_char( "You cannot modify this object.\r\n", ch );
	  return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to modify this object.\r\n", ch );
	  return FALSE;
	}
	if ( vnum >= pArea->low_o_vnum
	&&   vnum <= pArea->hi_o_vnum )
	  return TRUE;

	send_to_char( "That object is not in your allocated range.\r\n", ch );
	return FALSE;
}

bool can_oedit( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
	int vnum = obj->vnum;
	AREA_DATA *pArea;

	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( get_trust( ch ) >= LEVEL_STONE_ADEPT )
	  return TRUE;
    if ( is_bestowed_obj(ch, vnum) )
        return TRUE;
	if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
	{
	  send_to_char( "You cannot modify this object.\r\n", ch );
	  return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to modify this object.\r\n", ch );
	  return FALSE;
	}
	if ( vnum >= pArea->low_o_vnum
	&&   vnum <= pArea->hi_o_vnum )
	  return TRUE;

	send_to_char( "That object is not in your allocated range.\r\n", ch );
	return FALSE;
}


int playerbuildercannot(CHAR_DATA *ch)
{
  if( get_trust(ch) < LEVEL_ENGINEER )
  {
    send_to_char("You are not allowed to do that.\r\n",ch);
    return(TRUE);
  }
  return(FALSE);
}


bool can_mmodify( CHAR_DATA *ch, CHAR_DATA *mob )
{
	int vnum;
	AREA_DATA *pArea;

	if ( mob == ch )
	  return TRUE;

	if ( !IS_NPC( mob ) )
	{
           if(playerbuildercannot(ch))
             return FALSE;
	   if ( get_trust( ch ) >= sysdata.level_modify_proto
              && get_trust(ch) > get_trust( mob ) )
	     return TRUE;
	   else
	     send_to_char( "You can't do that.\r\n", ch );
	     return FALSE;
	}

	vnum = mob->pIndexData->vnum;

	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( get_trust( ch ) >= sysdata.level_modify_proto )
	  return TRUE;
#if 0
        if ( is_bestowed_mob(ch, vnum) )
        return TRUE;
#endif
	if ( !IS_SET( mob->act, ACT_PROTOTYPE) )
	{
	  send_to_char( "You cannot modify this mobile.\r\n", ch );
	  return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to modify this mobile.\r\n", ch );
	  return FALSE;
	}
	if ( vnum >= pArea->low_m_vnum
	&&   vnum <= pArea->hi_m_vnum )
	  return TRUE;

	send_to_char( "That mobile is not in your allocated range.\r\n", ch );
	return FALSE;
}

bool can_medit( CHAR_DATA *ch, MOB_INDEX_DATA *mob )
{
	int vnum = mob->vnum;
	AREA_DATA *pArea;

	if ( IS_NPC( ch ) )
	  return FALSE;
	if ( get_trust( ch ) >= LEVEL_STONE_ADEPT )
	  return TRUE;
    if ( is_bestowed_mob(ch, vnum) )
        return TRUE;
	if ( !IS_SET( mob->act, ACT_PROTOTYPE) )
	{
	  send_to_char( "You cannot modify this mobile.\r\n", ch );
	  return FALSE;
	}
	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to modify this mobile.\r\n", ch );
	  return FALSE;
	}
	if ( vnum >= pArea->low_m_vnum
	&&   vnum <= pArea->hi_m_vnum )
	  return TRUE;

	send_to_char( "That mobile is not in your allocated range.\r\n", ch );
	return FALSE;
}


/* KSILYAN
	OBSOLETE- see itemtype_name_to_number instead!

int get_otype( char *type )
{
    int x;

    for ( x = 0; x < (sizeof(o_types) / sizeof(o_types[0]) ); x++ )
      if ( !str_prefix( type, o_types[x] ) )
        return x;
    return -1;
}
*/

int get_aflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, a_flags[x] ) )
        return x;
    return -1;
}

int get_trapflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, trap_flags[x] ) )
        return x;
    return -1;
}

int get_atype( const char *type )
{
    int x;

    for ( x = 0; x < MAX_APPLY_TYPE; x++ )
      if ( !str_prefix( type, a_types[x] ) )
        return x;
    return -1;
}

int get_npc_race( const char *type )
{
    int x;

    for ( x = 0; x < MAX_NPC_RACE; x++ )
      if ( !str_prefix( type, npc_race[x] ) )
        return x;
    return -1;
}

int get_wearloc( const char *type )
{
    int x;

    for ( x = 0; x < MAX_WEAR; x++ )
      if ( !str_prefix( type, wear_locs[x] ) )
        return x;
    return -1;
}

int get_exflag( const char *flag )
{
    int x;

    for ( x = 0; x <= MAX_EXFLAG; x++ )
      if ( !str_prefix( flag, ex_flags[x] ) )
        return x;
    return -1;
}

int get_rflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, r_flags[x] ) )
        return x;
    return -1;
}

int get_mpflag( const char *flag )
{
    int x;

    for ( x = 0; x < 64; x++ )
      if ( !str_prefix( flag, mprog_flags[x] ) )
        return x;
    return -1;
}

int get_oflag( const char *flag )
{
    int x;

    for ( x = 0; x <= MAX_O_FLAGS; x++ )
      if ( !str_prefix( flag, o_flags[x] ) )
        return x;
    return -1;
}
int get_oflag_2( const char *flag )
{
	int x;
	for ( x = 0; x <= MAX_O_FLAGS_2; x++)
		if ( !str_prefix(flag, o_flags_2[x] ) )
			return x;
	return -1;
}

int get_areaflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, area_flags[x] ) )
        return x;
    return -1;
}

int get_wflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, w_flags[x] ) )
        return x;
    return -1;
}

int get_actflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, act_flags[x] ) )
        return x;
    return -1;
}

int get_pcflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, pc_flags[x] ) )
        return x;
    return -1;
}
int get_plrflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, plr_flags[x] ) )
        return x;
    return -1;
}

int get_risflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, ris_flags[x] ) )
        return x;
    return -1;
}

int get_trigflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, trig_flags[x] ) )
        return x;
    return -1;
}

int get_partflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, part_flags[x] ) )
        return x;
    return -1;
}

int get_attackflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, attack_flags[x] ) )
        return x;
    return -1;
}

int get_defenseflag( const char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
      if ( !str_prefix( flag, defense_flags[x] ) )
        return x;
    return -1;
}

int get_langflag( const char *flag )
{
	int x;

	for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
		if ( !str_prefix( flag, lang_names[x] ) )
			return lang_array[x];
	return LANG_UNKNOWN;
}

/*
 * Remove carriage returns from a line
 */
const char *strip_cr( const char *str )
{
	static char newstr[MAX_STRING_LENGTH];
	int i, j;

	for ( i=j=0; str[i] != '\0'; i++ )
		if ( str[i] != '\r' )
		{
			newstr[j++] = str[i];
		}
	newstr[j] = '\0';
	return newstr;
}


/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
    int len;
    char last;
    char *strptr;

    strptr = str;

    len  = strlen( str );
    if ( len )
      last = strptr[len-1];
    else
      last = '\0';

    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }
    if ( len )
      strptr[len-1] = last;

    return;
}


void start_editing( CHAR_DATA *ch, const char *data )
{
	EDITOR_DATA *edit;
	sh_int lines, size, lpos;
	char c;

	if ( !ch->GetConnection() )
	{
		bug( "Fatal: start_editing: no desc", 0 );
		return;
	}
	if ( ch->substate == SUB_RESTRICTED )
		bug( "NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED", 0 );

	set_char_color( AT_GREEN, ch );
	send_to_char( "Begin entering your text now (/? = help /s = save /c = clear /l = list)\r\n", ch );
	send_to_char( "-----------------------------------------------------------------------\r\n> ", ch );
	if ( ch->editor )
		stop_editing( ch );

	CREATE( edit, EDITOR_DATA, 1 );
	edit->numlines = 0;
	edit->on_line  = 0;
	edit->size     = 0;
	size = 0;  lpos = 0;  lines = 0;
	if ( !data )
		bug("editor: data is NULL!\r\n",0);
	else
	{
		for ( ;; )
		{
			c = data[size++];
			if ( c == '\0' )
			{
				edit->line[lines][lpos] = '\0';
				break;
			}
			else
				if ( c == '\r' );
				else
					if ( c == '\n' || lpos > MAX_BUFFER_COLS)
					{
						edit->line[lines][lpos] = '\0';
						lines++;
						lpos = 0;
					}
					else
						edit->line[lines][lpos++] = c;
			if ( lines >= MAX_BUFFER_LINES || size > MAX_BUFFER_SIZE )
			{
				edit->line[lines][lpos] = '\0';
				break;
			}
		}
	}
	edit->numlines = lines;
	edit->size = size;
	edit->on_line = lines;
	ch->editor = edit;
	ch->GetConnection()->ConnectedState = CON_EDITING;
}

char *copy_buffer( CHAR_DATA *ch )
{
	char buf[MAX_BUFFER_SIZE];
	char tmp[MAX_BUFFER_COLS+3];
	sh_int x, len;

	if ( !ch )
	{
		bug( "copy_buffer: null ch", 0 );
		return STRALLOC( "" );
	}

	if ( !ch->editor )
	{
		bug( "copy_buffer: null editor", 0 );
		return STRALLOC( "" );
	}

	buf[0] = '\0';
	for ( x = 0; x < ch->editor->numlines; x++ )
	{
		strcpy( tmp, ch->editor->line[x] );
		smush_tilde( tmp );
		len = strlen(tmp);
		if ( tmp[len-1] == '~' )
			tmp[len-1] = '\0';
		else
			strcat( tmp, "\r\n" );
		strcat( buf, tmp );
	}

	// Check the string's length
	if ( strlen(buf) > MAX_STRING_LENGTH )
	{
		bug( "WARNING: copy_buffer: buffer too long! Truncating.", 0 );
		buf[MAX_STRING_LENGTH-2] = '\0';
	}


	return STRALLOC( buf );
}


// Another version of copy_buffer, but returns an STL string instead.
// this does not touch the hash table.
//     -Ksilyan
string copy_buffer_string( CHAR_DATA *ch )
{
	string buf;
	char tmp[MAX_BUFFER_COLS+3];
	sh_int x, len;

	if ( !ch )
	{
		bug( "copy_buffer_string: null ch", 0 );
		return "";
	}

	if ( !ch->editor )
	{
		bug( "copy_buffer_string: null editor", 0 );
		return "";
	}

	buf = "";
	for ( x = 0; x < ch->editor->numlines; x++ )
	{
		strcpy( tmp, ch->editor->line[x] );
		smush_tilde( tmp );
		len = strlen(tmp);
		if ( tmp[len-1] == '~' )
			tmp[len-1] = '\0';
		else
			strcat( tmp, "\r\n" );
		buf.append( tmp );
	}

	// Check the string's length
	if ( buf.length() > MAX_STRING_LENGTH )
	{
		bug( "WARNING: copy_buffer_string: buffer too long! Truncating.", 0 );
		buf = buf.substr(0, MAX_STRING_LENGTH-2);
	}

	return buf;
}

void stop_editing( CHAR_DATA *ch )
{
	set_char_color( AT_PLAIN, ch );
	DISPOSE( ch->editor );
	ch->editor = NULL;
	send_to_char( "Done.\r\n", ch );
	ch->dest_buf  = NULL;
	ch->spare_ptr = NULL;
	ch->substate  = SUB_NONE;
	if ( !ch->GetConnection() )
	{
		bug( "Fatal: stop_editing: no desc", 0 );
		return;
	}
	ch->GetConnection()->ConnectedState = CON_PLAYING;
}

/* Testaur: test to allow if player ch may build */

int playerbuilderallowedgoto(CHAR_DATA *ch)
{
	if(get_trust(ch)>=LEVEL_IMMORTAL)
		return(TRUE);

	/* lowbie builders have SO many restrictions */
	if(ch->position == POS_FIGHTING)
	{
		send_to_char("No way! You are still fighting!\r\n",ch);
		return(FALSE);
	}
	if(ch->position <= POS_STUNNED)
	{
		send_to_char("You are hurt too bad for that!\r\n",ch);
		return(FALSE);
	}
	if(ch->GetMount() )
	{
		send_to_char("Dismount first!\r\n",ch);
		return(FALSE);
	}
	if(ch->position < POS_STANDING)
	{
		send_to_char("You must be standing to do that properly.\r\n",ch);
		return(FALSE);
	}
	if ( get_timer(ch, TIMER_RECENTFIGHT) > 0)
	{
		send_to_char( "Your adrenaline is pumping too hard!\r\n", ch );
		return(FALSE);
	}
	if( IS_NPC(ch) && IS_SET(ch->act,ACT_POLYMORPHED))
	{
		send_to_char( "Not while polymorphed!\r\n",ch);
		return(FALSE);
	}
	if( IS_NPC(ch) )
	{
		send_to_char( "Mobs cannot do that!\r\n",ch);
		return(FALSE);
	}
	if(!ch->pcdata->area)
	{
		send_to_char("You must have an assigned area to do that!\r\n",ch);
		return(FALSE);
	}
	return(TRUE); /* okay, let the player use goto finally */
}

int playerbuilderallowed(CHAR_DATA *ch)
{
	if(!playerbuilderallowedgoto(ch))
		return(FALSE);
	if(get_trust( ch ) < LEVEL_IMMORTAL)
	{
		if(

				ch->GetInRoom()->vnum < ch->pcdata->area->low_r_vnum
				|| ch->GetInRoom()->vnum > ch->pcdata->area->hi_r_vnum
		  )
		{
			send_to_char("Not from outside your assigned area!\r\n",ch);
			return(FALSE);
		}
	}
	return(TRUE); /* okay, let the player build finally */
}


int playerbuilderbadvnum(CHAR_DATA *ch, int vnum)
{
	AREA_DATA *pArea;

	if( get_trust(ch) < LEVEL_ENGINEER )
	{
		if(
				ch->pcdata!=NULL
				&& (pArea=ch->pcdata->area)!=NULL
				&& vnum >= pArea->low_r_vnum
				&& vnum <= pArea->hi_r_vnum
		  )
			; /* allowed */
		else
		{
			send_to_char("Vnum not in your assigned range!\r\n",ch);
			return(TRUE);
		}
	}

	return(FALSE);
}

void do_goto(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    AREA_DATA *pArea;
    int vnum;

    one_argument( argument, arg );

    if(!playerbuilderallowed(ch))
      return; /* disallowed */

    if ( arg[0] == '\0' )
    {
        send_to_char( "Goto where?\r\n", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        vnum = dotted_to_vnum( ch->GetInRoom()->vnum, arg );

        if ( vnum < 0 || get_room_index(vnum) )
        {
            send_to_char( "No such location.\r\n", ch );
            return;
        }

        if ( vnum < 1 || IS_NPC(ch) || !ch->pcdata->area )
        {
            send_to_char( "No such location.\r\n", ch );
            return;
        }

        if ( get_trust( ch ) < sysdata.level_modify_proto )
        {
            if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
            {
                send_to_char( "You must have an assigned area to create rooms.\r\n", ch );
                return;
            }

            if ( vnum < pArea->low_r_vnum
            ||   vnum > pArea->hi_r_vnum )
            {
                send_to_char( "That room is not within your assigned range.\r\n", ch );
                return;
            }
        }

        location = make_room( vnum );

        if ( !location )
        {
            bug( "Goto: make_room failed", 0 );
            return;
        }

        location->area = ch->pcdata->area;
        set_char_color( AT_WHITE, ch );
        send_to_char( "Waving your hand, you form order from swirling chaos,\r\nand step into a new reality...\r\n", ch );
    }

    /* check destination for lowbie builders */
    if(playerbuilderbadvnum(ch,location->vnum))
      return;
    if( get_trust(ch) < LEVEL_ENGINEER )
    {
      if(
             ch->pcdata!=NULL
          && (pArea=ch->pcdata->area)!=NULL
          && location->vnum >= pArea->low_r_vnum
          && location->vnum <= pArea->hi_r_vnum
        )
          ; /* allowed */
      else
      {
        send_to_char("You can't go there!\r\n",ch);
        return;
      }
    }

    if ( room_is_private( location ) )
    {
        if ( get_trust( ch ) < sysdata.level_override_private )
        {
            send_to_char( "That room is private right now.\r\n", ch );
            return;
        }
        else
        {
            send_to_char( "Overriding private flag!\r\n", ch );
        }
    }

    in_room = ch->GetInRoom();

    if ( ch->IsFighting() )
        ch->StopAllFights(); //stop_fighting( ch, TRUE );

    if ( current_arena.pArea && !str_cmp(location->area->filename, current_arena.pArea->filename) && !in_arena(ch)) {
        char buf[MAX_STRING_LENGTH];

        arena_clear_char(ch);

		if ( ! ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_WIZINVIS )
			&&    ch->pcdata->wizinvis >= LEVEL_HERO_MIN ))
		{
        	sprintf(buf, "%s has descended down to watch the combating mortals.", NAME(ch) );
	        talk_channel(ch, buf, CHANNEL_ARENA, "");
		}
    }

    if ( current_arena.pArea && str_cmp(location->area->filename, current_arena.pArea->filename)
         && in_arena(ch)) {
        char buf[MAX_STRING_LENGTH];

        arena_clear_char(ch);

		 if ( ! ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_WIZINVIS )
		 	&&    ch->pcdata->wizinvis >= LEVEL_HERO_MIN ))
		{
	        sprintf(buf, "%s, growing tired of the fighting, has left the arena.", NAME(ch));
	        talk_channel(ch, buf, CHANNEL_ARENA, "");
		}
    }


    for ( fch = in_room->first_person; fch; fch = fch->next_in_room ) {
        if ( can_see(fch, ch) )
        {
            char buf[MAX_STRING_LENGTH];
            sprintf(buf, "%s",
    	        (ch->pcdata && ch->pcdata->bamfOut_.length() > 0)
    	        ? ch->pcdata->bamfOut_.c_str() : "$n leaves in a swirling mist.");
    	    act( AT_IMMORT, buf, ch, NULL, fch, TO_VICT);
        }
    }

    ch->regoto = ch->GetInRoom()->vnum;
    char_from_room( ch );

    if ( ch->GetMount() )
    {
        char_from_room( ch->GetMount() );
        char_to_room( ch->GetMount(), location );
    }

    char_to_room( ch, location );

    for ( fch = location->first_person; fch; fch = fch->next_in_room ) {
        if ( can_see(fch, ch) )
        {
            char buf[MAX_STRING_LENGTH];
            sprintf(buf, "%s",
    	        (ch->pcdata && ch->pcdata->bamfIn_.length() > 0)
    	        ? ch->pcdata->bamfIn_.c_str() : "$n appears in a swirling mist.");
    	    act( AT_IMMORT, buf, ch, NULL, fch, TO_VICT);
        }
    }

    do_look( ch, "auto" );

    if ( ch->GetInRoom() == in_room )
      return;

    for ( fch = in_room->first_person; fch; fch = fch_next )
    {
        fch_next = fch->next_in_room;

        if ( fch->MasterId == ch->GetId() && IS_IMMORTAL(fch) )
        {
            act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
            do_goto( fch, argument );
        }
    }
    return;
}

void do_mset(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char buf  [MAX_STRING_LENGTH];
	char outbuf[MAX_STRING_LENGTH];
	int  num,size,plus;
	char char1,char2;
	CHAR_DATA *victim;
	int value;
	int minattr, maxattr;
	bool lockvictim;
	const char *origarg = argument;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't mset.\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}
	if (!playerbuilderallowed(ch))
		return;

	switch( ch->substate )
	{
		default:
			break;
		case SUB_MOB_DESC:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_mset: sub_mob_desc: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			victim = (CHAR_DATA *) ch->dest_buf;
			if ( char_died(victim) )
			{
				send_to_char( "Your victim died!\r\n", ch );
				stop_editing( ch );
				return;
			}
			victim->description_ = copy_buffer_string( ch );
			if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			{
				victim->pIndexData->description_ = victim->description_;
			}
			stop_editing( ch );
			ch->substate = ch->tempnum;
			return;
	}

	victim = NULL;
	lockvictim = FALSE;
	argument = smash_tilde_static( argument );

	if ( ch->substate == SUB_REPEATCMD )
	{
		victim = (CHAR_DATA *) ch->dest_buf;
		if ( char_died(victim) )
		{
			send_to_char( "Your victim died!\r\n", ch );
			victim = NULL;
			argument = "done";
		}
		if ( argument[0] == '\0' || !str_cmp( argument, " " )
				||   !str_prefix( argument, "stat" ) )
		{
			if ( victim )
				do_mstat( ch, (char*) victim->getName().c_str() );
			else
				send_to_char( "No victim selected.  Type '?' for help.\r\n", ch );
			return;
		}
		if ( !str_prefix( argument, "done" ) || !str_prefix( argument, "off" ) )
		{
			send_to_char( "Mset mode off.\r\n", ch );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
			if ( ch->pcdata && ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			return;
		}
	}
	if ( victim )
	{
		lockvictim = TRUE;
		strcpy( arg1, victim->getName().c_str() );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}
	else
	{
		lockvictim = FALSE;
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}

	if ( !str_prefix( arg1, "on" ) )
	{
		send_to_char( "Syntax: mset <victim|vnum> on.\r\n", ch );
		return;
	}

	if ( arg1[0] == '\0' || (arg2[0] == '\0' && ch->substate != SUB_REPEATCMD)
			||   !str_cmp( arg1, "?" ) )
	{
		if ( ch->substate == SUB_REPEATCMD )
		{
			if ( victim )
				send_to_char( "Syntax: <field>  <value>\r\n",		ch );
			else
				send_to_char( "Syntax: <victim> <field>  <value>\r\n",	ch );
		}
		else
			send_to_char( "Syntax: mset <victim> <field>  <value>\r\n",	ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Field being one of:\r\n",			ch );
		send_to_char( "  str int wis dex con cha lck sex class\r\n",	ch );
		send_to_char( "  gold hp mana move practice align race\r\n",	ch );
		send_to_char( "  hitroll damroll armor affected level\r\n",	ch );
		send_to_char( "  thirst drunk full blood flags\r\n",		ch );
		send_to_char( "  pos defpos part (see BODYPARTS)\r\n",		ch );
		send_to_char( "  sav1 sav2 sav4 sav4 sav5 (see SAVINGTHROWS)\r\n", ch );
		send_to_char( "  resistant immune susceptible (see RIS)\r\n",	ch );
		send_to_char( "  attack defense numattacks\r\n",		ch );
		send_to_char( "  speaking speaks (see LANGUAGES)\r\n",		ch );
		send_to_char( "  name short long description title spec clan\r\n", ch );
		send_to_char( "  council quest qp qpa favor recall bank deity\r\n", ch );
		send_to_char( "  homevnum scent\r\n", ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "For editing index/prototype mobiles:\r\n",	ch );
		send_to_char( "  hitnumdie hitsizedie hitplus (hit points)\r\n",ch );
		send_to_char( "  damnumdie damsizedie damplus (damage roll)\r\n",ch );
		send_to_char( "To toggle area flag: aloaded\r\n",ch);
		send_to_char( "\r\n", ch);
		send_to_char( "Thief flags:\r\n", ch);
		send_to_char( "  citythief\r\n", ch);
		return;
	}

	if ( !victim && get_trust( ch ) < LEVEL_STONE_ADEPT )
	{
		if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
	}
	else
		if ( !victim )
		{
			if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
			{
				send_to_char( "No one like that in all the realms.\r\n", ch );
				return;
			}
		}

	if ( !IS_NPC(victim)
			&& ( get_trust( ch ) < get_trust(victim)
				|| get_trust(ch) < LEVEL_IMMORTAL
			   )
	   )
	{
		send_to_char( "You can't do that!\r\n", ch );
		ch->dest_buf = NULL;
		return;
	}
	if ( !IS_NPC(victim) && ch != victim )
	{ /* Testaur - certain affect player msets ARE allowed */
		if ( !str_prefix(arg2,"title")
				|| !str_prefix(arg2,"mentalstate")
				|| !str_prefix(arg2,"emotion")
				|| !str_prefix(arg2,"favor")
		   )
			; /* the above are all allowed */
		else
		{/* imm wants to change something else */
			if ( get_trust( ch ) < sysdata.level_mset_player )
			{
				send_to_char("You can't do that!\r\n", ch);
				ch->dest_buf = NULL;
				return;
			}
		}
	}
	if ( lockvictim )
		ch->dest_buf = victim;

	if ( IS_NPC(victim) )
	{
		minattr = 1;
		maxattr = 25;
	}
	else
	{
		minattr = 3;
		maxattr = 18;
	}

	if ( !str_prefix( arg2, "on" ) )
	{
#if 0 /* Testaur - temporarily forbidden */
		CHECK_SUBRESTRICTED( ch );
		ch_printf( ch, "Mset mode on. (Editing %s).\r\n",
				victim->getName().c_str() );
		ch->substate = SUB_REPEATCMD;
		ch->dest_buf = victim;
		if ( ch->pcdata )
		{
			if ( ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			if ( IS_NPC(victim) )
				sprintf( buf, "<&CMset &W#%s&w> %%i", vnum_to_dotted(victim->pIndexData->vnum) );
			else
				sprintf( buf, "<&CMset &W%s&w> %%i", victim->getName().c_str() );
			ch->pcdata->subprompt = STRALLOC( buf );
		}
#else
		send_to_char("Sorry, this command closed for repair.\r\n",ch);
#endif
		return;
	}
	value = is_number( arg3 ) ? atoi( arg3 ) : -1;

	if ( atoi(arg3) < -1 && value == -1 )
		value = atoi(arg3);

	if ( !str_prefix( arg2, "str" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Strength range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_str = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_str = value;
		return;
	}

	if ( !str_prefix( arg2, "int" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Intelligence range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_int = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_int = value;
		return;
	}

	if ( !str_prefix( arg2, "wis" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Wisdom range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_wis = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_wis = value;
		return;
	}

	if ( !str_prefix( arg2, "dex" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Dexterity range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_dex = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_dex = value;
		return;
	}

	if ( !str_prefix( arg2, "con" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Constitution range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_con = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_con = value;
		return;
	}

	if ( !str_prefix( arg2, "cha" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Charisma range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_cha = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_cha = value;
		return;
	}

	if ( !str_prefix( arg2, "lck" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf( ch, "Luck range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_lck = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_lck = value;
		return;
	}

	if ( !str_prefix( arg2, "sav1" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_poison_death = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_poison_death = value;
		return;
	}

	if ( !str_prefix( arg2, "sav2" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_wand = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_wand = value;
		return;
	}

	if ( !str_prefix( arg2, "sav3" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_para_petri = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_para_petri = value;
		return;
	}

	if ( !str_prefix( arg2, "sav4" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_breath = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_breath = value;
		return;
	}

	if ( !str_prefix( arg2, "sav5" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < -30 || value > 30 )
		{
			send_to_char( "Saving throw range is -30 to 30.\r\n", ch );
			return;
		}
		victim->saving_spell_staff = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_spell_staff = value;
		return;
	}

	if ( !str_prefix( arg2, "sex" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 2 )
		{
			send_to_char( "Sex range is 0 to 2.\r\n", ch );
			return;
		}
		victim->sex = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->sex = value;
		return;
	}

	if ( !str_prefix( arg2, "scent" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;

		if ( value < 0 )
		{
			send_to_char( "Scent ID must be more than 0\r\n", ch );
			return;
		}

		if ( ScentController::instance()->findScent(value) == NULL && value != 0)
		{
			send_to_char("Scent ID does not exist.\r\n", ch);
			return;
		}

		victim->setScent(value);
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->ScentId = value;
		return;
	}

	if ( !str_prefix( arg2, "class" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;

		if( IS_NPC(victim) )   /* Broken by Haus... fixed by Thoric */
		{
			if ( value == -1 ) {
				int i;

				for ( i = 0; i < MAX_NPC_CLASS; i++ ) {
					if ( !str_prefix(arg3, npc_class[i]) ) {
						value = i;
						break;
					}
				}

				if ( value == -1 ) {
					send_to_char("Valid classes:\n", ch);
					for ( i = 0; i < MAX_NPC_CLASS; i+=3 ) {
						ch_printf(ch, "%-20s %-20s %-20s\n",
								i+0 >= MAX_NPC_CLASS ? "" : npc_class[i+0],
								i+1 >= MAX_NPC_CLASS ? "" : npc_class[i+1],
								i+2 >= MAX_NPC_CLASS ? "" : npc_class[i+2]
								);
					}
					return;
				}
			}

			if ( value > MAX_NPC_CLASS && value >= 0 )
			{
				ch_printf( ch, "NPC Class range is 0 to %d.\n", MAX_NPC_CLASS-1 );              return;
			}

			victim->Class = value;

			if ( IS_SET( victim->act, ACT_PROTOTYPE ) )
				victim->pIndexData->Class = value;
			return;
		}

		if ( value == -1 ) {
			int i;

			for ( i = 0; i < MAX_CLASS; i++ ) {
				if ( !str_prefix(arg3, npc_class[i]) ) {
					value = i;
					break;
				}
			}

			if ( value == -1 ) {
				send_to_char("Valid classes:\n", ch);
				for ( i = 0; i < MAX_CLASS; i+=3 ) {
					ch_printf(ch, "%-20s %-20s %-20s\n",
							i+0 >= MAX_CLASS ? "" : npc_class[i+0],
							i+1 >= MAX_CLASS ? "" : npc_class[i+1],
							i+2 >= MAX_CLASS ? "" : npc_class[i+2]
							);
				}
				return;
			}
		}

		if ( value < 0 || value >= MAX_CLASS )
		{
			ch_printf( ch, "Class range is 0 to %d.\n", MAX_CLASS-1 );
			return;
		}
		victim->Class = value;
		return;
	}

	if ( !str_prefix( arg2, "race" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		value = get_npc_race( arg3 );
		if ( value < 0 )
			value = atoi( arg3 );
		if ( !IS_NPC(victim) && (value < 0 || value >= MAX_RACE) )
		{
			ch_printf( ch, "Race range is 0 to %d.\n", MAX_RACE-1 );
			return;
		}
		if ( IS_NPC(victim) && (value < 0 || value >= MAX_NPC_RACE) )
		{
			ch_printf( ch, "Race range is 0 to %d.\n", MAX_NPC_RACE-1 );
			return;
		}
		victim->race = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->race = value;
		return;
	}

	if ( !str_prefix( arg2, "armor" ) || !str_prefix(arg2, "ac") )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < -300 || value > 300 )
		{
			send_to_char( "AC range is -300 to 300.\r\n", ch );
			return;
		}
		victim->armor = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->ac = value;
		return;
	}

	if ( !str_prefix( arg2, "level" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > LEVEL_HERO_MAX )
		{
			ch_printf( ch, "Level range is 0 to %d.\r\n", LEVEL_HERO_MAX );
			return;
		}
		victim->level = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->level = value;
		return;
	}

	if ( !str_prefix( arg2, "numattacks" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > 20 )
		{
			send_to_char( "Attacks range is 0 to 20.\r\n", ch );
			return;
		}
		victim->numattacks = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->numattacks = value;
		return;
	}

	if ( !str_prefix( arg2, "gold" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		victim->gold = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->gold = value;
		return;
	}

	if ( !str_prefix( arg2, "hitroll" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		victim->hitroll = URANGE(0, value, 85);
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitroll = victim->hitroll;
		return;
	}

	if ( !str_prefix( arg2, "damroll" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		victim->damroll = URANGE(0, value, 65);
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damroll = victim->damroll;
		return;
	}

	if ( !str_prefix( arg2, "hp" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 1 || value > 32700 )
		{
			send_to_char( "Hp range is 1 to 32,700 hit points.\r\n", ch );
			return;
		}
		victim->max_hit = value;
		return;
	}

	if ( !str_prefix( arg2, "mana" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 30000 )
		{
			send_to_char( "Mana range is 0 to 30,000 mana points.\r\n", ch );
			return;
		}
		victim->max_mana = value;
		return;
	}

	if ( !str_prefix( arg2, "move" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 30000 )
		{
			send_to_char( "Move range is 0 to 30,000 move points.\r\n", ch );
			return;
		}
		victim->max_move = value;
		return;
	}

	if ( !str_prefix( arg2, "practice" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 100 )
		{
			send_to_char( "Practice range is 0 to 100 sessions.\r\n", ch );
			return;
		}
		victim->practice = value;
		return;
	}

	if ( !str_prefix( arg2, "align" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < -1000 || value > 1000 )
		{
			send_to_char( "Alignment range is -1000 to 1000.\r\n", ch );
			return;
		}
		victim->alignment = value;
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->alignment = value;
		return;
	}

	// Ksilyan
	if ( !str_prefix( arg2, "homevnum" ) )
	{
		ROOM_INDEX_DATA * room = NULL;

		if (!IS_NPC(victim))
		{
			send_to_char( "Not on players.\r\n", ch );
			return;
		}

		if (value == 0)
		{
			victim->HomeVnum = 0;
			ch_printf(ch, "Mob's home removed.\r\n");
			if ( IS_SET(victim->act, ACT_PROTOTYPE) )
				victim->pIndexData->HomeVnum = 0;
			return;
		}

		if ( strlen(arg3) == 0)
		{
			send_to_char( "Please specify a home vnum.\r\n", ch );
			return;
		}
		if ( (room = get_room_index(dotted_to_vnum(ch->GetInRoom()->vnum, arg3))) == NULL )
		{
			send_to_char( "That room does not exist.\r\n", ch );
			return;
		}
		victim->HomeVnum = room->vnum;
		ch_printf(ch, "Mob's home set to: %s (%s).\r\n", room->name_.c_str(), arg3);
		if ( IS_SET(victim->act, ACT_PROTOTYPE) )
			victim->pIndexData->HomeVnum = room->vnum;
		return;
	}
	if ( !str_prefix( arg2, "bank" ) )
	{
		if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if( IS_NPC( victim ) )
		{
			send_to_char( "Not on mobs.\r\n", ch );
			return;
		}
		victim->pcdata->bank_gold = value;

	}
	if ( !str_prefix( arg2, "recall" ) )
	{
		if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC( victim ) )
		{
			send_to_char( "Not on mobs.\r\n", ch );
			return;
		}
		if ( get_room_index( value ) == NULL )
		{
			send_to_char( "No such location.\r\n", ch );
			return;
		}
		victim->pcdata->recall_room = value;
	}
	if ( !str_prefix( arg2, "password" ) )
	{
		char *pwdnew;
		char *p;

		if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC( victim ) )
		{
			send_to_char( "Mobs don't have passwords.\r\n", ch );
			return;
		}

		if ( strlen(arg3) < 5 )
		{
			send_to_char(
					"New password must be at least five characters long.\r\n", ch );
			return;
		}

		/*
		 * No tilde allowed because of player file format.
		 */
		pwdnew = crypt( arg3, ch->getName().c_str() );
		for ( p = pwdnew; *p != '\0'; p++ )
		{
			if ( *p == '~' )
			{
				send_to_char(
						"New password not acceptable, try again.\r\n", ch );
				return;
			}
		}

		DISPOSE( victim->pcdata->pwd );
		victim->pcdata->pwd = str_dup( pwdnew );
		if ( IS_SET(sysdata.save_flags, SV_PASSCHG) )
			save_char_obj( victim );
		send_to_char( "Ok.\r\n", ch );
		ch_printf( victim, "Your password has been changed by %s.\r\n", ch->getName().c_str() );
		return;
	}

	if ( !str_prefix( arg2, "quest" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > 500 )
		{
			send_to_char( "The current quest range is 0 to 500.\r\n", ch );
			return;
		}

		victim->pcdata->quest_number = value;
		return;
	}

	if ( !str_prefix( arg2, "qp" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > 5000 )
		{
			send_to_char( "The current quest point range is 0 to 5000.\r\n", ch );
			return;
		}

		victim->pcdata->quest_curr = value;
		return;
	}

	if ( !str_prefix( arg2, "qpa" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		victim->pcdata->quest_accum = value;
		return;
	}

	if ( !str_prefix( arg2, "favor" ) )
	{
		if ( IS_NPC( victim ) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( value < -1000 || value > 1000 )
		{
			send_to_char( "Range is from -1000 to 1000.\r\n", ch );
			return;
		}

		victim->pcdata->favor = value;
		return;
	}

	if ( !str_prefix( arg2, "mentalstate" ) )
	{
		if ( value < -100 || value > 100 )
		{
			send_to_char( "Value must be in range -100 to +100.\r\n", ch );
			return;
		}
		victim->mental_state = value;
		return;
	}

	if ( !str_prefix( arg2, "emotion" ) )
	{
		if ( value < -100 || value > 100 )
		{
			send_to_char( "Value must be in range -100 to +100.\r\n", ch );
			return;
		}
		victim->emotional_state = value;
		return;
	}

	if ( !str_prefix( arg2, "thirst" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > 100 )
		{
			send_to_char( "Thirst range is 0 to 100.\r\n", ch );
			return;
		}

		victim->pcdata->condition[COND_THIRST] = value;
		return;
	}

	if ( !str_prefix( arg2, "drunk" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > 100 )
		{
			send_to_char( "Drunk range is 0 to 100.\r\n", ch );
			return;
		}

		victim->pcdata->condition[COND_DRUNK] = value;
		return;
	}

	if ( !str_prefix( arg2, "full" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > 100 )
		{
			send_to_char( "Full range is 0 to 100.\r\n", ch );
			return;
		}

		victim->pcdata->condition[COND_FULL] = value;
		return;
	}

	if ( !str_prefix( arg2, "blood" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( value < 0 || value > MAX_LEVEL+10 )
		{
			ch_printf( ch, "Blood range is 0 to %d.\r\n", MAX_LEVEL+10 );
			return;
		}

		victim->pcdata->condition[COND_BLOODTHIRST] = value;
		return;
	}

	if ( !str_prefix( arg2, "name" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !IS_NPC(victim) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		victim->setName( arg3 );
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->playerName_ = victim->getName();
		}
		return;
	}

	if ( !str_prefix( arg2, "minsnoop" ) )
	{
		if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( victim->pcdata )
		{
			victim->pcdata->min_snoop = value;
			return;
		}
	}

	if ( !str_prefix( arg2, "clan" ) )
	{
		ClanData *clan;

		if ( get_trust( ch ) < LEVEL_STONE_INITIATE )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( arg3[0] == '\0' )
		{
			victim->pcdata->clanName_	= "";
			victim->pcdata->clan	= NULL;
			send_to_char( "Removed from clan.\r\nPlease make sure you adjust that clan's members accordingly.\r\n", ch );
			return;
		}
		clan = get_clan( arg3 );
		if ( !clan )
		{
			send_to_char( "No such clan.\r\n", ch );
			return;
		}
		victim->pcdata->clanName_ = clan->name_.c_str();
		victim->pcdata->clan = clan;
		send_to_char( "Done.\r\nPlease make sure you adjust that clan's members accordingly.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "deity" ))
	{
		DeityData *deity;

		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( arg3[0] == '\0' )
		{
			victim->pcdata->deityName_        = "";
			victim->pcdata->deity             = NULL;
			send_to_char( "Deity removed.\r\n", ch );
			return;
		}

		deity = get_deity( arg3 );
		if ( !deity )
		{
			send_to_char( "No such deity.\r\n", ch );
			return;
		}
		victim->pcdata->deityName_ = deity->name_;
		victim->pcdata->deity = deity;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "council" ) )
	{
		CouncilData *council;

		if ( get_trust( ch ) < LEVEL_STONE_INITIATE )
		{
			send_to_char( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		if ( arg3[0] == '\0' )
		{
			victim->pcdata->councilName_	= "";
			victim->pcdata->council		= NULL;
			send_to_char( "Removed from council.\r\nPlease make sure you adjust that council's members accordingly.\r\n", ch );
			return;
		}

		council = get_council( arg3 );
		if ( !council )
		{
			send_to_char( "No such council.\r\n", ch );
			return;
		}
		victim->pcdata->councilName_ = council->name_.c_str();
		victim->pcdata->council = council;
		send_to_char( "Done.\r\nPlease make sure you adjust that council's members accordingly.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "exitmsg" ) )
	{
		if ( arg3[0] != '\0' )
			victim->exitDesc_ = arg3;
		else
			send_to_char("Exit Message cleared.\n", ch);

		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE) )
		{
			if ( arg3[0] != '\0' )
				victim->pIndexData->exitDesc_ = victim->exitDesc_;
		}
		return;
	}

	if ( !str_prefix( arg2, "entermsg" ) )
	{
		if ( arg3[0] != '\0' )
			victim->enterDesc_ = arg3;
		else
			send_to_char("Enter message cleared.\n", ch);

		if ( IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE) )
		{
			if ( arg3[0] != '\0' )
				victim->pIndexData->enterDesc_ = victim->enterDesc_;
		}

		return;
	}


	if ( !str_prefix( arg2, "short" ) )
	{
		victim->setShort(arg3);
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->shortDesc_ = victim->getShort(false);
		}
		return;
	}

	if ( !str_prefix( arg2, "long" ) )
	{
		strcpy( buf, arg3 );
		strcat( buf, "\r\n" );
		victim->setLong(buf);
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
		{
			victim->pIndexData->longDesc_ = victim->getLong();
		}
		return;
	}

	if ( !str_prefix( arg2, "description" ) )
	{
		if ( arg3[0] )
		{
			victim->description_ = arg3;
			if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			{
				victim->pIndexData->description_ = victim->description_;
			}
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_MOB_DESC;
		ch->dest_buf = victim;
		start_editing( ch, victim->description_.c_str() );
		return;
	}

	if ( !str_prefix( arg2, "title" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Not on NPC's.\r\n", ch );
			return;
		}

		set_title( victim, arg3 );
		return;
	}

	if ( !str_prefix( arg2, "spec" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Not on PC's.\r\n", ch );
			return;
		}

		if ( !str_prefix( arg3, "none" ) )
		{
			victim->spec_fun = NULL;
			send_to_char( "Special function removed.\r\n", ch );
			if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
				victim->pIndexData->spec_fun = victim->spec_fun;
			return;
		}

		if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
		{
			send_to_char( "No such spec fun.\r\n", ch );
			return;
		}
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->spec_fun = victim->spec_fun;
		return;
	}

	if ( !str_prefix( arg2, "flags" ) )
	{
		bool pcflag;

		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's flags.\r\n", ch );
			return;
		}

		if ( !can_mmodify( ch, victim ) )
			return;

		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> flags <flag> [flag]...\r\n", ch );
			return;
		}

		while ( argument[0] != '\0' )
		{
			char prefix;
			pcflag = FALSE;

			argument = one_argument( argument, arg3 );

			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a;
				++b; prefix = *a;
				while((*a++ = *b++));
			}

			value = IS_NPC( victim) ? get_actflag( arg3 ) : get_plrflag( arg3 );

			if ( !IS_NPC( victim ) && ( value < 0 || value > 31 ) )
			{
				pcflag = TRUE;
				value = get_pcflag( arg3 );
			}

			if ( value < 0 || value > 31 )
			{
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			}
			else
			{
				if ( IS_NPC(victim)
						&&	 1 << value == ACT_PROTOTYPE
						&&	 get_trust( ch ) < sysdata.level_modify_proto_flag
						&&	!is_name ("protoflag", ch->pcdata->bestowments_.c_str()) )
					send_to_char( "You cannot change the prototype flag.\r\n", ch );
				else if ( IS_NPC(victim) && 1 << value == ACT_IS_NPC )
					send_to_char( "If that could be changed, it would cause many problems.\r\n", ch );
				else if ( IS_NPC(victim) && 1 << value == ACT_POLYMORPHED )
					send_to_char( "Changing that would be a _bad_ thing.\r\n", ch);
				else
				{
					if ( pcflag )
					{
						if ( prefix == '-' ) {
							REMOVE_BIT( victim->pcdata->flags, 1 << value );
						} else if ( prefix == '+' ) {
							SET_BIT(victim->pcdata->flags, 1 << value );
						} else {
							TOGGLE_BIT( victim->pcdata->flags, 1 << value );
						}
					}
					else
					{
						if ( prefix == '-' ) {
							REMOVE_BIT( victim->act, 1 << value );
						} else if ( prefix == '+' ) {
							SET_BIT( victim->act, 1 << value );
						} else {
							TOGGLE_BIT( victim->act, 1 << value );
						}
						/* NPC check added by Gorog */
						if ( IS_NPC(victim) && (1 << value == ACT_PROTOTYPE) )
							victim->pIndexData->act = victim->act;
					}
				}
			}
		}

		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->act = victim->act;
		return;
	}

	if ( !str_prefix( arg2, "affected" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's flags.\r\n", ch );
			return;
		}

		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> affected <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_aflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->affected_by, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->affected_by, 1 << value );
				} else {
					TOGGLE_BIT( victim->affected_by, 1 << value );
				}
			}
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->affected_by = victim->affected_by;
		return;
	}

	/* Testaur - (replaced incorrect str_prefix with str_cmp) */
	/* it was causing endless recursion leading to seg violation */
	/*
	 * save some more finger-leather for setting RIS stuff
	 */

	if ( !str_cmp( arg2, "r" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sprintf(outbuf,"%s resistant %s",arg1, arg3);
		do_mset( ch, outbuf );
		return;
	}
	if ( !str_cmp( arg2, "i" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;


		sprintf(outbuf,"%s immune %s",arg1, arg3);
		do_mset( ch, outbuf );
		return;
	}
	if ( !str_cmp( arg2, "s" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sprintf(outbuf,"%s susceptible %s",arg1, arg3);
		do_mset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "ri" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sprintf(outbuf,"%s resistant %s",arg1, arg3);
		do_mset( ch, outbuf );
		sprintf(outbuf,"%s immune %s",arg1, arg3);
		do_mset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "rs" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sprintf(outbuf,"%s resistant %s",arg1, arg3);
		do_mset( ch, outbuf );
		sprintf(outbuf,"%s susceptible %s",arg1, arg3);
		do_mset( ch, outbuf );
		return;
	}
	if ( !str_cmp( arg2, "is" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sprintf(outbuf,"%s immune %s",arg1, arg3);
		do_mset( ch, outbuf );
		sprintf(outbuf,"%s susceptible %s",arg1, arg3);
		do_mset( ch, outbuf );
		return;
	}
	if ( !str_cmp( arg2, "ris" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sprintf(outbuf,"%s resistant %s",arg1, arg3);
		do_mset( ch, outbuf );
		sprintf(outbuf,"%s immune %s",arg1, arg3);
		do_mset( ch, outbuf );
		sprintf(outbuf,"%s susceptible %s",arg1, arg3);
		do_mset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "resistant" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's resistancies.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> resistant <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_risflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->resistant, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->resistant, 1 << value );
				} else {
					TOGGLE_BIT( victim->resistant, 1 << value );
				}
			}
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->resistant = victim->resistant;
		return;
	}

	if ( !str_cmp( arg2, "immune" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's immunities.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> immune <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_risflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->immune, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->immune, 1 << value );
				} else {
					TOGGLE_BIT( victim->immune, 1 << value );
				}
			}
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->immune = victim->immune;
		return;
	}

	if ( !str_cmp( arg2, "susceptible" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's susceptibilities.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> susceptible <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_risflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->susceptible, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->susceptible, 1 << value );
				} else {
					TOGGLE_BIT( victim->susceptible, 1 << value );
				}
			}
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->susceptible = victim->susceptible;
		return;
	}
	/* End of Testaur's repair */

	if ( !str_prefix( arg2, "part" ) )
	{
		if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char( "You can only modify a mobile's parts.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> part <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_partflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->xflags, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->xflags, 1 << value );
				} else {
					TOGGLE_BIT( victim->xflags, 1 << value );
				}
			}
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->xflags = victim->xflags;
		return;
	}

	if ( !str_prefix( arg2, "attacks" ))
	{
		if ( !IS_NPC( victim ) )
		{
			send_to_char( "You can only modify a mobile's attacks.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> attack <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_attackflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->attacks, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->attacks, 1 << value );
				} else {
					TOGGLE_BIT( victim->attacks, 1 << value );
				}
			}
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->attacks = victim->attacks;
		return;
	}

	if ( !str_prefix( arg2, "defenses" ))
	{
		if ( !IS_NPC( victim ) )
		{
			send_to_char( "You can only modify a mobile's defenses.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> defense <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_defenseflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->defenses, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->defenses, 1 << value );
				} else {
					TOGGLE_BIT( victim->defenses, 1 << value );
				}
			}
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->defenses = victim->defenses;
		return;
	}

	if ( !str_prefix( arg2, "position" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > POS_STANDING )
		{
			ch_printf( ch, "Position range is 0 to %d.\r\n", POS_STANDING );
			return;
		}
		victim->position = value;
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->position = victim->position;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "defpos" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > POS_STANDING )
		{
			ch_printf( ch, "Position range is 0 to %d.\r\n", POS_STANDING );
			return;
		}
		victim->defposition = value;
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->defposition = victim->defposition;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	/*
	 * save some finger-leather
	 */
	if ( !str_prefix( arg2, "hitdie" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
		sprintf(outbuf,"%s hitnumdie %d",arg1, num);
		do_mset( ch, outbuf );

		sprintf(outbuf,"%s hitsizedie %d",arg1, size);
		do_mset( ch, outbuf );

		sprintf(outbuf,"%s hitplus %d",arg1, plus);
		do_mset( ch, outbuf );
		return;
	}
	/*
	 * save some more finger-leather
	 */
	if ( !str_prefix( arg2, "damdie" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;

		sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
		sprintf(outbuf,"%s damnumdie %d",arg1, num);
		do_mset( ch, outbuf );
		sprintf(outbuf,"%s damsizedie %d",arg1, size);
		do_mset( ch, outbuf );
		sprintf(outbuf,"%s damplus %d",arg1, plus);
		do_mset( ch, outbuf );
		return;
	}

	if ( !str_prefix( arg2, "hitnumdie" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 32767 )
		{
			send_to_char( "Number of hitpoint dice range is 0 to 30000.\r\n", ch );
			return;
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitnodice = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "hitsizedie" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 32767 )
		{
			send_to_char( "Hitpoint dice size range is 0 to 30000.\r\n", ch );
			return;
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitsizedice = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "hitplus" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 32767 )
		{
			send_to_char( "Hitpoint bonus range is 0 to 30000.\r\n", ch );
			return;
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitplus = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "damnumdie" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 100 )
		{
			send_to_char( "Number of damage dice range is 0 to 100.\r\n", ch );
			return;
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damnodice = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "damsizedie" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 100 )
		{
			send_to_char( "Damage dice size range is 0 to 100.\r\n", ch );
			return;
		}
		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damsizedice = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "damplus" ) )
	{
		if ( !IS_NPC(victim) )
		{
			send_to_char( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( value < 0 || value > 1000 )
		{
			send_to_char( "Damage bonus range is 0 to 1000.\r\n", ch );
			return;
		}

		if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damplus = value;
		send_to_char( "Done.\r\n", ch );
		return;

	}


	if ( !str_prefix( arg2, "aloaded" ) )
	{
		if ( IS_NPC(victim) )
		{
			send_to_char( "Player Characters only.\r\n", ch );
			return;
		}


		if ( !can_mmodify( ch, victim ) )
			return;

		if ( !IS_SET(victim->pcdata->area->status, AREA_LOADED ) )
		{
			SET_BIT( victim->pcdata->area->status, AREA_LOADED );
			send_to_char( "Your area set to LOADED!\r\n", victim );
			if ( ch != victim )
				send_to_char( "Area set to LOADED!\r\n", ch );
			return;
		}
		else
		{
			REMOVE_BIT( victim->pcdata->area->status, AREA_LOADED );
			send_to_char( "Your area set to NOT-LOADED!\r\n", victim );
			if ( ch != victim )
				send_to_char( "Area set to NON-LOADED!\r\n", ch );
			return;
		}
	}

	if ( !str_prefix( arg2, "speaks" ) )
	{
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> speaks <language> [language] ...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_langflag( arg3 );
			if ( value == LANG_UNKNOWN )
				ch_printf( ch, "Unknown language: %s\r\n", arg3 );
			else
				if ( !IS_NPC( victim ) )
				{
					int valid_langs = LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_PIXIE
						| LANG_OGRE | LANG_ORCISH | LANG_TROLLISH | LANG_GOBLIN
						| LANG_HALFLING;

					if ( !(value &= valid_langs) )
					{
						ch_printf( ch, "Players may not know %s.\r\n", arg3 );
						continue;
					}
				}
			if ( prefix == '-' ) {
				REMOVE_BIT( victim->speaks, value );
			} else if ( prefix == '+' ) {
				SET_BIT( victim->speaks, value );
			} else {
				TOGGLE_BIT( victim->speaks, value );
			}
		}
		if ( !IS_NPC( victim ) )
		{
			REMOVE_BIT( victim->speaks, race_table[victim->race].language );
			if ( !knows_language( victim, victim->speaking, victim ) )
				victim->speaking = race_table[victim->race].language;
		}
		else
			if ( IS_SET( victim->act, ACT_PROTOTYPE ) )
				victim->pIndexData->speaks = victim->speaks;
		send_to_char( "Done.\r\n", ch );
		return;
	}



	if ( !str_prefix( arg2, "speaking" ) )
	{
		if ( !IS_NPC( victim ) )
		{
			send_to_char( "Players must choose the language they speak themselves.\r\n", ch );
			return;
		}
		if ( !can_mmodify( ch, victim ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: mset <victim> speaking <language> [language]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_langflag( arg3 );
			if ( value == LANG_UNKNOWN )
				ch_printf( ch, "Unknown language: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( victim->speaking, value );
				} else if ( prefix == '+' ) {
					SET_BIT( victim->speaking, value );
				} else {
					TOGGLE_BIT( victim->speaking, value );
				}
			}
		}
		if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->speaking = victim->speaking;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	/*
	   CITY THIEF
	   Added by Ksilyan
	 */
	if ( !str_prefix( arg2, "citythief") )
	{
		if (IS_NPC( victim ) ) {
			send_to_char( "Only players have city thief flags.\r\n", ch);
			return;
		}
		value = is_number( arg3 ) ? atoi( arg3 ) : -1;
		if (value == -1) {
			send_to_char( "Usage: mset <victim> citythief <city number>...\r\n", ch );
			return;
		}
		if ( value >= MAX_CITIES ) {
			send_to_char( "That city is not valid.\r\n", ch);
			return;
		}

		victim->pcdata->citythief[value] = 1 - victim->pcdata->citythief[value];
		if (victim->pcdata->citythief[value] == 0) {
			ch_printf(ch, "City thief flag set to off for %s in %s.", victim->getName().c_str(), city_names[value]);
		} else if (victim->pcdata->citythief[value] == 1) {
			ch_printf(ch, "City thief flag set to on for %s in %s.", victim->getName().c_str(), city_names[value]);
		} else {
			victim->pcdata->citythief[value] = 0;
			send_to_char( "City thief flag was strange. Set to off.", ch);
		}

		return;
	}

	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_mset;
	}
	else
		do_mset( ch, "" );
	return;
}


void DoItemValue(int valNumber, Character * ch, Object * obj,
		const char * arg, int value)
{
	// special case for levers/buttons/etc.
	if ( valNumber == 1 || valNumber == 2 || valNumber == 3 )
	{
		switch ( obj->item_type )
		{
			default:
				break;

			case ITEM_SWITCH:
			case ITEM_LEVER:
			case ITEM_PULLCHAIN:
			case ITEM_BUTTON:
				value = dotted_to_vnum(ch->GetInRoom()->vnum, arg);
		}
	}

	if ( !can_omodify( ch, obj ) )
		return;

	obj->value[valNumber] = value;
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		obj->pIndexData->value[valNumber] = value;

	return;
}

void do_oset(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char buf  [MAX_STRING_LENGTH];
	char outbuf  [MAX_STRING_LENGTH];
	OBJ_DATA *obj, *tmpobj;
	ExtraDescData *ed;
	bool lockobj;
	const char *origarg = argument;

	int value, tmp;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't oset\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	if(!playerbuilderallowed(ch))
		return;

	switch( ch->substate )
	{
		default:
			break;

		case SUB_OBJ_EXTRA:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_oset: sub_obj_extra: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			/*
			 * hopefully the object didn't get extracted...
			 * if you're REALLY paranoid, you could always go through
			 * the object and index-object lists, searching through the
			 * extra_descr lists for a matching pointer...
			 */
			ed  = (ExtraDescData *) ch->dest_buf;
			ed->description_ = copy_buffer_string( ch );
			tmpobj = (OBJ_DATA *) ch->spare_ptr;
			stop_editing( ch );
			ch->dest_buf = tmpobj;
			ch->substate = ch->tempnum;
			return;

		case SUB_OBJ_LONG:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_oset: sub_obj_long: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			obj = (OBJ_DATA *) ch->dest_buf;
			if ( obj && obj_extracted(obj) )
			{
				send_to_char( "Your object was extracted!\r\n", ch );
				stop_editing( ch );
				return;
			}
			obj->longDesc_ = copy_buffer_string( ch );
			if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			{
				obj->pIndexData->longDesc_ = obj->longDesc_;
			}
			tmpobj = (OBJ_DATA *) ch->spare_ptr;
			stop_editing( ch );
			ch->substate = ch->tempnum;
			ch->dest_buf = tmpobj;
			return;
	}

	obj = NULL;
	argument = smash_tilde_static ( argument );

	if ( ch->substate == SUB_REPEATCMD )
	{
		obj = (OBJ_DATA *) ch->dest_buf;
		if ( obj && obj_extracted(obj) )
		{
			send_to_char( "Your object was extracted!\r\n", ch );
			obj = NULL;
			argument = "done";
		}
		if ( argument[0] == '\0' || !str_cmp( argument, " " )
				||   !str_prefix( argument, "stat" ) )
		{
			if ( obj )
				do_ostat( ch, (char*) obj->name_.c_str() );
			else
				send_to_char( "No object selected.  Type '?' for help.\r\n", ch );
			return;
		}
		if ( !str_prefix( argument, "done" ) || !str_prefix( argument, "off" ) )
		{
			send_to_char( "Oset mode off.\r\n", ch );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
			if ( ch->pcdata && ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			return;
		}
	}
	if ( obj )
	{
		lockobj = TRUE;
		strcpy( arg1, obj->name_.c_str() );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}
	else
	{
		lockobj = FALSE;
		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );
		strcpy( arg3, argument );
	}

	if ( !str_prefix( arg1, "on" ) )
	{
		send_to_char( "Syntax: oset <object|vnum> on.\r\n", ch );
		return;
	}

	if ( arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp( arg1, "?" ) )
	{
		if ( ch->substate == SUB_REPEATCMD )
		{
			if ( obj )
				send_to_char( "Syntax: <field>  <value>\r\n",		ch );
			else
				send_to_char( "Syntax: <object> <field>  <value>\r\n",	ch );
		}
		else
			send_to_char( "Syntax: oset <object> <field>  <value>\r\n",	ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Field being one of:\r\n",			ch );
		send_to_char( "  flags wear   level  weight cost   rent timer\r\n",	ch );
		send_to_char( "  name  short  long   ed     rmed   actiondesc\r\n",	ch );
		send_to_char( "  type  value0 value1 value2 value3 value4 value5\r\n",	ch );
		send_to_char( "  affect rmaffect layers\r\n",						ch );
		send_to_char( "  condition max_condition\r\n", 						ch );
		send_to_char( "For weapons:\r\n",									ch );
		send_to_char( "  weapontype damagetype mindamage maxdamage\r\n",	ch );
		send_to_char( "  ammotype	power      message\r\n",				ch );
		send_to_char( "For armor:\r\n",										ch );
		send_to_char( "  armor classification\r\n",							ch );
		send_to_char( "For scrolls, potions and pills:\r\n",				ch );
		send_to_char( "  slevel spell1 spell2 spell3\r\n",		ch );
		send_to_char( "For wands and staves:\r\n",			ch );
		send_to_char( "  slevel spell maxcharges charges\r\n",		ch );
		send_to_char( "For containers:          For levers and switches:\r\n", ch );
		send_to_char( "  cflags key capacity      tflags\r\n",		ch );
		send_to_char( "For gems:\r\n", ch);
		send_to_char( "  size quality basevalue flags spell charges\r\n", ch );
		return;
	}

	if ( !obj && get_trust(ch) < LEVEL_STONE_ADEPT )
	{
		if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You can't find that here.\r\n", ch );
			return;
		}
	}
	else
		if ( !obj )
		{
			if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
			{
				send_to_char( "There is nothing like that in all the realms.\r\n", ch );
				return;
			}
		}
	if ( lockobj )
		ch->dest_buf = obj;
	else
		ch->dest_buf = NULL;

	separate_obj( obj );
	value = atoi( arg3 );

	if ( !str_prefix( arg2, "on" ) )
	{
#if 0 /* Testaur - temporarily close this command */
		ch_printf( ch, "Oset mode on. (Editing '%s' vnum %s).\r\n",
				obj->name, vnum_to_dotted(obj->pIndexData->vnum) );
		ch->substate = SUB_REPEATCMD;
		ch->dest_buf = obj;
		if ( ch->pcdata )
		{
			if ( ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			sprintf( buf, "<&COset &W#%s&w> %%i", vnum_to_dotted(obj->pIndexData->vnum) );
			ch->pcdata->subprompt = STRALLOC( buf );
		}
#else
		send_to_char("Sorry, this command closed for repair.\r\n",ch);
#endif
		return;
	}


	if ( !str_prefix( arg2, "type" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> type <type>\r\n", ch );
			return;
		}

		value = itemtype_name_to_number( argument );
		if ( value < 1 )
		{
			ch_printf( ch, "Unknown type: %s\r\n", arg3 );
			return;
		}

		obj->item_type = (sh_int) value;

		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->item_type = obj->item_type;

		return;
	}

	if (obj->item_type == ITEM_NONE)
	{
		send_to_char( "Please set an item type before editing the object.\r\n", ch);
		return;
	}

	if ( !str_prefix( arg2, "condition" ) )
	{
		if ( !can_omodify( ch, obj) )
			return;

		obj->condition = value;
		return;
	}
	if ( !str_prefix( arg2, "max_condition" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;

		obj->max_condition = value;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->max_condition = value;
		return;
	}

	if ( !str_prefix( arg2, "value0" ) || !str_prefix( arg2, "v0" ) )
	{
		DoItemValue(0, ch, obj, arg3, value);
		return;
	}

	if ( !str_prefix( arg2, "value1" ) || !str_prefix( arg2, "v1" ) )
	{
		DoItemValue(1, ch, obj, arg3, value);
		return;
	}

	if ( !str_prefix( arg2, "value2" ) || !str_prefix( arg2, "v2" ) )
	{
		DoItemValue(2, ch, obj, arg3, value);
		return;
	}

	if ( !str_prefix( arg2, "value3" ) || !str_prefix( arg2, "v3" ) )
	{
		DoItemValue(3, ch, obj, arg3, value);
		return;
	}

	if ( !str_prefix( arg2, "value4" ) || !str_prefix( arg2, "v4" ) )
	{
		DoItemValue(4, ch, obj, arg3, value);
		return;
	}

	if ( !str_prefix( arg2, "value5" ) || !str_prefix( arg2, "v5" ) )
	{
		DoItemValue(5, ch, obj, arg3, value);
		return;
	}


	if ( !str_prefix( arg2, "flags" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> flags <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix = '\0';
			bool second_set; /* KSILYAN- for more flags */
			second_set = FALSE;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_oflag( arg3 );
			if ( value < 0 || value > MAX_O_FLAGS )
			{
				second_set = TRUE;
				value = get_oflag_2(arg3);
			}
			if ( value < 0 || ((value > MAX_O_FLAGS && second_set == FALSE) || (value > MAX_O_FLAGS_2 && second_set == TRUE)) )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( 1 << value == ITEM_PROTOTYPE
						&&   get_trust( ch ) < sysdata.level_modify_proto_flag
						&&   !is_name ("protoflag", ch->pcdata->bestowments_.c_str()) )
					send_to_char( "You cannot change the prototype flag.\r\n", ch );
				else
				{
					if ( prefix == '-' ) {
						REMOVE_BIT(second_set ? obj->extra_flags_2 : obj->extra_flags, 1 << value);
					} else if ( prefix == '+' ) {
						SET_BIT(second_set ? obj->extra_flags_2 : obj->extra_flags, 1 << value);
					} else {
						TOGGLE_BIT(second_set ? obj->extra_flags_2 : obj->extra_flags, 1 << value);
					}
					if ( 1 << value == ITEM_PROTOTYPE )
					{
						obj->pIndexData->extra_flags = obj->extra_flags;
						obj->pIndexData->extra_flags_2 = obj->extra_flags_2;
					}
				}
			}
		}
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			obj->pIndexData->extra_flags = obj->extra_flags;
			obj->pIndexData->extra_flags_2 = obj->extra_flags_2;
		}
		return;
	}

	if ( !str_prefix( arg2, "wear" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> wear <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg3 );
			if ( arg3[0] && (arg3[0] == '-' || arg3[0] == '+') ) {
				char *a = arg3; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_wflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT(obj->wear_flags, 1<<value );
				} else if ( prefix == '+' ) {
					SET_BIT(obj->wear_flags, 1<<value );
				} else {
					TOGGLE_BIT( obj->wear_flags, 1 << value );
				}
			}
		}

		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->wear_flags = obj->wear_flags;
		return;
	}

	if ( !str_prefix( arg2, "level" ) )
	{
#ifdef USE_OBJECT_LEVELS
		if ( !can_omodify( ch, obj ) )
			return;
		obj->level = value;
#else
		send_to_char("Use of this value has been depreciated.\n", ch);
#endif
		return;
	}

	if ( !str_prefix( arg2, "weight" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		obj->weight = value;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->weight = value;
		return;
	}

	if ( !str_prefix( arg2, "cost" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		obj->cost = value;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->cost = value;
		return;
	}

	if ( !str_prefix( arg2, "rent" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->rent = value;
		else
			send_to_char( "Item must have prototype flag to set this value.\r\n", ch );
		return;
	}

	if ( !str_prefix(arg2, "rarecount") )
	{
		if ( !can_omodify(ch, obj) )
			return;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
			obj->pIndexData->rare = value;
		else
			send_to_char("Item must have the prototype flag set to modify this value.\r\n", ch);
		return;
	}

	if ( !str_prefix( arg2, "layers" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->layers = value;
		else
			send_to_char( "Item must have prototype flag to set this value.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "timer" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		obj->timer = value;
		return;
	}

	if ( !str_prefix( arg2, "name" ) )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		obj->name_ = arg3;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			obj->pIndexData->name_ = obj->name_;
		}
		return;
	}

	if ( !str_prefix( arg2, "short" ) )
	{
		obj->shortDesc_ = arg3;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			obj->pIndexData->shortDesc_ = obj->shortDesc_;
		}
		else
			/* Feature added by Narn, Apr/96
			 * If the item is not proto, add the word 'rename' to the keywords
			 * if it is not already there.
			 */
		{
			if ( str_infix( "rename", obj->name_.c_str() ) )
			{
				sprintf( buf, "%s %s", obj->name_.c_str(), "rename" );
				obj->name_ = buf;
			}
		}
		return;
	}

	if ( !str_prefix( arg2, "actiondesc" ) )
	{
		if ( strstr( arg3, "%n" )
				||   strstr( arg3, "%d" )
				||   strstr( arg3, "%l" ) )
		{
			send_to_char( "Illegal characters!\r\n", ch );
			return;
		}
		obj->actionDesc_ = STRALLOC( arg3 );
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			obj->pIndexData->actionDesc_ = obj->actionDesc_;
		}
		return;
	}

	if ( !str_prefix( arg2, "long" ) )
	{
		if ( arg3[0] )
		{
			obj->longDesc_ = arg3;
			if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			{
				obj->pIndexData->longDesc_ = obj->longDesc_;
			}
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		if ( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;
		ch->substate = SUB_OBJ_LONG;
		ch->dest_buf = obj;
		start_editing( ch, obj->longDesc_.c_str() );
		return;
	}

	if ( !str_prefix( arg2, "affect" ) )
	{
		AFFECT_DATA *paf;
		sh_int loc;
		int bitv;

		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' || !argument || argument[0] == 0 )
		{
			send_to_char( "Usage: oset <object> affect <field> <value>\r\n", ch );
			return;
		}
		loc = get_atype( arg2 );
		if ( loc < 1 )
		{
			ch_printf( ch, "Unknown field: %s\r\n", arg2 );
			return;
		}
		if ( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
		{
			bitv = 0;
			while ( argument[0] != '\0' )
			{
				argument = one_argument( argument, arg3 );
				if ( loc == APPLY_AFFECT )
					value = get_aflag( arg3 );
				else
					value = get_risflag( arg3 );
				if ( value < 0 || value > 31 )
					ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
				else
					SET_BIT( bitv, 1 << value );
			}
			if ( !bitv )
				return;
			value = bitv;
		}
		else
		{
			argument = one_argument( argument, arg3 );
			value = atoi( arg3 );
		}
		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= -1;
		paf->duration		= -1;
		paf->location		= loc;
		paf->modifier		= value;
		paf->bitvector		= 0;
		paf->next		= NULL;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			LINK( paf, obj->pIndexData->first_affect,
					obj->pIndexData->last_affect, next, prev );
		else
			LINK( paf, obj->first_affect, obj->last_affect, next, prev );
		++top_affect;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "rmaffect" ) )
	{
		AFFECT_DATA *paf;
		sh_int loc, count;

		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: oset <object> rmaffect <affect#>\r\n", ch );
			return;
		}
		loc = atoi( argument );
		if ( loc < 1 )
		{
			send_to_char( "Invalid number.\r\n", ch );
			return;
		}

		count = 0;

		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			OBJ_INDEX_DATA *pObjIndex;

			pObjIndex = obj->pIndexData;
			for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
			{
				if ( ++count == loc )
				{
					UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
					DISPOSE( paf );
					send_to_char( "Removed.\r\n", ch );
					--top_affect;
					return;
				}
			}
			send_to_char( "Not found.\r\n", ch );
			return;
		}
		else
		{
			for ( paf = obj->first_affect; paf; paf = paf->next )
			{
				if ( ++count == loc )
				{
					UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
					DISPOSE( paf );
					send_to_char( "Removed.\r\n", ch );
					--top_affect;
					return;
				}
			}
			send_to_char( "Not found.\r\n", ch );
			return;
		}
	}

	if ( !str_prefix( arg2, "ed" ) )
	{
		if ( arg3[0] == '\0' )
		{
			send_to_char( "Syntax: oset <object> ed <keywords>\r\n",
					ch );
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		if ( obj->timer )
		{
			send_to_char("It's not safe to edit an extra description on an object with a timer.\r\nTurn it off first.\r\n", ch );
			return;
		}
		if ( obj->item_type == ITEM_PAPER )
		{
			send_to_char("You can not add an extra description to a note paper at the moment.\r\n", ch);
			return;
		}
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			ed = SetOExtraProto( obj->pIndexData, arg3 );
		else
			ed = SetOExtra( obj, arg3 );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		if ( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;
		ch->substate = SUB_OBJ_EXTRA;
		ch->dest_buf = ed;
		start_editing( ch, ed->description_.c_str() );
		return;
	}

	if ( !str_prefix( arg2, "rmed" ) )
	{
		if ( arg3[0] == '\0' )
		{
			send_to_char( "Syntax: oset <object> rmed <keywords>\r\n", ch );
			return;
		}
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
		{
			if ( DelOExtraProto( obj->pIndexData, arg3 ) )
				send_to_char( "Deleted.\r\n", ch );
			else
				send_to_char( "Not found.\r\n", ch );
			return;
		}
		if ( DelOExtra( obj, arg3 ) )
			send_to_char( "Deleted.\r\n", ch );
		else
			send_to_char( "Not found.\r\n", ch );
		return;
	}
	/*
	 * save some finger-leather
	 */
	if ( !str_cmp( arg2, "ris" ) )
	{
		sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
		do_oset( ch, outbuf );
		sprintf(outbuf, "%s affect immune %s", arg1, arg3);
		do_oset( ch, outbuf );
		sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
		do_oset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "r" ) )
	{
		sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
		do_oset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "i" ) )
	{
		sprintf(outbuf, "%s affect immune %s", arg1, arg3);
		do_oset( ch, outbuf );
		return;
	}
	if ( !str_cmp( arg2, "s" ) )
	{
		sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
		do_oset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "ri" ) )
	{
		sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
		do_oset( ch, outbuf );
		sprintf(outbuf, "%s affect immune %s", arg1, arg3);
		do_oset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "rs" ) )
	{
		sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
		do_oset( ch, outbuf );
		sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
		do_oset( ch, outbuf );
		return;
	}

	if ( !str_cmp( arg2, "is" ) )
	{
		sprintf(outbuf, "%s affect immune %s", arg1, arg3);
		do_oset( ch, outbuf );
		sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
		do_oset( ch, outbuf );
		return;
	}

	/*
	 * Make it easier to set special object values by name than number
	 * 						-Thoric
	 * - with additions by Ksilyan
	 */
	tmp = -1;
	switch( obj->item_type )
	{
		case ITEM_GEM:
			if ( !str_prefix( arg2, "basevalue" ) )
			{
				if ( !is_number(arg3) )
				{
					send_to_char( "Please enter a valid number!\r\n", ch);
					return;
				}
				value = atoi(arg3);
				if ( value < 0 )
				{
					send_to_char( "Please enter a number greater than 0.\r\n", ch);
					return;
				}
				tmp = OBJECT_GEM_VALUE;
				ch_printf(ch, "Gem's value set to %d.\r\n", value);
			}
			else if ( !str_prefix( arg2, "quality" ) )
			{
				if ( !is_number(arg3) )
				{
					send_to_char( "Please enter a valid number!\r\n", ch);
					return;
				}
				value = atoi(arg3);
				if ( value < 0 )
				{
					send_to_char( "Please enter a number greater than 0.\r\n", ch);
					return;
				}
				tmp = OBJECT_GEM_QUALITY;
				ch_printf(ch, "Gem's quality set to %d (%s).\r\n", value, gem_quality_number_to_name(value));
			}
			else if ( !str_prefix( arg2, "size" ) )
			{
				if ( !is_number(arg3) )
				{
					send_to_char( "Please enter a valid number!\r\n", ch);
					return;
				}
				value = atoi(arg3);
				if ( value < 0 )
				{
					send_to_char( "Please enter a number greater than 0.\r\n", ch);
					return;
				}
				tmp = OBJECT_GEM_SIZE;
				ch_printf(ch, "Gem's size set to %d (%s).\r\n", value, gem_size_number_to_name(value));
			}
			else if ( !str_prefix( arg2, "gemflags" ) )
			{
				if ( !is_number(arg3) )
				{
					send_to_char( "Please enter a valid number!\r\n", ch);
					return;
				}
				value = atoi(arg3);
				if ( value < 0 )
				{
					send_to_char( "Please enter a number greater than 0.\r\n", ch);
					return;
				}
				tmp = OBJECT_GEM_FLAGS;
				ch_printf(ch, "Gem's flags set to %d.\r\n", value);
			}
			else if ( !str_prefix( arg2, "spell" ) )
			{
				value = skill_lookup(arg3);
				if ( value <= 0 )
				{
					send_to_char( "Please enter a valid spell name!\r\n", ch);
					return;
				}
				tmp = OBJECT_GEM_SPELL;
				ch_printf(ch, "Gem's spell set to %d (%s).\r\n", value, skill_table[value]->name_.c_str());
			}
			else if ( !str_prefix( arg2, "charges" ) )
			{
				if ( !is_number(arg3) )
				{
					send_to_char( "Please enter a valid number!\r\n", ch);
					return;
				}
				value = atoi(arg3);
				if ( value < 0 )
				{
					send_to_char( "Please enter a number greater than 0.\r\n", ch);
					return;
				}
				tmp = OBJECT_GEM_CHARGES;
				ch_printf(ch, "Gem's charges set to %d.\r\n", value);
			}
			obj->value[tmp] = value;
			if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
				obj->pIndexData->value[tmp] = value;

			return;
		case ITEM_QUIVER:
			if ( !str_prefix( arg2, "ammotype" ) )
			{
				value = ammotype_name_to_number(arg3);
				if ( value == AMMO_NONE )
				{
					send_to_char( "Unknown ammo type.\r\n", ch);
					return;
				}

				obj->value[OBJECT_QUIVER_AMMOTYPE] = value;
				ch_printf(ch, "Object's ammo type set to %s.\r\n", arg3);
				if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
				{
					obj->pIndexData->value[OBJECT_QUIVER_AMMOTYPE] = value;
				}
			}
			else if ( !str_prefix( arg2, "capacity" ) )
			{
				obj->value[OBJECT_QUIVER_CAPACITY] = value;
				ch_printf( ch, "Object's capacity has been set to %d.\r\n", value);
				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_QUIVER_CAPACITY] = value;
			}
			return;
		case ITEM_SCABBARD:
			if ( !str_prefix( arg2, "weapontype" ) )
			{
				value = weapontype_name_to_number(arg3);

				if (value == WEAPON_NONE)
				{
					send_to_char( "Unknown weapon type.\r\n", ch);
					return;
				}

				obj->value[OBJECT_SCABBARD_WEAPONTYPE] = value;

				ch_printf(ch, "Scabbard's weapon type set to %s.\r\n", arg3);

				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
				{
					obj->pIndexData->value[OBJECT_SCABBARD_WEAPONTYPE] = value;
				}
			}
			return;
		case ITEM_WEAPON:
			if ( !str_prefix( arg2, "weapontype" ) )
			{
				value = weapontype_name_to_number(arg3);

				if ( value == WEAPON_NONE )
				{
					send_to_char( "Unknown weapon type.\r\n", ch );
					return;
				}

				obj->value[OBJECT_WEAPON_WEAPONTYPE] = value;

				if (IS_RANGED_WEAPON(value))
				{
					obj->value[OBJECT_WEAPON_DAMAGETYPE] = weapontype_to_ammotype(value);
					ch_printf( ch, "Ammo type defaulted to %s.\r\n",
							ammotype_number_to_name(obj->value[OBJECT_WEAPON_DAMAGETYPE]) );
				}
				else
				{
					obj->value[OBJECT_WEAPON_POWER] = 0;
					send_to_char( "Object power set to 0.\r\n", ch);
				}
				ch_printf( ch, "Object's weapon type has been set to %s.\r\n", arg3 );

				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
				{
					if (IS_RANGED_WEAPON(value))
					{
						obj->pIndexData->value[OBJECT_WEAPON_DAMAGETYPE] = weapontype_to_ammotype(value);
					}
					else
					{
						obj->pIndexData->value[OBJECT_WEAPON_POWER] = 0;
					}

					obj->pIndexData->value[OBJECT_WEAPON_WEAPONTYPE] = value;
				}
				return;
				break;
			}
			if ( !str_prefix( arg2, "damagetype" ) )
			{
				if (IS_RANGED_WEAPON(obj->value[OBJECT_WEAPON_WEAPONTYPE]))
				{
					send_to_char( "Please use ammotype for ranged weapons.\r\n", ch);
					return;
				}
				value = damagetype_name_to_number(arg3);

				obj->value[OBJECT_WEAPON_DAMAGETYPE] = value;
				ch_printf( ch, "Object's damage type has been set to %s.\r\n", damagetype_number_to_name(value) );

				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_WEAPON_DAMAGETYPE] = value;
				return;
				break;
			}
			if ( !str_prefix( arg2, "mindamage" ) )
			{
				if (IS_RANGED_WEAPON(obj->value[OBJECT_WEAPON_WEAPONTYPE]))
				{
					send_to_char( "You cannot set the damage for a missile weapon. Use power instead.\r\n", ch);
					return;
				}
				obj->value[OBJECT_WEAPON_MINDAMAGE] = value;
				ch_printf( ch, "Object's minimum damage has been set to %d.\r\n", value);

				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_WEAPON_MINDAMAGE] = value;
				return;
				break;
			}
			if ( !str_prefix( arg2, "maxdamage" ) )
			{
				if (IS_RANGED_WEAPON(obj->value[OBJECT_WEAPON_WEAPONTYPE]))
				{
					send_to_char( "You cannot set the damage for a missile weapon. Use power instead.\r\n", ch);
					return;
				}
				obj->value[OBJECT_WEAPON_MAXDAMAGE] = value;
				ch_printf( ch, "Object's maximum damage has been set to %d.\r\n", value);
				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_WEAPON_MAXDAMAGE] = value;
				return;
				break;
			}
			if ( !str_prefix( arg2, "ammotype" ) )
			{
				if (!IS_RANGED_WEAPON(obj->value[OBJECT_WEAPON_WEAPONTYPE]))
				{
					send_to_char( "Please use damage type for melee weapons.\r\n", ch);
					return;
				}
				value = ammotype_name_to_number(arg3);
				obj->value[OBJECT_WEAPON_DAMAGETYPE] = value;
				ch_printf( ch, "Object's ammotype has been set to %s.\r\n", ammotype_number_to_name(value));
				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_WEAPON_DAMAGETYPE] = value;
				return;
				break;
			}
			if ( !str_prefix( arg2, "power" ) )
			{
				if (!IS_RANGED_WEAPON(obj->value[OBJECT_WEAPON_WEAPONTYPE]))
				{
					send_to_char( "You cannot set the power for a melee weapon.\r\n", ch);
					return;
				}
				obj->value[OBJECT_WEAPON_POWER] = value;
				ch_printf( ch, "Object's power has been set to %d.\r\n", value);
				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_WEAPON_POWER] = value;
				return;
				break;
			}
			if ( !str_prefix( arg2, "message" ) || !str_prefix( arg2, "msg" ) )
			{
				if (IS_RANGED_WEAPON(obj->value[OBJECT_WEAPON_WEAPONTYPE]))
				{
					send_to_char( "You cannot set the damage message of a missile weapon.\r\n", ch);
					return;
				}
				value = damagemsg_name_to_number(arg3);
				if (value == DAMAGE_MSG_NONE)
				{
					send_to_char( "Unknown damage message.", ch);
					return;
				}
				obj->value[OBJECT_WEAPON_DAMAGEMESSAGE] = value;
				ch_printf( ch, "Object's damage message has been set to %s.\r\n", arg3);
				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_WEAPON_DAMAGEMESSAGE] = value;
				return;
				break;
			}
			break;
		case ITEM_PROJECTILE:
			if ( !str_prefix( arg2, "ammotype" ) )
			{
				value = ammotype_name_to_number(arg3);
				if (value == AMMO_NONE)
				{
					send_to_char( "Unknown ammo type.", ch);
					return;
				}
				obj->value[OBJECT_PROJECTILE_AMMOTYPE] = value;
				ch_printf( ch, "Object's ammo type has been set to %s.\r\n", arg3);
				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
					obj->pIndexData->value[OBJECT_PROJECTILE_AMMOTYPE] = value;
				return;
			}
			if ( !str_prefix( arg2, "mindamage" ))	tmp = OBJECT_PROJECTILE_MINDAMAGE;
			if ( !str_prefix( arg2, "maxdamage" ))	tmp = OBJECT_PROJECTILE_MAXDAMAGE;
			if ( !str_prefix( arg2, "range" ))		tmp = OBJECT_PROJECTILE_RANGE;
			if ( !str_prefix( arg2, "message" ) || !str_prefix( arg2, "msg" ) )
			{
				value = damagemsg_name_to_number(arg3);
				if (value == DAMAGE_MSG_NONE)
				{
					send_to_char( "Unknown damage message.", ch);
					return;
				}
				obj->value[OBJECT_PROJECTILE_DAMAGEMESSAGE] = value;
				ch_printf( ch, "Object's damage message has been set to %d.\r\n", value);
				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_PROJECTILE_DAMAGEMESSAGE] = value;
				return;
				break;
			}
			if ( !str_prefix( arg2, "damagetype" ) )
			{
				value = damagetype_name_to_number(arg3);

				obj->value[OBJECT_PROJECTILE_DAMAGETYPE] = value;
				ch_printf( ch, "Object's damage type has been set to %s.\r\n", damagetype_number_to_name(value) );

				if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
					obj->pIndexData->value[OBJECT_PROJECTILE_DAMAGETYPE] = value;
				return;
				break;
			}
			break;
		case ITEM_ARMOR:
			if ( !str_prefix( arg2, "ac" ) || !str_prefix(arg2, "armor")	)		tmp = OBJECT_ARMOR_AC;
			if ( !str_prefix( arg2, "classification" ) ) tmp = OBJECT_ARMOR_CLASSIFICATION;
			break;
		case ITEM_COMPONENT:
			if ( !str_prefix( arg2, "quantity" ) ) tmp = OBJECT_COMPONENT_QUANTITY;
			break;
		case ITEM_SALVE:
			if ( !str_prefix( arg2, "slevel"   ) )		tmp = 0;
			if ( !str_prefix( arg2, "maxdoses" ) )		tmp = 1;
			if ( !str_prefix( arg2, "doses"    ) )		tmp = 2;
			if ( !str_prefix( arg2, "delay"    ) )		tmp = 3;
			if ( !str_prefix( arg2, "spell1"   ) )		tmp = 4;
			if ( !str_prefix( arg2, "spell2"   ) )		tmp = 5;
			if ( tmp >=4 && tmp <= 5 )			value = skill_lookup(arg3);
			break;
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			if ( !str_prefix( arg2, "slevel" ) )		tmp = 0;
			if ( !str_prefix( arg2, "spell1" ) )		tmp = 1;
			if ( !str_prefix( arg2, "spell2" ) )		tmp = 2;
			if ( !str_prefix( arg2, "spell3" ) )		tmp = 3;
			if ( tmp >=1 && tmp <= 3 )			value = skill_lookup(arg3);
			break;
		case ITEM_STAFF:
		case ITEM_WAND:
			if ( !str_prefix( arg2, "slevel" ) )		tmp = 0;
			if ( !str_prefix( arg2, "spell" ) )
			{
				tmp = 3;
				value = skill_lookup(arg3);
			}
			if ( !str_prefix( arg2, "maxcharges" )	)	tmp = 1;
			if ( !str_prefix( arg2, "charges" ) )		tmp = 2;
			break;
		case ITEM_CONTAINER:
			if ( !str_prefix( arg2, "capacity" ) )		tmp = 0;
			if ( !str_prefix( arg2, "cflags" ) )		tmp = 1;
			if ( !str_prefix( arg2, "key" ) )		tmp = 2;
			break;
		case ITEM_SWITCH:
		case ITEM_LEVER:
		case ITEM_PULLCHAIN:
		case ITEM_BUTTON:
			if ( !str_prefix( arg2, "tflags" ) )
			{
				tmp = 0;
				value = get_trigflag(arg3);
			}

			// Added televnum for levers etc.
			if ( !str_prefix( arg2, "televnum" ) )
			{
				if ( IS_SET( obj->value[0], TRIG_TELEPORT )
					||   IS_SET( obj->value[0], TRIG_TELEPORTALL )
					||   IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
				{
					tmp = 1;
				}
				else
				{
					ch->sendText("Can't set televnum of non-teleport lever/button/etc.\n\r");
					return;
				}
			}
			break;
	}

	if ( tmp >= 0 && tmp <= 5 )
	{
		if ( !can_omodify( ch, obj ) )
			return;
		obj->value[tmp] = value;
		if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[tmp] = value;
		send_to_char("Done.\r\n", ch);
		return;
	}

	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_oset;
	}
	else
		do_oset( ch, "" );
	return;
}


void do_oreport(CHAR_DATA *ch, const char* argument)
{
	Area * area = ch->GetInRoom()->area;

	ostringstream os;

	os << "   Object Report                        " << endl;
	os << "\"" << area->name << "\"" << endl;
	os << "----------------------------------------" << endl;

	for (int vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++)
	{
		OBJ_INDEX_DATA * pObjIndex = get_obj_index(vnum);

		if ( pObjIndex != NULL)
		{
			string name;
			string layer;
			string location;
			string rare;
			int		ac, ac_or_damage;
			int		min_damage, max_damage, average_damage;
			int		max_condition;
			int		weight;
			int		hp, mana, move;
			int		number_of_specials;
			int		num_stats, num_res, num_susc, num_imm;
			string  area_name;
			ostringstream notes;
			string  vnum_str;
			int		newflags;

			notes << "Notes: ";

			Object * obj = create_object( pObjIndex, 0 );

			name = obj->name_.c_str();
			ac_or_damage = 0;


			switch(obj->item_type)
			{
				default:
					continue; break;

				case ITEM_ARMOR:
					newflags = obj->wear_flags;
					newflags &= ~(ITEM_TAKE);
					location = flag_string(newflags, w_flags);

					ac = obj->value[OBJECT_ARMOR_AC];
					ac_or_damage = ac;

					switch(obj->pIndexData->layers)
					{
						default:  layer = "Unknown!"; break;
						case 0:   layer = "none";     break;
						case 1:   layer = "1";        break;
						case 2:   layer = "2";        break;
						case 4:   layer = "4";        break;
						case 8:   layer = "8";        break;
						case 16:  layer = "16";       break;
						case 32:  layer = "32";       break;
						case 64:  layer = "64";       break;
						case 128: layer = "128";      break;
					}

					break;

				case ITEM_WEAPON:
				{
					location = "Weapon";
					min_damage = 0;
					max_damage = 0;
					min_damage = obj->value[OBJECT_WEAPON_MINDAMAGE];
					max_damage = obj->value[OBJECT_WEAPON_MAXDAMAGE];
					average_damage = (min_damage + max_damage) / 2;
					notes << "Min: " << min_damage << " Max: " << max_damage << " ";

					layer = weapontype_number_to_name(obj->value[OBJECT_WEAPON_WEAPONTYPE]);

					ac_or_damage = average_damage;

					break;
				}

				case ITEM_PROJECTILE:
				{
					location = "Weapon";
					min_damage = 0;
					max_damage = 0;
					min_damage = obj->value[OBJECT_PROJECTILE_MINDAMAGE];
					max_damage = obj->value[OBJECT_PROJECTILE_MAXDAMAGE];
					average_damage = (min_damage + max_damage) / 2;

					notes << "Min: " << min_damage << " Max: " << max_damage << " ";

					switch(obj->value[OBJECT_PROJECTILE_AMMOTYPE])
					{
						default:
							layer = ammotype_number_to_name(obj->value[OBJECT_PROJECTILE_AMMOTYPE]);
							break;
						case AMMO_LONGARROW:	case AMMO_BOLT:
							layer = "Long arrow / bolt";
							break;
					}

					break;
				}

				case ITEM_LIGHT:
					location = "lights";
					layer = "1";
					break;

				case ITEM_HOLD:
					location = "Held";
					layer = "1";
					break;
			}

			vnum_str = vnum_to_dotted(obj->pIndexData->vnum);
			max_condition = obj->max_condition;
			weight = obj->pIndexData->weight;

			if ( obj->pIndexData->rare == 0 )
				rare = "No";
			else if ( obj->pIndexData->rare < 0 || obj->pIndexData->rare > 20 )
				rare = "(bad value)";
			else
			{
				ostringstream buf;
				buf << obj->pIndexData->rare;
				rare = buf.str();
			}

			number_of_specials = 0;
			hp = mana = move = 0;
			num_stats = num_res = num_susc = num_imm = 0;

			for ( AFFECT_DATA * paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
			{
				switch(paf->location)
				{
					default:
					{
						notes << ". Af " << affect_loc_name(paf->location) << " by " << paf->modifier << ".";
						break;
					}

					case APPLY_STR:	case APPLY_INT:	case APPLY_CON:	case APPLY_CHA:
					case APPLY_DEX:	case APPLY_WIS:	case APPLY_LCK:
					{
						number_of_specials++;
						num_stats++;
						notes << ". " << affect_loc_name(paf->location) << ": " << paf->modifier << ".";

						break;
					}

					case APPLY_HIT:		number_of_specials++;	hp += paf->modifier;    break;
					case APPLY_MANA:	number_of_specials++;	mana += paf->modifier;  break;
					case APPLY_MOVE:	number_of_specials++;	move += paf->modifier;  break;

					case APPLY_RESISTANT:
					{
						number_of_specials++;
						num_res++;
						notes << ". Resist " << ris_number_to_name(paf->modifier) << ".";
						break;
					}

					case APPLY_SUSCEPTIBLE:
					{
						num_susc++;
						number_of_specials++;
						notes << ". Suscept " << ris_number_to_name(paf->modifier) << ".";
						break;
					}
					case APPLY_IMMUNE:
					{
						num_imm++;
						number_of_specials++;
						notes << ". Immune " << ris_number_to_name(paf->modifier) << ".";
						break;
					}
				}

				if ( paf->duration > -1 )
				{
					notes << "(duration: " << paf->duration << ").";
				}
			}

			notes << "-- " << extra_bit_name(obj->pIndexData->extra_flags);

			os << "\"" << name << "\" (#" << vnum_str << "): type: " << location << ", layer: "
				<< layer << ", Rare: " << rare << endl;

			os << "Condition: " << max_condition << ", Weight: "
				<< weight << ", HP/Mana/Mv bonus: " << hp << "/" << mana << "/" << move << endl;

			os << "# of Bonuses: stats (" << num_stats << ") res(" << num_res << ") susc("
				<< num_susc << ") imm(" << num_imm << ")" << endl;

			os << notes.str() << endl;

			if (obj->pIndexData->item_type == ITEM_WEAPON)
				os << "Min/max damage: " << obj->pIndexData->value[OBJECT_WEAPON_MINDAMAGE] << "/"
					<< obj->pIndexData->value[OBJECT_WEAPON_MAXDAMAGE] << endl;

			if (obj->pIndexData->item_type == ITEM_ARMOR)
				os << "Armor value: " << obj->pIndexData->value[OBJECT_ARMOR_AC] << endl;

			/*flags = obj->pIndexData->extra_flags;
			fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", IS_SET(flags, ITEM_ORGANIC) ? 1 : 0, 	IS_SET(flags, ITEM_METAL) ? 1 : 0,
					IS_SET(flags, ITEM_GLOW) ? 1 : 0, 		IS_SET(flags, ITEM_HUM) ? 1 : 0, 		IS_SET(flags, ITEM_DARK) ? 1 : 0,
					IS_SET(flags, ITEM_EVIL) ? 1 : 0,		IS_SET(flags, ITEM_INVIS) ? 1 : 0,		IS_SET(flags, ITEM_MAGIC) ? 1 : 0,
					IS_SET(flags, ITEM_NODROP) ? 1 : 0,		IS_SET(flags, ITEM_BLESS) ? 1 : 0,		IS_SET(flags, ITEM_INVENTORY) ? 1 : 0);

			fprintf( fp, "\n" );*/
			os << "---------------------------" << endl;
			extract_obj(obj , FALSE);

		}       /* if */
	}

	/* close file */

	ch->sendText( os.str() );
}



/*
 * Returns value 0 - 9 based on directional text.
 */
int get_dir( const char *txt )
{
    int edir;
    char c1,c2;

    if ( !str_cmp( txt, "northeast" ) )
      return DIR_NORTHEAST;
    if ( !str_cmp( txt, "northwest" ) )
      return DIR_NORTHWEST;
    if ( !str_cmp( txt, "southeast" ) )
      return DIR_SOUTHEAST;
    if ( !str_cmp( txt, "southwest" ) )
      return DIR_SOUTHWEST;
    if ( !str_cmp( txt, "somewhere" ) )
      return 10;

    c1 = txt[0];
    if ( c1 == '\0' )
      return 0;
    c2 = txt[1];
    edir = 0;
    switch ( c1 )
    {
	  case 'n':
	  switch ( c2 )
	  {
		    default:   edir = 0; break;	/* north */
		    case 'e':  edir = 6; break; /* ne	 */
		    case 'w':  edir = 7; break; /* nw	 */
	  }
	  break;    case '0':  edir = 0; break; /* north */
	  case 'e': case '1':  edir = 1; break; /* east  */
	  case 's':
	  switch ( c2 )
	  {
		    default:   edir = 2; break; /* south */
		    case 'e':  edir = 8; break; /* se	 */
		    case 'w':  edir = 9; break; /* sw	 */
	  }
	  break;    case '2':  edir = 2; break; /* south */
	  case 'w': case '3':  edir = 3; break; /* west	 */
	  case 'u': case '4':  edir = 4; break; /* up	 */
	  case 'd': case '5':  edir = 5; break; /* down	 */
		    case '6':  edir = 6; break; /* ne	 */
		    case '7':  edir = 7; break; /* nw	 */
		    case '8':  edir = 8; break; /* se	 */
		    case '9':  edir = 9; break; /* sw	 */
		    case '?':  edir = 10;break; /* somewhere */
    }
    return edir;
}

char *sprint_reset( CHAR_DATA *ch, RESET_DATA *pReset, sh_int num, bool rlist )
{
    static char buf[MAX_STRING_LENGTH];
    char mobname[MAX_STRING_LENGTH];
    char roomname[MAX_STRING_LENGTH];
    char objname[MAX_STRING_LENGTH];
    static ROOM_INDEX_DATA *room;
    static OBJ_INDEX_DATA *obj, *obj2;
    static MOB_INDEX_DATA *mob;
    int rvnum;

    if ( ch->GetInRoom() )
      rvnum = ch->GetInRoom()->vnum;
    if ( num == 1 )
    {
	room = NULL;
	obj  = NULL;
	obj2 = NULL;
	mob  = NULL;
    }

    switch( pReset->command )
    {
	default:
	  sprintf( buf, "%2d) *** BAD RESET: %c %d %d %d %d ***\r\n",
	  			num,
	  			pReset->command,
	  			pReset->extra,
	  			pReset->arg1,
	  			pReset->arg2,
	  			pReset->arg3 );
	  break;
	case 'M':
	  mob = get_mob_index( pReset->arg1 );
	  room = get_room_index( pReset->arg3 );
	  if ( mob )
	    strcpy( mobname, mob->playerName_.c_str() );
	  else
	    strcpy( mobname, "Mobile: *BAD VNUM*" );
	  if ( room )
	    strcpy( roomname, room->name_.c_str() );
	  else
	    strcpy( roomname, "Room: *BAD VNUM*" );
	  sprintf( buf, "%2d) %s (%d) -> %s (%d) [%d]\r\n",
	  			num,
	  			mobname,
	  			pReset->arg1,
	  			roomname,
	  			pReset->arg3,
	  			pReset->arg2 );
	  break;
	case 'E':
	  if ( !mob )
	      strcpy( mobname, "* ERROR: NO MOBILE! *" );
	  if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	      strcpy( objname, obj->name_.c_str() );
	  sprintf( buf, "%2d) %s (%d) -> %s (%s) [%d]\r\n",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			mobname,
	  			wear_locs[pReset->arg3],
	  			pReset->arg2 );
	  break;
	case 'H':
	  if ( pReset->arg1 > 0
	  &&  (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	  if ( !obj )
	      strcpy( objname, "Object: *NULL obj*" );
	  sprintf( buf, "%2d) Hide %s (%d)\r\n",
	  			num,
	  			objname,
	  			obj ? obj->vnum : pReset->arg1 );
	  break;
	case 'G':
	  if ( !mob )
	      strcpy( mobname, "* ERROR: NO MOBILE! *" );
	  if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	      strcpy( objname, obj->name_.c_str() );
	  sprintf( buf, "%2d) %s (%d) -> %s (carry) [%d]\r\n",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			mobname,
	  			pReset->arg2 );
	  break;
	case 'O':
	  if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object: *BAD VNUM*" );
	  else
	      strcpy( objname, obj->name_.c_str() );
	  room = get_room_index( pReset->arg3 );
	  if ( !room )
	      strcpy( roomname, "Room: *BAD VNUM*" );
	  else
	      strcpy( roomname, room->name_.c_str() );
	  sprintf( buf, "%2d) (object) %s (%d) -> %s (%d) [%d]\r\n",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			roomname,
	  			pReset->arg3,
	  			pReset->arg2 );
	  break;
	case 'P':
	  if ( (obj2 = get_obj_index( pReset->arg1 )) == NULL )
	      strcpy( objname, "Object1: *BAD VNUM*" );
	  else
	      strcpy( objname, obj2->name_.c_str() );
	  if ( pReset->arg3 > 0
	  &&  (obj = get_obj_index( pReset->arg3 )) == NULL )
	      strcpy( roomname, "Object2: *BAD VNUM*" );
	  else
	  if ( !obj )
	      strcpy( roomname, "Object2: *NULL obj*" );
	  else
	      strcpy( roomname, obj->name_.c_str() );
	  sprintf( buf, "%2d) (Put) %s (%d) -> %s (%d) [%d]\r\n",
	  			num,
	  			objname,
	  			pReset->arg1,
	  			roomname,
	  			obj ? obj->vnum : pReset->arg3,
	  			pReset->arg2 );
	  break;
	case 'D':
	  if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR+1 )
		pReset->arg2 = 0;
	  if ( (room = get_room_index( pReset->arg1 )) == NULL )
	  {
		strcpy( roomname, "Room: *BAD VNUM*" );
		sprintf( objname, "%s (no exit)",
				dir_name[pReset->arg2] );
	  }
	  else
	  {
		strcpy( roomname, room->name_.c_str() );
		sprintf( objname, "%s%s",
				dir_name[pReset->arg2],
		get_exit(room,pReset->arg2) ? "" : " (NO EXIT!)" );
	  }
	  switch( pReset->arg3 )
	  {
	    default:	strcpy( mobname, "(* ERROR *)" );	break;
	    case 0:	strcpy( mobname, "Open" );		break;
	    case 1:	strcpy( mobname, "Close" );		break;
	    case 2:	strcpy( mobname, "Close and lock" );	break;
	  }
	  sprintf( buf, "%2d) %s [%d] the %s [%d] door %s (%d)\r\n",
	  			num,
	  			mobname,
	  			pReset->arg3,
	  			objname,
	  			pReset->arg2,
	  			roomname,
	  			pReset->arg1 );
	  break;
	case 'R':
	  if ( (room = get_room_index( pReset->arg1 )) == NULL )
		strcpy( roomname, "Room: *BAD VNUM*" );
	  else
		strcpy( roomname, room->name_.c_str() );
	  sprintf( buf, "%2d) Randomize exits 0 to %d -> %s (%d)\r\n",
	  			num,
	  			pReset->arg2,
	  			roomname,
	  			pReset->arg1 );
	  break;
	case 'T':
	  sprintf( buf, "%2d) TRAP: %d %d %d %d (%s)\r\n",
	  		num,
			pReset->extra,
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			flag_string(pReset->extra, trap_flags) );
	  break;
    }
    if ( rlist && (!room || (room && room->vnum != rvnum)) )
	return NULL;
    return buf;
}

void do_redit(CHAR_DATA *ch, const char* argument)
{
	char arg [MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf [MAX_STRING_LENGTH];
	ROOM_INDEX_DATA	*location, *tmp;
	ExtraDescData	*ed;
	//char		dir;
	ExitData		*xit, *texit;
	int			value;
	int			edir, ekey, evnum;
	const char		*origarg = argument;

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor.\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
		default:
			break;
		case SUB_ROOM_DESC:
			location = (ROOM_INDEX_DATA *) ch->dest_buf;
			if ( !location )
			{
				bug( "redit: sub_room_desc: NULL ch->dest_buf", 0 );
				location = ch->GetInRoom();
			}
			location->description_ = copy_buffer_string( ch );
			stop_editing( ch );
			ch->substate = ch->tempnum;
			return;
		case SUB_ROOM_EXTRA:
			ed = (ExtraDescData *) ch->dest_buf;
			if ( !ed )
			{
				bug( "redit: sub_room_extra: NULL ch->dest_buf", 0 );
				stop_editing( ch );
				return;
			}
			ed->description_ = copy_buffer_string( ch );
			stop_editing( ch );
			ch->substate = ch->tempnum;
			return;
	}

	location = ch->GetInRoom();

	if(!playerbuilderallowed(ch))
		return;

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg );

	if ( ch->substate == SUB_REPEATCMD )
	{
		if ( arg[0] == '\0' )
		{
			do_rstat( ch, "" );
			return;
		}
		if ( !str_prefix( arg, "done" ) || !str_prefix( arg, "off" ) )
		{
			send_to_char( "Redit mode off.\r\n", ch );
			if ( ch->pcdata && ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			ch->substate = SUB_NONE;
			return;
		}
	}
	if ( arg[0] == '\0' || !str_cmp( arg, "?" ) )
	{
		if ( ch->substate == SUB_REPEATCMD )
			send_to_char( "Syntax: <field> value\r\n",			ch );
		else
			send_to_char( "Syntax: redit <field> value\r\n",		ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Field being one of:\r\n",			ch );
		send_to_char( "  name desc ed rmed\r\n",			ch );
		send_to_char( "  exit bexit exdesc exflags exname exkey\r\n",	ch );
		send_to_char( "  bexflags bexkey bexname\r\n", ch);
		send_to_char( "  flags sector teledelay televnum tunnel\r\n",	ch );
		send_to_char( "  rlist exdistance\r\n",				ch );
		send_to_char( "  random_type random_number\r\n",    ch );
		return;
	}

	if ( !can_rmodify( ch, location ) )
		return;

	if ( !str_prefix( arg, "on" ) )
	{
		if(playerbuildercannot(ch))
			return;
		send_to_char( "Redit mode on.\r\n", ch );
		ch->substate = SUB_REPEATCMD;
		if ( ch->pcdata )
		{
			if ( ch->pcdata->subprompt )
				STRFREE( ch->pcdata->subprompt );
			ch->pcdata->subprompt = STRALLOC( "<&CRedit &W#%r&w> %i" );
		}
		return;
	}
	/* Commented by Ksilyan
	   if ( !str_prefix( arg, "substate" ) )
	   {
	   if(playerbuildercannot(ch))
	   return;
	   argument = one_argument( argument, arg2);
	   if( !str_prefix( arg2, "north" )  )
	   {
	   ch->inter_substate = SUB_NORTH;
	   return;
	   }
	   if( !str_prefix( arg2, "east" )  )
	   {
	   ch->inter_substate = SUB_EAST;
	   return;
	   }
	   if( !str_prefix( arg2, "south" )  )
	   {
	   ch->inter_substate = SUB_SOUTH;
	   return;
	   }
	   if( !str_prefix( arg2, "west" )  )
	   {
	   ch->inter_substate = SUB_WEST;
	   return;
	   }
	   if( !str_prefix( arg2, "up" )  )
	   {
	   ch->inter_substate = SUB_UP;
	   return;
	   }
	   if( !str_prefix( arg2, "down" )  )
	   {
	   ch->inter_substate = SUB_DOWN;
	   return;
	   }
	   send_to_char( " unrecognized substate in redit\r\n", ch);
	   return;
	   }*/


	if ( !str_prefix( arg, "name" ) )
	{
		if ( argument[0] == '\0' )
		{
			send_to_char( "Set the room name.  A very brief single line room description.\r\n", ch );
			send_to_char( "Usage: redit name <Room summary>\r\n", ch );
			return;
		}
		location->name_ = argument;
		return;
	}

	if ( !str_prefix(arg, "scent" ) )
	{
		if ( argument[0] == '\0' )
		{
			send_to_char("Set the rooms scent.  Use scentedit list for a list of all descriptions.\n\r", ch);
			send_to_char("Set scent to 0 for no scent.\r\n", ch);
			return;
		}

		location->scentID = atoi(argument);
		return;
	}

	if ( !str_prefix(arg, "description"))
	{
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_DESC;
		ch->dest_buf = location;
		start_editing( ch, location->description_.c_str() );
		return;
	}

	if ( !str_prefix( arg, "tunnel" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the maximum characters allowed in the room at one time. (0 = unlimited).\r\n", ch );
			send_to_char( "Usage: redit tunnel <value>\r\n", ch );
			return;
		}
		location->tunnel = URANGE( 0, atoi(argument), 1000 );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg, "ed" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Create an extra description.\r\n", ch );
			send_to_char( "You must supply keyword(s).\r\n", ch );
			return;
		}
		CHECK_SUBRESTRICTED( ch );
		ed = SetRExtra( location, argument );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_EXTRA;
		ch->dest_buf = ed;
		start_editing( ch, ed->description_.c_str() );
		return;
	}

	if ( !str_prefix( arg, "rmed" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Remove an extra description.\r\n", ch );
			send_to_char( "You must supply keyword(s).\r\n", ch );
			return;
		}
		if ( DelRExtra( location, argument ) )
			send_to_char( "Deleted.\r\n", ch );
		else
			send_to_char( "Not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg, "rlist" ) )
	{
		RESET_DATA *pReset;
		char *bptr;
		AREA_DATA *tarea;
		sh_int num;

		tarea = location->area;
		if ( !tarea->first_reset )
		{
			send_to_char( "This area has no resets to list.\r\n", ch );
			return;
		}
		num = 0;
		for ( pReset = tarea->first_reset; pReset; pReset = pReset->next )
		{
			num++;
			if ( (bptr = sprint_reset( ch, pReset, num, TRUE )) == NULL )
				continue;
			send_to_char( bptr, ch );
		}
		return;
	}

	if(!str_prefix(arg, "flags")) {
		if(!argument || argument[0] == '\0') {
			send_to_char("Toggle the room flags.\r\n", ch);
			send_to_char("Usage: redit flags <flag> [flag]...\r\n", ch);
			return;
		}

		while(argument[0] != '\0') {
			char prefix = ' ';
			argument = one_argument(argument, arg2);

			if(arg2[0] &&(arg2[0] == '-' || arg2[0] == '+')) {
				char *a = arg2;
				char *b = a;
				++b;
				prefix = *a;
				while((*a++ = *b++));
			}

			value = get_rflag(arg2);

			if(value < 0 || value > 31)
				ch_printf(ch, "Unknown flag: %s\r\n", arg2);
			else {
				if( (1 << value == ROOM_PROTOTYPE
							&&   get_trust(ch) < sysdata.level_modify_proto_flag)
						|| (1 << value == ROOM_NOFLOOR
							&&   get_trust(ch) < LEVEL_IMMORTAL)
				  )
					send_to_char("You cannot change that flag.\r\n", ch);
				else {
					if(prefix == '-') {
						REMOVE_BIT(location->room_flags, 1 << value);
					} else if(prefix == '+') {
						SET_BIT(location->room_flags, 1 << value);
					} else {
						TOGGLE_BIT(location->room_flags, 1 << value);
					}

					/* Hard Kludge to update auction room list dynamically.
					 * Not that you can't remove rooms from the list currently,
					 * only a reboot/copyover will reset the list.
					 * 08.08.2000 -- Warp
					 */
					if(1<< value == ROOM_AUCTIONROOM)
						add_auctionroom(location);
				}
			}
		}
		return;
	}

	if ( !str_prefix( arg, "teledelay") )
	{
		if(playerbuildercannot(ch))
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the delay of the teleport. (0 = off).\r\n", ch );
			send_to_char( "Usage: redit teledelay <value>\r\n", ch );
			return;
		}
		location->tele_delay = atoi( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg, "televnum" ) )
	{
		if(playerbuildercannot(ch))
			return;
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the vnum of the room to teleport to.\r\n", ch );
			send_to_char( "Usage: redit televnum <vnum>\r\n", ch );
			return;
		}
		evnum=dotted_to_vnum(ch->GetInRoom()->vnum, argument);
		if(playerbuilderbadvnum(ch,evnum))
			return;
		location->tele_vnum = evnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	/*
	   KSILYAN
	   Random room stuff
	 */
	if ( !str_prefix( arg, "random_type" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the random room type.\r\n", ch);
			send_to_char( "Usage: redit random_type <value>\r\n", ch);
			return;
		}
		location->random_room_type = atoi( argument );
		if ( location->random_room_type < 0 || location->random_room_type > MAX_RANDOM_DESCRIPTION_TYPE )
		{
			location->random_room_type = 0;
			send_to_char( "No such type; set to normal room.\r\n", ch);
		}
		else
		{
			location->random_description = number_range( 0 , location->area->random_description_counts[location->random_room_type] - 1);
		}
		send_to_char( "Done.\r\n", ch);
		return;
	}

	if ( !str_prefix( arg, "sector" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Set the sector type.\r\n", ch );
			send_to_char( "Usage: redit sector [sector name]\r\n", ch );
			return;
		}
		location->sector_type = sector_name_to_number(argument);
		if ( location->sector_type < 0 || location->sector_type >= SECT_MAX )
		{
			location->sector_type = 1;
			send_to_char( "Out of range\r\n.", ch );
		}
		else
			send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg, "exkey" ) )
	{
		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );
		if ( arg2[0] == '\0' || arg3[0] == '\0' )
		{
			send_to_char( "Usage: redit exkey <dir> <key vnum>\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		value = dotted_to_vnum( ch->GetInRoom()->vnum, arg3 );
		if ( !xit )
		{
			send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
			return;
		}
		xit->key = value;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg, "exname" ) )
	{
		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Change or clear exit keywords.\r\n", ch );
			send_to_char( "Usage: redit exname <dir> [keywords]\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if ( !xit )
		{
			send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
			return;
		}
		xit->keyword_ = argument;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg, "exflags" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Toggle or display exit flags.\r\n", ch );
			send_to_char( "Usage: redit exflags <dir> <flag> [flag]...\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if ( !xit )
		{
			send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
			return;
		}
		if ( argument[0] == '\0' )
		{
			sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\r\n[ ",
					xit->vdir, xit->keyword_.c_str(), xit->key );
			for ( value = 0; value <= MAX_EXFLAG; value++ )
			{
				if ( IS_SET( xit->exit_info, 1 << value ) )
				{
					strcat( buf, ex_flags[value] );
					strcat( buf, " " );
				}
			}
			strcat( buf, "]\r\n" );
			send_to_char( buf, ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			char prefix;
			argument = one_argument( argument, arg2 );
			if ( arg2[0] && (arg2[0] == '-' || arg2[0] == '+') ) {
				char *a = arg2; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}
			value = get_exflag( arg2 );
			if ( value < 0 || value > MAX_EXFLAG )
				ch_printf( ch, "Unknown flag: %s\r\n", arg2 );
			else
			{
				if ( prefix == '-' ) {
					REMOVE_BIT( xit->exit_info, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT( xit->exit_info, 1 << value );
				} else {
					TOGGLE_BIT( xit->exit_info, 1 << value );
				}
			}
		}
		return;
	}


	/* Ibuild commented by Ksilyan
	   if ( !str_prefix( arg, "ex_flags" ) )
	   {
	   argument = one_argument( argument, arg2 );
	   switch(ch->inter_substate)
	   {
	   case SUB_EAST : dir = 'e'; edir = 1; break;
	   case SUB_WEST : dir = 'w'; edir = 3; break;
	   case SUB_SOUTH: dir = 's'; edir = 2; break;
	   case SUB_UP   : dir = 'u'; edir = 4; break;
	   case SUB_DOWN : dir = 'd'; edir = 5; break;
	   default:
	   case SUB_NORTH: dir = 'n'; edir = 0; break;
	   }

	   value = get_exflag(arg2);
	   if ( value < 0 )
	   {
	   send_to_char("Bad exit flag. \r\n", ch);
	   return;
	   }
	   if ( (xit = get_exit(location,edir)) == NULL )
	   {
	   sprintf(buf,"exit %c 1",dir);
	   do_redit(ch,buf);
	   xit = get_exit(location,edir);
	   }
	   TOGGLE_BIT( xit->exit_info, value );
	   return;
	   }


	   if ( !str_prefix( arg, "ex_to_room" ) )
	   {
	   argument = one_argument( argument, arg2 );
	   switch(ch->inter_substate)
	   {
	   case SUB_EAST : dir = 'e'; edir = 1; break;
	   case SUB_WEST : dir = 'w'; edir = 3; break;
	   case SUB_SOUTH: dir = 's'; edir = 2; break;
	   case SUB_UP   : dir = 'u'; edir = 4; break;
	   case SUB_DOWN : dir = 'd'; edir = 5; break;
	   default:
	   case SUB_NORTH: dir = 'n'; edir = 0; break;
	   }

	   evnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg2);

	   if ( evnum < 1 || evnum > 1048575999 )
	   {
	   send_to_char( "Invalid room number.\r\n", ch );
	   return;
	   }
	   if ( (tmp = get_room_index( evnum )) == NULL )
	   {
	   send_to_char( "Non-existant room.\r\n", ch );
	   return;
	   }
	   if( playerbuilderbadvnum(ch,evnum))
	   return;

	   if ( (xit = get_exit(location,edir)) == NULL )
	   {
	   sprintf(buf,"exit %c 1",dir);
	   do_redit(ch,buf);
	   xit = get_exit(location,edir);
	   }
	   xit->vnum = evnum;
	   return;
	   }

	   if ( !str_prefix( arg, "ex_key" ) )
	{
		argument = one_argument( argument, arg2 );
		switch(ch->inter_substate)
		{
			case SUB_EAST : dir = 'e'; edir = 1; break;
			case SUB_WEST : dir = 'w'; edir = 3; break;
			case SUB_SOUTH: dir = 's'; edir = 2; break;
			case SUB_UP   : dir = 'u'; edir = 4; break;
			case SUB_DOWN : dir = 'd'; edir = 5; break;
			default:
			case SUB_NORTH: dir = 'n'; edir = 0; break;
		}
		if ( (xit = get_exit(location,edir)) == NULL )
		{
			sprintf(buf,"exit %c 1",dir);
			do_redit(ch,buf);
			xit = get_exit(location,edir);
		}
		xit->key = atoi( arg2 );
		return;
	}

	if ( !str_prefix( arg, "ex_exdesc" ) )
	{
		switch(ch->inter_substate)
		{
			case SUB_EAST : dir = 'e'; edir = 1; break;
			case SUB_WEST : dir = 'w'; edir = 3; break;
			case SUB_SOUTH: dir = 's'; edir = 2; break;
			case SUB_UP   : dir = 'u'; edir = 4; break;
			case SUB_DOWN : dir = 'd'; edir = 5; break;
			default:
			case SUB_NORTH: dir = 'n'; edir = 0; break;
		}
		if ( (xit = get_exit(location, edir)) == NULL )
		{
			sprintf(buf,"exit %c 1",dir);
			do_redit(ch,buf);
		}
		sprintf(buf,"exdesc %c %s",dir,argument);
		do_redit(ch,buf);
		return;
	}

	if ( !str_prefix( arg, "ex_keywords" ) )  *//* not called yet *//*
	{
		switch(ch->inter_substate)
		{
			case SUB_EAST : dir = 'e'; edir = 1; break;
			case SUB_WEST : dir = 'w'; edir = 3; break;
			case SUB_SOUTH: dir = 's'; edir = 2; break;
			case SUB_UP   : dir = 'u'; edir = 4; break;
			case SUB_DOWN : dir = 'd'; edir = 5; break;
			default:
			case SUB_NORTH: dir = 'n'; edir = 0; break;
		}
		if ( (xit = get_exit(location, edir)) == NULL )
		{
			sprintf(buf, "exit %c 1", dir);
			do_redit(ch,buf);
			if ( (xit = get_exit(location, edir)) == NULL )
				return;
		}
		sprintf( buf, "%s %s", xit->keyword, argument );
		STRFREE( xit->keyword );
		xit->keyword = STRALLOC( buf );
		return;
	}*/

	if ( !str_prefix( arg, "mine" ) )
	{
		// Add, Delete, Edit etc.
		argument = one_argument( argument, arg2 );

		// The vnum argument
		argument = one_argument( argument, arg3 );

		if ( arg2[0] == '\0')
		{
			send_to_char( "Usage: redit mine <add/delete/edit> <vnum>\r\n", ch);
			return;
		}

		if ( !str_prefix(arg2, "add" ) )
		{
			if ( arg3[0] == '\0' )
			{
				send_to_char("Usage: redit mine add <vnum> <rarity>\r\n", ch);
				return;
			}

			location->mineMap[dotted_to_vnum(0, arg3)] = atoi(argument);
			send_to_char("Done.\n\r", ch);
			return;
		}

		if ( !str_prefix(arg2, "delete" ) )
		{
			if ( arg3[0] == '\0' )
			{
				send_to_char("Usage: redit mine delete <vnum>\r\n", ch);
				return;
			}

			location->mineMap.erase(dotted_to_vnum(0, arg3));
			send_to_char("Done.\n\r", ch);
			return;
		}

		if ( !str_prefix(arg2, "edit" ) )
		{
			if ( arg3[0] == '\0' )
			{
				send_to_char("Usage: Redit mine edit <vnum> <rarity>\r\n", ch);
				return;
			}

			location->mineMap[dotted_to_vnum(0, arg3)] = atoi(argument);
			send_to_char("Done.\n\r", ch);
			return;
		}
	}

	if ( !str_prefix( arg, "exit" ) )
	{
		bool addexit, numnotdir;

		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Create, change or remove an exit.\r\n", ch );
			send_to_char( "Usage: redit exit <dir> [room] [flags] [key] [keywords]\r\n", ch );
			return;
		}
		addexit = numnotdir = FALSE;
		switch( arg2[0] )
		{
			default:	edir = get_dir(arg2);			  break;
			case '+':	edir = get_dir(arg2+1);	addexit = TRUE;	  break;
			case '#':	edir = atoi(arg2+1);	numnotdir = TRUE; break;
		}
		if ( arg3[0] == '\0' )
			evnum = 0;
		else
			evnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg3);
		if ( numnotdir )
		{
			if ( (xit = get_exit_num(location, edir)) != NULL )
				edir = xit->vdir;
		}
		else
			xit = get_exit(location, edir);
		if ( !evnum )
		{
			if ( xit )
			{
				extract_exit(location, xit);
				send_to_char( "Exit removed.\r\n", ch );
				return;
			}
			send_to_char( "No exit in that direction.\r\n", ch );
			return;
		}
		if ( evnum < 1 || evnum > 1048575999 )
		{
			send_to_char( "Invalid room number.\r\n", ch );
			return;
		}
		if ( (tmp = get_room_index( evnum )) == NULL )
		{
			send_to_char( "Non-existant room.\r\n", ch );
			return;
		}
		if(playerbuilderbadvnum(ch,evnum))
			return;
		if ( addexit || !xit )
		{
			if ( numnotdir )
			{
				send_to_char( "Cannot add an exit by number, sorry.\r\n", ch );
				return;
			}
			if ( addexit && xit && get_exit_to(location, edir, tmp->vnum) )
			{
				send_to_char( "There is already an exit in that direction leading to that location.\r\n", ch );
				return;
			}
			xit = make_exit( location, tmp, edir );
			xit->keyword_		= "";
			xit->description_		= "";
			xit->key			= -1;
			xit->exit_info		= 0;
			act( AT_IMMORT, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
		}
		else
			act( AT_IMMORT, "Something is different...", ch, NULL, NULL, TO_ROOM );
		if ( xit->to_room != tmp )
		{
			xit->to_room = tmp;
			xit->vnum = evnum;
			texit = get_exit_to( xit->to_room, rev_dir[edir], location->vnum );
			if ( texit )
			{
				texit->rexit = xit;
				xit->rexit = texit;
			}
		}
		argument = one_argument( argument, arg3 );
		if ( arg3[0] != '\0' )
			xit->exit_info = atoi( arg3 );
		if ( argument && argument[0] != '\0' )
		{
			one_argument( argument, arg3 );
			ekey = dotted_to_vnum(ch->GetInRoom()->vnum, arg3);
			if ( ekey != 0 || arg3[0] == '0' )
			{
				argument = one_argument( argument, arg3 );
				xit->key = ekey;
			}
			if ( argument && argument[0] != '\0' )
			{
				xit->keyword_ = argument;
			}
		}
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix(arg, "bexkey") ) {
		ExitData *xit, *rxit;
		ROOM_INDEX_DATA *tmploc;
		int vnum;

		argument = one_argument(argument, arg2);
		argument = one_argument(argument, arg3);

		if ( arg2[0] == '\0' || arg3[0] == '\0' ) {
			send_to_char("Usage: redit bexkey <dir> <vnum>\r\n", ch);
			return;
		}

		edir = get_dir(arg2);

		tmploc = location;
		xit = get_exit(tmploc, edir);
		rxit = NULL;
		vnum = 0;

		if ( !xit ) {
			send_to_char("No exit in that direction.\r\n", ch);
			return;
		}

		rxit = get_exit(xit->to_room, rev_dir[xit->vdir]);

		if ( !rxit ) {
			send_to_char("No return exit.\r\n", ch);
			return;
		}

		xit->key = dotted_to_vnum(ch->GetInRoom()->vnum, arg3);
		rxit->key = xit->key;

		send_to_char("Done.\r\n", ch);
		return;
	}


	if ( !str_prefix(arg, "bexname") ) {
		ExitData *xit, *rxit;
		ROOM_INDEX_DATA *tmploc;
		int vnum;

		argument = one_argument(argument, arg2);

		if ( arg2[0] == '\0' ) {
			send_to_char("Usage: redit bexname <dir> <name>\r\n", ch);
			return;
		}

		edir = get_dir(arg2);

		tmploc = location;
		xit = get_exit(tmploc, edir);
		rxit = NULL;
		vnum = 0;

		if ( !xit ) {
			send_to_char("No exit in that direction.\r\n", ch);
			return;
		}

		rxit = get_exit(xit->to_room, rev_dir[xit->vdir]);

		if ( !rxit ) {
			send_to_char("No return exit.\r\n", ch);
			return;
		}

		xit->keyword_  = argument;
		rxit->keyword_ = argument;

		send_to_char("Done.\r\n", ch);
		return;
	}



	if ( !str_prefix(arg, "bexflags") ) {
		ExitData *xit, *rxit;
		ROOM_INDEX_DATA *tmploc;
		int vnum;

		argument = one_argument(argument, arg2);

		if ( arg2[0] == '\0' ) {
			send_to_char("Usage: redit bexflags <dir> [flags]\r\n", ch);
			return;
		}

		edir = get_dir(arg2);

		tmploc = location;
		xit = get_exit(tmploc, edir);
		rxit = NULL;
		vnum = 0;

		if ( !xit ) {
			send_to_char("No exit in that direction.\r\n", ch);
			return;
		}

		rxit = get_exit(xit->to_room, rev_dir[xit->vdir]);

		if ( !rxit ) {
			send_to_char("No return exit.\r\n", ch);
			return;
		}

		if ( argument[0] == '\0' ) {
			sprintf(buf, "Flags for exit: Keywords: %s  Key: %d\r\n[ ",
					xit->keyword_.c_str(), xit->key);

			for ( value = 0; value <= MAX_EXFLAG; value++ ) {
				if ( IS_SET(xit->exit_info, 1 << value ) )
				{
					strcat(buf, ex_flags[value]);
					strcat(buf, " ");
				}
			}
			strcat(buf, "]\r\n");
			send_to_char(buf, ch);

			sprintf(buf, "Flags for return exit: Keywords: %s  Key: %d\r\n[ ",
					rxit->keyword_.c_str(), rxit->key);

			for ( value = 0; value <= MAX_EXFLAG; value++ ) {
				if ( IS_SET(rxit->exit_info, 1 << value )  ) {
					strcat(buf, ex_flags[value]);
					strcat(buf, " ");
				}
			}

			strcat(buf, "]\r\n");
			send_to_char(buf, ch);

			return;
		}

		while ( argument[0] != '\0' ) {
			char prefix;
			int value;

			argument = one_argument(argument, arg2);

			if ( arg2[0] && (arg2[0] == '-' || arg2[0] == '+') ) {
				char *a = arg2; char *b = a; ++b; prefix = *a;
				while((*a++=*b++));
			}

			value = get_exflag(arg2);

			if ( value < 0 || value > MAX_EXFLAG ) {
				ch_printf(ch, "Unknown flag: %s\r\n", arg2);
			} else {
				if ( prefix == '-' ) {
					REMOVE_BIT( xit->exit_info, 1 << value );
					REMOVE_BIT(rxit->exit_info, 1 << value );
				} else if ( prefix == '+' ) {
					SET_BIT   ( xit->exit_info, 1 << value );
					SET_BIT   (rxit->exit_info, 1 << value );
				} else {
					TOGGLE_BIT( xit->exit_info, 1 << value );
					TOGGLE_BIT(rxit->exit_info, 1 << value );
				}
			}
		}
		send_to_char("Done.\r\n", ch);
		return;
	}

	/*
	 * Twisted and evil, but works				-Thoric
	 * Makes an exit, and the reverse in one shot.
	 */
	if ( !str_prefix( arg, "bexit" ) )
	{
		ExitData *xit, *rxit;
		char tmpcmd[MAX_INPUT_LENGTH];
		ROOM_INDEX_DATA *tmploc;
		int vnum, exnum;
		char rvnum[MAX_INPUT_LENGTH];
		bool numnotdir;

		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Create, change or remove a two-way exit.\r\n", ch );
			send_to_char( "Usage: redit bexit <dir> [room] [flags] [key] [keywords]\r\n", ch );
			return;
		}
		numnotdir = FALSE;
		switch( arg2[0] )
		{
			default:
				edir = get_dir( arg2 );
				break;
			case '#':
				numnotdir = TRUE;
				edir = atoi( arg2+1 );
				break;
			case '+':
				edir = get_dir( arg2+1 );
				break;
		}
		tmploc = location;
		exnum = edir;
		if ( numnotdir )
		{
			if ( (xit = get_exit_num(tmploc, edir)) != NULL )
				edir = xit->vdir;
		}
		else
			xit = get_exit(tmploc, edir);
		rxit = NULL;
		vnum = 0;
		rvnum[0] = '\0';
		if ( xit )
		{
			vnum = xit->vnum;
			if ( arg3[0] != '\0' )
				sprintf( rvnum, "%d", tmploc->vnum );
			if ( xit->to_room )
				rxit = get_exit(xit->to_room, rev_dir[edir]);
			else
				rxit = NULL;
		}
		sprintf( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
		do_redit( ch, tmpcmd );
		if ( numnotdir )
			xit = get_exit_num(tmploc, exnum);
		else
			xit = get_exit(tmploc, edir);
		if ( !rxit && xit )
		{
			vnum = xit->vnum;
			if ( arg3[0] != '\0' )
				sprintf( rvnum, "%d", tmploc->vnum );
			if ( xit->to_room )
				rxit = get_exit(xit->to_room, rev_dir[edir]);
			else
				rxit = NULL;
		}
		if ( vnum )
		{
			sprintf( tmpcmd, "%d redit exit %d %s %s",
					vnum,
					rev_dir[edir],
					rvnum,
					argument );
			do_at( ch, tmpcmd );
		}
		return;
	}

	if ( !str_prefix( arg, "exdistance" ) )
	{
		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Set the distance (in rooms) between this room, and the destination room.\r\n", ch );
			send_to_char( "Usage: redit exdistance <dir> [distance]\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if ( xit )
		{
			xit->distance = URANGE( 1, atoi(argument), 50 );
			send_to_char( "Done.\r\n", ch );
			return;
		}
		send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg, "exdesc" ) )
	{
		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Create or clear a description for an exit.\r\n", ch );
			send_to_char( "Usage: redit exdesc <dir> [description]\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi( arg2+1 );
			xit = get_exit_num( location, edir );
		}
		else
		{
			edir = get_dir( arg2 );
			xit = get_exit( location, edir );
		}
		if ( xit )
		{
			if ( !argument || argument[0] == '\0' )
				xit->description_ = "";
			else
			{
				sprintf( buf, "%s\r\n", argument );
				xit->description_ = buf;
			}
			send_to_char( "Done.\r\n", ch );
			return;
		}
		send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\r\n", ch );
		return;
	}

	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_redit;
	}
	else
		do_redit( ch, "" );
	return;
}

void do_ocreate(CHAR_DATA *ch, const char* argument)
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*obj;
    int			 vnum, cvnum;

    if ( IS_NPC(ch) )
    {
	send_to_char( "Mobiles cannot create.\r\n", ch );
	return;
    }

    if(!playerbuilderallowed(ch))
      return;

    argument = one_argument( argument, arg );

    vnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg);

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: ocreate <vnum> [copy vnum] <item name>\r\n", ch );
	return;
    }

    if ( vnum < 1 || vnum > 1048576000 )
    {
	send_to_char( "Bad number.\r\n", ch );
	return;
    }

    one_argument( argument, arg2 );
    cvnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg2 );
    if ( cvnum < 1 )
      cvnum = 0;
    if ( cvnum != 0 )
    {
      argument = one_argument( argument, arg2 );
      if(playerbuilderbadvnum(ch,cvnum))
        return;
    }
    if ( get_obj_index( vnum ) )
    {
	send_to_char( "An object with that number already exists.\r\n", ch );
	return;
    }

    if ( IS_NPC( ch ) )
      return;
    if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
    {
	AREA_DATA *pArea;

	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to create objects.\r\n", ch );
	  return;
	}
	if ( vnum < pArea->low_o_vnum
	&&   vnum > pArea->hi_o_vnum )
	{
	  send_to_char( "That number is not in your allocated range.\r\n", ch );
	  return;
	}
    }

    pObjIndex = make_object( vnum, cvnum, argument );
    if ( !pObjIndex )
    {
	send_to_char( "Error.\r\n", ch );
	log_string( "do_ocreate: make_object failed." );
	return;
    }
    obj = create_object( pObjIndex, get_trust(ch) );
    obj_to_char( obj, ch );
    act( AT_IMMORT, "$n makes some ancient arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
    act( AT_IMMORT, "You make some ancient arcane gestures, and open your hands to reveal $p!", ch, obj, NULL, TO_CHAR );
}

void do_mcreate(CHAR_DATA *ch, const char* argument)
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*mob;
    int			 vnum, cvnum;

    if ( IS_NPC(ch) )
    {
	send_to_char( "Mobiles cannot create.\r\n", ch );
	return;
    }

    if(!playerbuilderallowed(ch))
      return;

    argument = one_argument( argument, arg );

    vnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg);
    if(playerbuilderbadvnum(ch,vnum))
      return;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: mcreate <vnum> [cvnum] <mobile name>\r\n", ch );
	return;
    }

    if ( vnum < 1 || vnum > 1048576000 )
    {
	send_to_char( "Bad number.\r\n", ch );
	return;
    }

    one_argument( argument, arg2 );
    cvnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg2);
    if ( cvnum < 1 )
        cvnum = 0;
    if ( cvnum != 0 )
    {
        argument = one_argument( argument, arg2 );
        if(playerbuilderbadvnum(ch,cvnum))
          return;
    }
    if ( get_mob_index( vnum ) )
    {
	send_to_char( "A mobile with that number already exists.\r\n", ch );
	return;
    }

    if ( IS_NPC( ch ) )
      return;
    if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
    {
	AREA_DATA *pArea;

	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to create mobiles.\r\n", ch );
	  return;
	}
	if ( vnum < pArea->low_m_vnum
	&&   vnum > pArea->hi_m_vnum )
	{
	  send_to_char( "That number is not in your allocated range.\r\n", ch );
	  return;
	}
    }

    pMobIndex = make_mobile( vnum, cvnum, argument );
    if ( !pMobIndex )
    {
	send_to_char( "Error.\r\n", ch );
	log_string( "do_mcreate: make_mobile failed." );
	return;
    }
    mob = create_mobile( pMobIndex );
    char_to_room( mob, ch->GetInRoom() );
    act( AT_IMMORT, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
    act( AT_IMMORT, "You wave your arms about, and $N appears at your command!", ch, NULL, mob, TO_CHAR );
}


/*
 * Simple but nice and handle line editor.			-Thoric
 */
void edit_buffer( CHAR_DATA *ch, const char *argument )
{
    PlayerConnection * d;
    EDITOR_DATA *edit;
    char cmd[MAX_INPUT_LENGTH];
    char buf[MAX_BUFFER_SIZE];
    sh_int x, line, max_buf_lines;
    bool save;

    if ( (d = ch->GetConnection()) == NULL )
    {
	send_to_char( "You have no descriptor.\r\n", ch );
	return;
    }

   if ( d->ConnectedState != CON_EDITING )
   {
	send_to_char( "You can't do that!\r\n", ch );
	bug( "Edit_buffer: d->connected != CON_EDITING", 0 );
	return;
   }

   if ( ch->substate <= SUB_PAUSE )
   {
	send_to_char( "You can't do that!\r\n", ch );
	bug( "Edit_buffer: illegal ch->substate (%d)", ch->substate );
	d->ConnectedState = CON_PLAYING;
	return;
   }

   if ( !ch->editor )
   {
	send_to_char( "You can't do that!\r\n", ch );
	bug( "Edit_buffer: null editor", 0 );
	d->ConnectedState = CON_PLAYING;
	return;
   }

   edit = ch->editor;
   save = FALSE;
   max_buf_lines = MAX_BUFFER_LINES;

   if ( argument[0] == '/' || argument[0] == '\\' )
   {
	one_argument( argument, cmd );
	if ( !str_cmp( cmd+1, "?" ) )
	{
	    send_to_char( "Editing commands\r\n---------------------------------\r\n", ch );
	    send_to_char( "/l              list buffer\r\n",	ch );
	    send_to_char( "/c              clear buffer\r\n",	ch );
	    send_to_char( "/d [line]       delete line\r\n",	ch );
	    send_to_char( "/g <line>       goto line\r\n",	ch );
	    send_to_char( "/i <line>       insert line\r\n",	ch );
	    send_to_char( "/r <old> <new>  global replace\r\n",	ch );
	    send_to_char( "/a              abort editing\r\n",	ch );
	    if ( get_trust(ch) > LEVEL_IMMORTAL )
	      send_to_char( "/! <command>    execute command (do not use another editing command)\r\n",  ch );
	    send_to_char( "/s              save buffer\r\n\r\n> ",ch );
	    return;
	}
	if ( !str_cmp( cmd+1, "c" ) )
	{
	    memset( edit, '\0', sizeof(EDITOR_DATA) );
	    edit->numlines = 0;
	    edit->on_line   = 0;
	    send_to_char( "Buffer cleared.\r\n> ", ch );
	    return;
	}
	if ( !str_cmp( cmd+1, "r" ) )
	{
	    char word1[MAX_INPUT_LENGTH];
	    char word2[MAX_INPUT_LENGTH];
	    char *sptr, *wptr, *lwptr;
	    int x, count, wordln, word2ln, lineln;

	    sptr = one_argument( argument, word1 );
	    sptr = one_argument( sptr, word1 );
	    sptr = one_argument( sptr, word2 );
	    if ( word1[0] == '\0' || word2[0] == '\0' )
	    {
		send_to_char( "Need word to replace, and replacement.\r\n> ", ch );
		return;
	    }
	    if ( strcmp( word1, word2 ) == 0 )
	    {
		send_to_char( "Done.\r\n> ", ch );
		return;
	    }
	    count = 0;  wordln = strlen(word1);  word2ln = strlen(word2);
	    ch_printf( ch, "Replacing all occurrences of %s with %s...\r\n", word1, word2 );
	    for ( x = edit->on_line; x < edit->numlines; x++ )
	    {
		lwptr = edit->line[x];
		while ( (wptr = strstr( lwptr, word1 )) != NULL )
		{
		    sptr = lwptr;
		    lwptr = wptr + wordln;
		    sprintf( buf, "%s%s", word2, wptr + wordln );
		    lineln = wptr - edit->line[x] - wordln;
		    ++count;
		    if ( strlen(buf) + lineln > 79 )
		    {
			lineln = UMAX(0, (79 - strlen(buf)));
			buf[lineln] = '\0';
			break;
		    }
		    else
			lineln = strlen(buf);
		    buf[lineln] = '\0';
		    strcpy( wptr, buf );
		}
	    }
	    ch_printf( ch, "Found and replaced %d occurrence(s).\r\n> ", count );
	    return;
	}

	if ( !str_cmp( cmd+1, "i" ) )
	{
            if ( edit->numlines >= max_buf_lines-2 )
		send_to_char( "Buffer is full.\r\n> ", ch );
	    else
	    {
		if ( argument[2] == ' ' )
		  line = atoi( argument + 2 ) - 1;
		else
		  line = edit->on_line;
		if ( line < 0 )
		  line = edit->on_line;
		if ( line < 0 || line > edit->numlines )
		  send_to_char( "Out of range.\r\n> ", ch );
		else
		{
		  for ( x = ++edit->numlines; x > line; x-- )
			strcpy( edit->line[x], edit->line[x-1] );
		  strcpy( edit->line[line], "" );
		  send_to_char( "Line inserted.\r\n> ", ch );
		}
 	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "d" ) )
	{
	    if ( edit->numlines == 0 )
		send_to_char( "Buffer is empty.\r\n> ", ch );
	    else
	    {
		if ( argument[2] == ' ' )
		  line = atoi( argument + 2 ) - 1;
		else
		  line = edit->on_line;
		if ( line < 0 )
		  line = edit->on_line;
		if ( line < 0 || line > edit->numlines )
		  send_to_char( "Out of range.\r\n> ", ch );
		else
		{
		  if ( line == 0 && edit->numlines == 1 )
		  {
			memset( edit, '\0', sizeof(EDITOR_DATA) );
			edit->numlines = 0;
			edit->on_line   = 0;
			send_to_char( "Line deleted.\r\n> ", ch );
			return;
		  }
		  for ( x = line; x < (edit->numlines - 1); x++ )
			strcpy( edit->line[x], edit->line[x+1] );
		  strcpy( edit->line[edit->numlines--], "" );
		  if ( edit->on_line > edit->numlines )
		    edit->on_line = edit->numlines;
		  send_to_char( "Line deleted.\r\n> ", ch );
		}
 	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "g" ) )
	{
	    if ( edit->numlines == 0 )
		send_to_char( "Buffer is empty.\r\n> ", ch );
	    else
	    {
		if ( argument[2] == ' ' )
		  line = atoi( argument + 2 ) - 1;
		else
		{
		    send_to_char( "Goto what line?\r\n> ", ch );
		    return;
		}
		if ( line < 0 )
		  line = edit->on_line;
		if ( line < 0 || line > edit->numlines )
		  send_to_char( "Out of range.\r\n> ", ch );
		else
		{
		  edit->on_line = line;
		  ch_printf( ch, "(On line %d)\r\n> ", line+1 );
		}
 	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "l" ) )
	{
	    if ( edit->numlines == 0 )
	      send_to_char( "Buffer is empty.\r\n> ", ch );
	    else
	    {
	      send_to_char( "------------------\r\n", ch );
	      for ( x = 0; x < edit->numlines; x++ )
		 ch_printf_nocolor( ch, "%2d> %s\r\n", x+1, edit->line[x] );
	      send_to_char( "------------------\r\n> ", ch );
	    }
	    return;
	}
	if ( !str_cmp( cmd+1, "a" ) )
	{
	    send_to_char( "\r\nAborting... ", ch );
	    stop_editing( ch );
	    return;
	}
	if ( get_trust(ch) > LEVEL_IMMORTAL && !str_cmp( cmd+1, "!" ) )
	{
	    DO_FUN *last_cmd;
	    int substate = ch->substate;

	    last_cmd = ch->last_cmd;
	    ch->substate = SUB_RESTRICTED;
	    interpret(ch, argument+3);
	    ch->substate = substate;
	    ch->last_cmd = last_cmd;
	    set_char_color( AT_GREEN, ch );
	    send_to_char( "\r\n> ", ch );
	    return;
	}
	if ( !str_cmp( cmd+1, "s" ) )
	{
	    d->ConnectedState = CON_PLAYING;
	    if ( !ch->last_cmd )
	      return;
	    (*ch->last_cmd) ( ch, "" );
	    return;
	}
   }

   if ( edit->size + strlen(argument) + 1 >= MAX_BUFFER_SIZE - 8 )
   {
        send_to_char( "Buffer full.\r\n", ch );
        save = TRUE;
   }
   else
   {
	if ( strlen(argument) >= MAX_BUFFER_COLS )
	{
	  strncpy( buf, argument, MAX_BUFFER_COLS );
	  buf[MAX_BUFFER_COLS-1] = 0;
	  ch_printf(ch, "Long line trimmed to:\r\n\t%s\r\n> ", buf );
	}
	else
	  strcpy( buf, argument );
	strcpy( edit->line[edit->on_line++], buf );
	if ( edit->on_line > edit->numlines )
	  edit->numlines++;
        if ( edit->numlines >= max_buf_lines-2 )
	{
          edit->numlines = max_buf_lines-2;
	  send_to_char( "Buffer full.\r\n", ch );
	  save = TRUE;
	}
   }

   if ( save )
   {
      d->ConnectedState = CON_PLAYING;
      if ( !ch->last_cmd )
        return;
      (*ch->last_cmd) ( ch, "" );
      return;
   }
   send_to_char( "> ", ch );
}

void free_reset( AREA_DATA *are, RESET_DATA *res )
{
    UNLINK( res, are->first_reset, are->last_reset, next, prev );
    DISPOSE( res );
}

void free_area( AREA_DATA *are )
{
	DISPOSE( are->name );
	DISPOSE( are->filename );
	while ( are->first_reset )
		free_reset( are, are->first_reset );
	delete are;
	are = NULL;
}

void assign_area( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char taf[1024];
	AREA_DATA *tarea, *tmp;
	bool created = FALSE;

	if ( IS_NPC( ch ) )
		return;
	if ( /* get_trust( ch ) > LEVEL_HERO_MAX     Testaur allows it
			&& */  ch->pcdata->r_range_lo
			&&   ch->pcdata->r_range_hi )
	{
		tarea = ch->pcdata->area;
		sprintf( taf, "%s.are", capitalize( ch->getName().c_str() ) );
		if ( !tarea )
		{
			for ( tmp = first_build; tmp; tmp = tmp->next )
				if ( !str_cmp( taf, tmp->filename ) )
				{
					tarea = tmp;
					break;
				}
		}
		if ( !tarea )
		{
			sprintf( buf, "Creating area entry for %s", ch->getName().c_str() );
			log_string_plus( buf, LOG_NORMAL, ch->level );

			tarea = new Area();
			LINK( tarea, first_build, last_build, next, prev );
			tarea->first_reset	= NULL;
			tarea->last_reset	= NULL;
			sprintf( buf, "{PROTO} %s's area in progress", ch->getName().c_str() );
			tarea->name		= str_dup( buf );
			tarea->filename	= str_dup( taf );
			sprintf( buf2, "%s", ch->getName().c_str() );
			tarea->author 	= STRALLOC( buf2 );
			tarea->age		= 0;
			tarea->nplayer	= 0;
			created = TRUE;
		}
		else
		{
			sprintf( buf, "Updating area entry for %s", ch->getName().c_str() );
			log_string_plus( buf, LOG_NORMAL, ch->level );
		}
		tarea->low_r_vnum	= ch->pcdata->r_range_lo;
		tarea->low_o_vnum	= ch->pcdata->o_range_lo;
		tarea->low_m_vnum	= ch->pcdata->m_range_lo;
		tarea->hi_r_vnum	= ch->pcdata->r_range_hi;
		tarea->hi_o_vnum	= ch->pcdata->o_range_hi;
		tarea->hi_m_vnum	= ch->pcdata->m_range_hi;
		ch->pcdata->area	= tarea;
		if ( created )
			sort_area( tarea, TRUE );
	}
}

void do_aassign(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *tarea, *tmp;

	if ( IS_NPC( ch ) )
	  return;

	if ( argument[0] == '\0' )
	{
	    send_to_char( "Syntax: aassign <filename.are>\r\n", ch );
	    return;
	}

	if ( !str_cmp( "none", argument )
	||   !str_cmp( "null", argument )
	||   !str_cmp( "clear", argument ) )
	{
	    ch->pcdata->area = NULL;
	    assign_area( ch );
	    if ( !ch->pcdata->area )
	      send_to_char( "Area pointer cleared.\r\n", ch );
	    else
	      send_to_char( "Originally assigned area restored.\r\n", ch );
	    return;
	}

	sprintf( buf, "%s", argument );
        tarea = NULL;

	if ( get_trust(ch) >= sysdata.level_modify_proto

/*	if ( get_trust(ch) >= LEVEL_SUB_IMPLEM*/
        ||  (is_name( buf, ch->pcdata->bestowments_.c_str() )
        ) )
	  for ( tmp = first_area; tmp; tmp = tmp->next )
	    if ( !str_cmp( buf, tmp->filename ) )
	    {
		tarea = tmp;
		break;
	    }

	if ( !tarea )
	  for ( tmp = first_build; tmp; tmp = tmp->next )
	    if ( !str_cmp( buf, tmp->filename ) )
	    {
		if ( get_trust(ch) >= sysdata.level_modify_proto
/*		if ( get_trust(ch) >= LEVEL_GREATER*/
	        ||   is_name( tmp->filename, ch->pcdata->bestowments_.c_str() ) )
		{
		    tarea = tmp;
		    break;
		}
		else
		{
		    send_to_char( "You do not have permission to use that area.\r\n", ch );
		    return;
		}
	    }

	if ( !tarea )
	{
	    if ( get_trust(ch) >= sysdata.level_modify_proto )
		send_to_char( "No such area.  Use 'zones'.\r\n", ch );
	    else
		send_to_char( "No such area.  Use 'newzones'.\r\n", ch );
	    return;
	}
	ch->pcdata->area = tarea;
	ch_printf( ch, "Assigning you: %s\r\n", tarea->name );
	return;
}


ExtraDescData *SetRExtra( ROOM_INDEX_DATA *room, const char *keywords )
{
    ExtraDescData *ed;

    for ( ed = room->first_extradesc; ed; ed = ed->next )
    {
	  if ( is_name( keywords, ed->keyword_.c_str() ) )
	    break;
    }
    if ( !ed )
    {
	ed = new ExtraDescData;
	LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
	ed->keyword_     = keywords;
	ed->description_ = "";
	top_ed++;
    }
    return ed;
}

bool DelRExtra( ROOM_INDEX_DATA *room, const char *keywords )
{
	ExtraDescData *rmed;

	for ( rmed = room->first_extradesc; rmed; rmed = rmed->next )
	{
		if ( is_name( keywords, rmed->keyword_.c_str() ) )
			break;
	}
	if ( !rmed )
		return FALSE;
	UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
	delete rmed;
	top_ed--;
	return TRUE;
}

ExtraDescData *SetOExtra( OBJ_DATA *obj, const char *keywords )
{
	ExtraDescData *ed;

	for ( ed = obj->first_extradesc; ed; ed = ed->next )
	{
		if ( is_name( keywords, ed->keyword_.c_str() ) )
			break;
	}
	if ( !ed )
	{
		ed = new ExtraDescData();
		LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		ed->keyword_     = keywords;
		ed->description_ = "";
		top_ed++;
	}
	return ed;
}

bool DelOExtra( OBJ_DATA *obj, const char *keywords )
{
	ExtraDescData *rmed;

	for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
	{
		if ( is_name( keywords, rmed->keyword_.c_str() ) )
			break;
	}
	if ( !rmed )
		return FALSE;
	UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
	delete rmed;
	top_ed--;
	return TRUE;
}

ExtraDescData *SetOExtraProto( OBJ_INDEX_DATA *obj, const char *keywords )
{
	ExtraDescData *ed;

	for ( ed = obj->first_extradesc; ed; ed = ed->next )
	{
		if ( is_name( keywords, ed->keyword_.c_str() ) )
			break;
	}
	if ( !ed )
	{
		ed = new ExtraDescData;
		LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		ed->keyword_     = keywords;
		ed->description_ = "";
		top_ed++;
	}
	return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA *obj, const char *keywords )
{
	ExtraDescData *rmed;

	for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
	{
		if ( is_name( keywords, rmed->keyword_.c_str() ) )
			break;
	}
	if ( !rmed )
		return FALSE;
	UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
	delete rmed;
	top_ed--;
	return TRUE;
}

void fold_area( AREA_DATA *tarea, const char *filename, bool install )
{
	RESET_DATA		*treset;
	ROOM_INDEX_DATA *room;
	MOB_INDEX_DATA	*pMobIndex;
	OBJ_INDEX_DATA	*pObjIndex;
	MPROG_DATA		*mprog;
	ExitData		*xit;
	ExtraDescData	*ed;
	AFFECT_DATA 	*paf;
	SHOP_DATA		*pShop;

	/* matthew */
	STABLE_DATA 	*pStable;

	REPAIR_DATA 	*pRepair;
	char		 buf[MAX_STRING_LENGTH];
	FILE		*fpout;
	int 		 vnum;
	int 		 val0, val1, val2, val3, val4, val5;
	int 		i, j;
	bool		 complexmob;

	/*
	   sprintf( buf, "Saving %s...", tarea->filename );
	   log_string_plus( buf, LOG_NORMAL, LEVEL_GREATER );
	 */

	sprintf( buf, "%s.bak", filename );
	rename( filename, buf );
	fclose( fpReserve );
	if ( ( fpout = fopen( filename, "w" ) ) == NULL )
	{
		bug( "fold_area: fopen", 0 );
		perror( filename );
		fpReserve = fopen( NULL_FILE, "r" );
		return;
	}

	fprintf( fpout, "#AREA   %s~\n\n\n\n", tarea->name );
	fprintf(fpout, "#VERSION %d\n", AREA_VERSION);

	fprintf( fpout, "#RANDOMDESCS\n\n");
	for (i = 0; i <= MAX_RANDOM_DESCRIPTION_TYPE; i++)
	{
		char desc_type[MAX_INPUT_LENGTH];
		switch(i)
		{
			default:
				continue;
				break;
			case RANDOM_PLAINS:
				strcpy(desc_type, "PlainsDesc");
				break;
			case RANDOM_MOUNTAIN:
				strcpy(desc_type, "MountainDesc");
				break;
			case RANDOM_DESERT:
				strcpy(desc_type, "DesertDesc");
				break;
			case RANDOM_HILL:
				strcpy(desc_type, "HillDesc");
				break;
			case RANDOM_FOREST:
				strcpy(desc_type, "ForestDesc");
				break;
			case RANDOM_SWAMP:
				strcpy(desc_type, "SwampDesc");
				break;
		}
		for (j = 0; j < MAX_RANDOM_DESCRIPTIONS; j++)
		{
			if (tarea->random_descriptions[i][j])
			{
				fprintf(fpout, "%s       %s~\n", desc_type, tarea->random_descriptions[i][j]);

				if (tarea->random_night_descriptions[i][j])
					fprintf(fpout, "$ %s~\n\n", tarea->random_night_descriptions[i][j]);
				else
					fprintf(fpout, "A\n\n" );
			}
		}
	}
	fprintf( fpout, "$\n\n");
	fprintf( fpout, "#AUTHOR %s~\n\n", tarea->author );
	fprintf( fpout, "#RANGES\n");
	fprintf( fpout, "%d %d %d %d\n", tarea->low_soft_range,
			tarea->hi_soft_range,
			tarea->low_hard_range,
			tarea->hi_hard_range );
	fprintf( fpout, "$\n\n");
	if ( tarea->resetmsg )	/* Rennard */
		fprintf( fpout, "#RESETMSG %s~\n\n", tarea->resetmsg );
	if ( tarea->reset_frequency )
		fprintf( fpout, "#FLAGS\n%d %d\n\n",
				tarea->flags, tarea->reset_frequency );
	else
		fprintf( fpout, "#FLAGS\n%d\n\n", tarea->flags );

	fprintf( fpout, "#INSTALLDATE %d\n\n", (int)tarea->secInstallDate);
	fprintf( fpout, "#ECONOMY %d %d\n\n", tarea->high_economy, tarea->low_economy );

	/* save mobiles */
	fprintf( fpout, "#MOBILES\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
			continue;
		if ( install )
			REMOVE_BIT( pMobIndex->act, ACT_PROTOTYPE );
		if ( pMobIndex->perm_str != 13	||	 pMobIndex->perm_int   != 13
				||	 pMobIndex->perm_wis != 13	||	 pMobIndex->perm_dex   != 13
				||	 pMobIndex->perm_con != 13	||	 pMobIndex->perm_cha   != 13
				||	 pMobIndex->perm_lck != 13
				||	 pMobIndex->hitroll  != 0	||	 pMobIndex->damroll    != 0
				||	 pMobIndex->race	 != 0	||	 pMobIndex->Class	   != 3
				||	 pMobIndex->attacks  != 0	||	 pMobIndex->defenses   != 0
				||	 pMobIndex->height	 != 0	||	 pMobIndex->weight	   != 0
				||	 pMobIndex->speaks	 != 0	||	 pMobIndex->speaking   != 0
				||	 pMobIndex->xflags	 != 0	||	 pMobIndex->numattacks != 0
				||	 pMobIndex->immune   != 0   ||   pMobIndex->resistant  != 0
				||   pMobIndex->susceptible != 0
				||	 pMobIndex->HomeVnum != 0 )
			complexmob = TRUE;
		else
			complexmob = FALSE;
		fprintf( fpout, "#%d\n",	vnum				);
		fprintf( fpout, "%s~\n",	pMobIndex->playerName_.c_str()		);
		fprintf( fpout, "%s~\n",	pMobIndex->shortDesc_.c_str()		);
		fprintf( fpout, "%s~\n",	strip_cr(pMobIndex->longDesc_.c_str()) );
		fprintf( fpout, "%s~\n",	strip_cr(pMobIndex->description_.c_str()));
		fprintf(fpout, "%s~\n", pMobIndex->exitDesc_.c_str() );
		fprintf(fpout, "%s~\n", pMobIndex->enterDesc_.c_str() );
		fprintf( fpout, "%d %d %d %c\n",pMobIndex->act,
				pMobIndex->affected_by,
				pMobIndex->alignment,
				complexmob ? 'C' : 'S'		);

		fprintf( fpout, "%d %d %d ",	pMobIndex->level,
				pMobIndex->mobthac0,
				pMobIndex->ac			);
		fprintf( fpout, "%dd%d+%d ",	pMobIndex->hitnodice,
				pMobIndex->hitsizedice,
				pMobIndex->hitplus		);
		fprintf( fpout, "%dd%d+%d\n",	pMobIndex->damnodice,
				pMobIndex->damsizedice,
				pMobIndex->damplus		);
		fprintf( fpout, "%d %d\n",	pMobIndex->gold,
				pMobIndex->exp			);
		fprintf( fpout, "%d %d %d\n",	pMobIndex->position,
				pMobIndex->defposition,
				pMobIndex->sex			);
		if ( complexmob )
		{
			fprintf( fpout, "%d %d %d %d %d %d %d\n",
					pMobIndex->perm_str,
					pMobIndex->perm_int,
					pMobIndex->perm_wis,
					pMobIndex->perm_dex,
					pMobIndex->perm_con,
					pMobIndex->perm_cha,
					pMobIndex->perm_lck );
			fprintf( fpout, "%d %d %d %d %d\n",
					pMobIndex->saving_poison_death,
					pMobIndex->saving_wand,
					pMobIndex->saving_para_petri,
					pMobIndex->saving_breath,
					pMobIndex->saving_spell_staff );
			fprintf( fpout, "%d %d %d %d %d %d %d\n",
					pMobIndex->race,
					pMobIndex->Class,
					pMobIndex->height,
					pMobIndex->weight,
					pMobIndex->speaks,
					pMobIndex->speaking,
					pMobIndex->numattacks );
			fprintf( fpout, "%d %d %d %d %d %d %d %d\n",
					pMobIndex->hitroll,
					pMobIndex->damroll,
					pMobIndex->xflags,
					pMobIndex->resistant,
					pMobIndex->immune,
					pMobIndex->susceptible,
					pMobIndex->attacks,
					pMobIndex->defenses );
			fprintf( fpout, "%d\n", pMobIndex->HomeVnum );
		}
		if ( pMobIndex->mudprogs )
		{
			for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
						mprog_type_to_name( mprog->type ),
						mprog->arglist, strip_cr(mprog->comlist) );
			fprintf( fpout, "|\n" );
		}
		if ( pMobIndex->skryptProgramSource_.length() > 0 )
		{
			pMobIndex->skryptWrite(fpout);
		}
	}
	fprintf( fpout, "#0\n\n\n" );
	if ( install && vnum < tarea->hi_m_vnum )
		tarea->hi_m_vnum = vnum - 1;

	/* save objects */
	fprintf( fpout, "#OBJECTS\n" );
	for ( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
	{
		if ( (pObjIndex = get_obj_index( vnum )) == NULL )
			continue;
		if ( install )
			REMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
		fprintf( fpout, "#%d\n",	vnum				);
		fprintf( fpout, "%s~\n",	pObjIndex->name_.c_str() 		);
		fprintf( fpout, "%s~\n",	pObjIndex->shortDesc_.c_str()		);
		fprintf( fpout, "%s~\n",	pObjIndex->longDesc_.c_str() );
		fprintf( fpout, "%s~\n",	pObjIndex->actionDesc_.c_str()		);
		if ( pObjIndex->layers )
			fprintf( fpout, "%d %d %d %d %d %d\n",	pObjIndex->item_type,
					pObjIndex->extra_flags,
					pObjIndex->wear_flags,
					pObjIndex->layers,
					pObjIndex->max_condition,
					pObjIndex->extra_flags_2);
		else
			fprintf( fpout, "%d %d %d 0 %d %d\n",	pObjIndex->item_type,
					pObjIndex->extra_flags,
					pObjIndex->wear_flags,
					pObjIndex->max_condition,
					pObjIndex->extra_flags_2);

		val0 = pObjIndex->value[0];
		val1 = pObjIndex->value[1];
		val2 = pObjIndex->value[2];
		val3 = pObjIndex->value[3];
		val4 = pObjIndex->value[4];
		val5 = pObjIndex->value[5];
		switch ( pObjIndex->item_type )
		{
			case ITEM_PILL:
			case ITEM_POTION:
			case ITEM_SCROLL:
				if ( IS_VALID_SN(val1) ) val1 = skill_table[val1]->slot;
				if ( IS_VALID_SN(val2) ) val2 = skill_table[val2]->slot;
				if ( IS_VALID_SN(val3) ) val3 = skill_table[val3]->slot;
				break;
			case ITEM_STAFF:
			case ITEM_WAND:
				if ( IS_VALID_SN(val3) ) val3 = skill_table[val3]->slot;
				break;
			case ITEM_SALVE:
				if ( IS_VALID_SN(val4) ) val4 = skill_table[val4]->slot;
				if ( IS_VALID_SN(val5) ) val5 = skill_table[val5]->slot;
				break;
		}
		if ( val4 || val5 )
			fprintf( fpout, "%d %d %d %d %d %d\n",val0,
					val1,
					val2,
					val3,
					val4,
					val5 );
		else
			fprintf( fpout, "%d %d %d %d\n",	val0,
					val1,
					val2,
					val3 );

		fprintf( fpout, "%d %d %d %d\n",	pObjIndex->weight,
				pObjIndex->cost,
				pObjIndex->rent ? pObjIndex->rent :
				(int) (pObjIndex->cost / 1),
				pObjIndex->rare);

		for ( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
			fprintf( fpout, "E\n%s~\n%s~\n",
					ed->keyword_.c_str(), strip_cr( ed->description_.c_str() )	);

		for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
			fprintf( fpout, "A\n%d %d\n", paf->location,
					((paf->location == APPLY_WEAPONSPELL
					  || paf->location == APPLY_WEARSPELL
					  || paf->location == APPLY_REMOVESPELL
					  || paf->location == APPLY_STRIPSN)
					 && IS_VALID_SN(paf->modifier))
					? skill_table[paf->modifier]->slot : paf->modifier		);

		if ( pObjIndex->mudprogs )
		{
			for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
						mprog_type_to_name( mprog->type ),
						mprog->arglist, strip_cr(mprog->comlist) );
			fprintf( fpout, "|\n" );
		}
		if ( pObjIndex->skryptProgramSource_.length() > 0 )
		{
			pObjIndex->skryptWrite(fpout);
		}
	}
	fprintf( fpout, "#0\n\n\n" );
	if ( install && vnum < tarea->hi_o_vnum )
		tarea->hi_o_vnum = vnum - 1;

	/* save rooms	*/
	fprintf( fpout, "#ROOMS\n" );
	for ( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
	{
		if ( (room = get_room_index( vnum )) == NULL )
			continue;
		if ( install )
		{
			CHAR_DATA *victim, *vnext;
			OBJ_DATA  *obj, *obj_next;

			/* remove prototype flag from room */
			REMOVE_BIT( room->room_flags, ROOM_PROTOTYPE );
			/* purge room of (prototyped) mobiles */
			for ( victim = room->first_person; victim; victim = vnext )
			{
				vnext = victim->next_in_room;
				if ( IS_NPC(victim) )
					extract_char( victim, TRUE );
			}
			/* purge room of (prototyped) objects */
			for ( obj = room->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				extract_obj( obj, TRUE );
			}
		}
		fprintf( fpout, "#%d\n",	vnum				);
		fprintf( fpout, "%s~\n",	room->name_.c_str()			);
		fprintf( fpout, "%s~\n",	strip_cr( room->description_.c_str() )	);
		fprintf( fpout, "%d %d\n", room->random_room_type, room->random_description);
		if ( (room->tele_delay > 0 && room->tele_vnum > 0) || room->tunnel > 0 )
			fprintf( fpout, "0 %d %d %d %d %d\n",	room->room_flags,
					room->sector_type,
					room->tele_delay,
					room->tele_vnum,
					room->tunnel		);
		else
			fprintf( fpout, "0 %d %d\n",	room->room_flags,
					room->sector_type	);
		for ( xit = room->first_exit; xit; xit = xit->next )
		{
			if ( IS_SET(xit->exit_info, EX_PORTAL) ) /* don't fold portals */
				continue;
			fprintf( fpout, "D%d\n",		xit->vdir );
			fprintf( fpout, "%s~\n",		strip_cr( xit->description_.c_str() ) );
			fprintf( fpout, "%s~\n",		strip_cr( xit->keyword_.c_str() ) );
			if ( xit->distance > 1 )
				fprintf( fpout, "%d %d %d %d\n",	xit->exit_info & ~EX_BASHED,
						xit->key,
						xit->vnum,
						xit->distance );
			else
				fprintf( fpout, "%d %d %d\n",	xit->exit_info & ~EX_BASHED,
						xit->key,
						xit->vnum );
		}
		for ( ed = room->first_extradesc; ed; ed = ed->next )
			fprintf( fpout, "E\n%s~\n%s~\n",
					ed->keyword_.c_str(), strip_cr( ed->description_.c_str() ));

		if ( room->scentID != 0 )
		{
			fprintf( fpout, "A %d\n", room->scentID);
		}

		if ( !room->mineMap.empty() )
		{
			std::map<int, int>::iterator it;

			for ( it = room->mineMap.begin(); it != room->mineMap.end(); ++it )
			{
				fprintf( fpout, "I %d %d\n", (*it).first, (*it).second);
			}
		}

		if ( room->map )   /* maps */
		{
#ifdef OLDMAPS
			fprintf( fpout, "M\n" );
			fprintf( fpout, "%s~\n", strip_cr( room->map )	);
#endif
			fprintf( fpout, "M %d %d %d %c\n",	room->map->vnum
					, room->map->x
					, room->map->y
					, room->map->entry );
		}
		if ( room->mudprogs )
		{
			for ( mprog = room->mudprogs; mprog; mprog = mprog->next )
				fprintf( fpout, "> %s %s~\n%s~\n",
						mprog_type_to_name( mprog->type ),
						mprog->arglist, strip_cr(mprog->comlist) );
			fprintf( fpout, "|\n" );
		}
		if ( room->RoomSkryptContainer->skryptProgramSource_.length() > 0 )
		{
			room->RoomSkryptContainer->skryptWrite(fpout);
		}
		fprintf( fpout, "S\n" );
	}
	fprintf( fpout, "#0\n\n\n" );
	if ( install && vnum < tarea->hi_r_vnum )
		tarea->hi_r_vnum = vnum - 1;

	/* save resets	 */
	fprintf( fpout, "#RESETS\n" );
	for ( treset = tarea->first_reset; treset; treset = treset->next )
	{
		switch( treset->command ) /* extra arg1 arg2 arg3 */
		{
			default:  case '*': break;
			case 'm': case 'M':
			case 'o': case 'O':
			case 'p': case 'P':
			case 'e': case 'E':
			case 'd': case 'D':
			case 't': case 'T':
								fprintf( fpout, "%c %d %d %d %d\n", UPPER(treset->command),
										treset->extra, treset->arg1, treset->arg2, treset->arg3 );
								break;
			case 'g': case 'G':
			case 'r': case 'R':
								fprintf( fpout, "%c %d %d %d\n", UPPER(treset->command),
										treset->extra, treset->arg1, treset->arg2 );
								break;
		}
	}
	fprintf( fpout, "S\n\n\n" );

	/* matthew */
	fprintf( fpout, "#STABLES\n");
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( (pMobIndex = get_mob_index(vnum) ) == NULL ) {
			continue;
		}

		if ( (pStable = pMobIndex->pStable) == NULL )
		{
			continue;
		}

		fprintf( fpout, "%d   %3d %3d %2d %2d     ; %s\n",
				pStable->keeper,
				pStable->stable_cost,
				pStable->unstable_cost,
				pStable->open_hour,
				pStable->close_hour,
				pMobIndex->shortDesc_.c_str());
	}
	fprintf(fpout, "0\n\n\n");


	/* save shops */
	fprintf( fpout, "#SHOPS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
			continue;
		if ( (pShop = pMobIndex->pShop) == NULL )
			continue;
		fprintf( fpout, " %d   %2d %2d %2d %2d %2d   %3d %3d",
				pShop->keeper,
				pShop->buy_type[0],
				pShop->buy_type[1],
				pShop->buy_type[2],
				pShop->buy_type[3],
				pShop->buy_type[4],
				pShop->profit_buy,
				pShop->profit_sell );
		fprintf( fpout, "        %2d %2d    ; %s\n",
				pShop->open_hour,
				pShop->close_hour,
				pMobIndex->shortDesc_.c_str() );
	}
	fprintf( fpout, "0\n\n\n" );

	/* save repair shops */
	fprintf( fpout, "#REPAIRS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
			continue;
		if ( (pRepair = pMobIndex->rShop) == NULL )
			continue;
		fprintf( fpout, " %d   %2d %2d %2d         %3d %3d",
				pRepair->keeper,
				pRepair->fix_type[0],
				pRepair->fix_type[1],
				pRepair->fix_type[2],
				pRepair->profit_fix,
				pRepair->shop_type );
		fprintf( fpout, "        %2d %2d    ; %s\n",
				pRepair->open_hour,
				pRepair->close_hour,
				pMobIndex->shortDesc_.c_str() );
	}
	fprintf( fpout, "0\n\n\n" );

	/* save specials */
	fprintf( fpout, "#SPECIALS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
			continue;
		if ( !pMobIndex->spec_fun )
			continue;
		fprintf( fpout, "M  %d %s\n",	pMobIndex->vnum,
				lookup_spec( pMobIndex->spec_fun ) );
	}
	fprintf( fpout, "S\n\n\n" );

	/* save trainers, easier to save to it's own section */
	fprintf( fpout, "#TRAINERS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
			continue;

		if (!pMobIndex->train)
			continue;

		fprintf( fpout, "M  %d %d %d %d %d %d %d\r\n",
				pMobIndex->vnum,
				pMobIndex->train->_class,
				pMobIndex->train->alignment,
				pMobIndex->train->min_level,
				pMobIndex->train->max_level,
				pMobIndex->train->base_cost,
				pMobIndex->train->gain_cost);
	}
	fprintf( fpout, "S\n\n\n" );

	fprintf( fpout, "#TRAINERSKILLS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		TRAIN_LIST_DATA *tlist;

		if ( (pMobIndex = get_mob_index( vnum )) == NULL )
			continue;

		if (!pMobIndex->train)
			continue;

		for ( tlist = pMobIndex->train->first_in_list; tlist; tlist = tlist->next)
		{
			SkillType *skill = get_skilltype(tlist->sn);

			if ( skill )
			{
				fprintf( fpout, "M  %d %d %d \'%s\'\r\n",
						pMobIndex->vnum,
						tlist->max,
						tlist->cost,
						skill->name_.c_str());
			}
		}
	}
	fprintf( fpout, "S\n\n\n" );


	/* END */
	fprintf( fpout, "#$\n" );
	fclose( fpout );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
}

void do_savearea(CHAR_DATA *ch, const char* argument)
{
    AREA_DATA	*tarea;
    char	 filename[256];

    if ( IS_NPC(ch) || !ch->pcdata
    ||  ( argument[0] == '\0' && !ch->pcdata->area) )
    {
	send_to_char( "You don't have an assigned area to save.\r\n", ch );
	return;
    }

    if ( argument[0] == '\0' )
	tarea = ch->pcdata->area;
    else
    {
	bool found;

	if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
	{
	    send_to_char( "You can only save your own area.\r\n", ch );
	    return;
	}
	for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
	    if ( !str_cmp( tarea->filename, argument ) )
	    {
		found = TRUE;
		break;
	    }
	if ( !found )
	{
	    send_to_char( "Area not found.\r\n", ch );
	    return;
	}
    }

    if ( !tarea )
    {
	send_to_char( "No area to save.\r\n", ch );
	return;
    }

/* Ensure not wiping out their area with save before load - Scryn 8/11 */
    if ( !IS_SET(tarea->status, AREA_LOADED ) )
    {
	send_to_char( "Your area is not loaded!\r\n", ch );
	return;
    }

    sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
    fold_area( tarea, filename, FALSE );
    send_to_char( "Done.\r\n", ch );
}

void do_loadarea(CHAR_DATA *ch, const char* argument)
{
    AREA_DATA	*tarea;
    char	 filename[256];
    int		tmp;

    if ( IS_NPC(ch) || !ch->pcdata
    ||  ( argument[0] == '\0' && !ch->pcdata->area) )
    {
	send_to_char( "You don't have an assigned area to load.\r\n", ch );
	return;
    }

    if ( argument[0] == '\0' )
	tarea = ch->pcdata->area;
    else
    {
	bool found;

	if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
	{
	    send_to_char( "You can only load your own area.\r\n", ch );
	    return;
	}
	for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
	    if ( !str_cmp( tarea->filename, argument ) )
	    {
		found = TRUE;
		break;
	    }
	if ( !found )
	{
	    send_to_char( "Area not found.\r\n", ch );
	    return;
	}
    }

    if ( !tarea )
    {
	send_to_char( "No area to load.\r\n", ch );
	return;
    }

/* Stops char from loading when already loaded - Scryn 8/11 */
    if ( IS_SET ( tarea->status, AREA_LOADED) )
    {
	send_to_char( "Your area is already loaded.\r\n", ch );
	return;
    }
    sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
    send_to_char( "Loading...\r\n", ch );
    load_area_file( tarea, filename );
    send_to_char( "Linking exits...\r\n", ch );
    fix_area_exits( tarea );
    if ( tarea->first_reset )
    {
	tmp = tarea->nplayer;
	tarea->nplayer = 0;
	send_to_char( "Resetting area...\r\n", ch );
	reset_area( tarea );
	tarea->nplayer = tmp;
    }
    send_to_char( "Done.\r\n", ch );
}

/*
 * Dangerous command.  Can be used to install an area that was either:
 *   (a) already installed but removed from area.lst
 *   (b) designed offline
 * The mud will likely crash if:
 *   (a) this area is already loaded
 *   (b) it contains vnums that exist
 *   (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.		-Thoric
 */
void do_unfoldarea(CHAR_DATA *ch, const char* argument)
{

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Unfold what?\r\n", ch );
	return;
    }

    fBootDb = TRUE;
    load_area_file( last_area, argument );
    fBootDb = FALSE;
    return;
}


void do_foldarea(CHAR_DATA *ch, const char* argument)
{
    AREA_DATA	*tarea;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Fold what?\r\n", ch );
	return;
    }

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
	if ( !str_cmp( tarea->filename, argument ) )
	{
	  send_to_char( "Folding...\r\n", ch );
	  fold_area( tarea, tarea->filename, FALSE );
	  send_to_char( "Done.\r\n", ch );
	  return;
	}
    }
    send_to_char( "No such area exists.\r\n", ch );
    return;
}

extern int top_area;

void write_area_list( )
{
    AREA_DATA *tarea;
    FILE *fpout;

    fpout = fopen( AREA_LIST, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open area.lst for writing!\r\n", 0 );
 	return;
    }
    fprintf( fpout, "help.are\n" );
    for ( tarea = first_area; tarea; tarea = tarea->next )
	fprintf( fpout, "%s\n", tarea->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
 * A complicated to use command as it currently exists.		-Thoric
 * Once area->author and area->name are cleaned up... it will be easier
 */
void do_installarea(CHAR_DATA *ch, const char* argument)
{
	AREA_DATA	*tarea;
	char	arg[MAX_INPUT_LENGTH];
	char	buf[MAX_STRING_LENGTH];
	int 	num;

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Syntax: installarea <filename> [Area title]\r\n", ch );
		return;
	}

	for ( tarea = first_build; tarea; tarea = tarea->next )
	{
		if ( !str_cmp( tarea->filename, arg ) )
		{
			if ( argument && argument[0] != '\0' )
			{
				DISPOSE( tarea->name );
				tarea->name = str_dup( argument );
			}

			/* Fold area with install flag -- auto-removes prototype flags */
			send_to_char( "Saving and installing file...\r\n", ch );
			fold_area( tarea, tarea->filename, TRUE );

			/* Remove from prototype area list */
			UNLINK( tarea, first_build, last_build, next, prev );

			/* Add to real area list */
			LINK( tarea, first_area, last_area, next, prev );

			/* Fix up author if online */
			itorSocketId itor;
			for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
			{
				// we know that the world's player connection list only holds player connections IDs,
				// so we can safely cast it to PlayerConnection*
				PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

				if ( d->GetCharacter()
					&&	 d->GetCharacter()->pcdata
					&&	 d->GetCharacter()->pcdata->area == tarea )
				{
					/* remove area from author */
					d->GetCharacter()->pcdata->area = NULL;
					/* clear out author vnums  */
					d->GetCharacter()->pcdata->r_range_lo = 0;
					d->GetCharacter()->pcdata->r_range_hi = 0;
					d->GetCharacter()->pcdata->o_range_lo = 0;
					d->GetCharacter()->pcdata->o_range_hi = 0;
					d->GetCharacter()->pcdata->m_range_lo = 0;
					d->GetCharacter()->pcdata->m_range_hi = 0;
				}
			}

			top_area++;
			send_to_char( "Writing area.lst...\r\n", ch );
			write_area_list( );
			send_to_char( "Resetting new area.\r\n", ch );
			num = tarea->nplayer;
			tarea->nplayer = 0;
			reset_area( tarea );
			tarea->nplayer = num;
			send_to_char( "Renaming author's building file.\r\n", ch );
			sprintf( buf, "%s%s.installed", BUILD_DIR, tarea->filename );
			sprintf( arg, "%s%s", BUILD_DIR, tarea->filename );
			rename( arg, buf );

			//Ksilyan:
			tarea->secInstallDate = secCurrentTime;
			send_to_char( "Done.\r\n", ch );
			return;
		}
	}
	send_to_char( "No such area exists.\r\n", ch );
	return;
}

void add_reset_nested( AREA_DATA *tarea, OBJ_DATA *obj )
{
   int limit;

   for ( obj = obj->first_content; obj; obj = obj->next_content )
   {
	limit = obj->pIndexData->count;
	if ( limit < 1 )
	  limit = 1;
	add_reset( tarea, 'P', 1, obj->pIndexData->vnum, limit,
				 obj->GetInObj()->pIndexData->vnum );
	if ( obj->first_content )
	  add_reset_nested( tarea, obj );
   }
}


/*
 * Parse a reset command string into a reset_data structure
 */
RESET_DATA *parse_reset( AREA_DATA *tarea, const char *argument, CHAR_DATA *ch )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	char letter;
	int extra, val1, val2, val3;
	int value;
	ROOM_INDEX_DATA *room;
	ExitData	*pexit;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	extra = 0; letter = '*';

	// Change by Ksilyan 2006-03-11 -- use dotted format instead of plain atoi.
	//val1 = atoi( arg2 );
	//val2 = atoi( arg3 );
	//val3 = atoi( arg4 );
	val1 = dotted_to_vnum( ch->GetInRoom()->vnum, arg2 );
	val2 = atoi(arg3);
	val3 = atoi(arg4);

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Reset commands: mob obj give equip door rand trap hide.\r\n", ch );
		return NULL;
	}

	if ( !str_prefix( arg1, "hide" ) )
	{
		if ( arg2[0] != '\0' && !get_obj_index(val1) )
		{
			send_to_char( "Reset: HIDE: no such object\r\n", ch );
			return NULL;
		}
		else
			val1 = 0;
		extra = 1;
		val2 = 0;
		val3 = 0;
		letter = 'H';
	}
	else if ( arg2[0] == '\0' )
	{
		send_to_char( "Reset: not enough arguments.\r\n", ch );
		return NULL;
	}
	else if ( val1 < 1 || val1 > 1048576000 )
	{
		send_to_char( "Reset: value out of range.\r\n", ch );
		return NULL;
	}
	else if ( !str_prefix( arg1, "mob" ) )
	{
		if ( !get_mob_index(val1) )
		{
			send_to_char( "Reset: MOB: no such mobile\r\n", ch );
			return NULL;
		}
		if ( !get_room_index(val2) )
		{
			send_to_char( "Reset: MOB: no such room\r\n", ch );
			return NULL;
		}
		if ( val3 < 1 )
			val3 = 1;
		letter = 'M';
	}
	else if ( !str_prefix( arg1, "obj" ) )
	{
		if ( !get_obj_index(val1) )
		{
			send_to_char( "Reset: OBJ: no such object\r\n", ch );
			return NULL;
		}
		if ( !get_room_index(val2) )
		{
			send_to_char( "Reset: OBJ: no such room\r\n", ch );
			return NULL;
		}
		if ( val3 < 1 )
			val3 = 1;
		letter = 'O';
	}
	else if ( !str_prefix( arg1, "give" ) )
	{
		if ( !get_obj_index(val1) )
		{
			send_to_char( "Reset: GIVE: no such object\r\n", ch );
			return NULL;
		}
		if ( val2 < 1 )
			val2 = 1;
		val3 = val2;
		val2 = 0;
		extra = 1;
		letter = 'G';
	}
	else if ( !str_prefix( arg1, "equip" ) )
	{
		if ( !get_obj_index(val1) )
		{
			send_to_char( "Reset: EQUIP: no such object\r\n", ch );
			return NULL;
		}
		if ( !is_number(arg3) )
			val2 = get_wearloc(arg3);
		if ( val2 < 0 || val2 >= MAX_WEAR )
		{
			send_to_char( "Reset: EQUIP: invalid wear location\r\n", ch );
			return NULL;
		}
		if ( val3 < 1 )
			val3 = 1;
		extra  = 1;
		letter = 'E';
	}
	else if ( !str_prefix( arg1, "put" ) )
	{
		if ( !get_obj_index(val1) )
		{
			send_to_char( "Reset: PUT: no such object\r\n", ch );
			return NULL;
		}
		if ( val2 > 0 && !get_obj_index(val2) )
		{
			send_to_char( "Reset: PUT: no such container\r\n", ch );
			return NULL;
		}
		extra = UMAX(val3, 0);
		argument = one_argument(argument, arg4);
		val3 = (is_number(argument) ? atoi(arg4) : 0);
		if ( val3 < 0 )
			val3 = 0;
		letter = 'P';
	}
	else if ( !str_prefix( arg1, "door" ) )
	{
		if ( (room = get_room_index(val1)) == NULL )
		{
			send_to_char( "Reset: DOOR: no such room\r\n", ch );
			return NULL;
		}
		if ( val2 < 0 || val2 > 9 )
		{
			send_to_char( "Reset: DOOR: invalid exit\r\n", ch );
			return NULL;
		}
		if ( (pexit = get_exit(room, val2)) == NULL
				||   !IS_SET( pexit->exit_info, EX_ISDOOR ) )
		{
			send_to_char( "Reset: DOOR: no such door\r\n", ch );
			return NULL;
		}
		if ( val3 < 0 || val3 > 2 )
		{
			send_to_char( "Reset: DOOR: invalid door state (0 = open, 1 = close, 2 = lock)\r\n", ch );
			return NULL;
		}
		letter = 'D';
		value = val3;
		val3  = val2;
		val2  = value;
	}
	else if ( !str_prefix( arg1, "rand" ) )
	{
		if ( !get_room_index(val1) )
		{
			send_to_char( "Reset: RAND: no such room\r\n", ch );
			return NULL;
		}
		if ( val2 < 0 || val2 > 9 )
		{
			send_to_char( "Reset: RAND: invalid max exit\r\n", ch );
			return NULL;
		}
		val3 = val2;
		val2 = 0;
		letter = 'R';
	}
	else if ( !str_prefix( arg1, "trap" ) )
	{
		if ( val2 < 1 || val2 > MAX_TRAPTYPE )
		{
			send_to_char( "Reset: TRAP: invalid trap type\r\n", ch );
			return NULL;
		}
		if ( val3 < 0 || val3 > 10000 )
		{
			send_to_char( "Reset: TRAP: invalid trap charges\r\n", ch );
			return NULL;
		}
		while ( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg4 );
			value = get_trapflag( arg4 );
			if ( value >= 0 || value < 32 )
				SET_BIT( extra, 1 << value );
			else
			{
				send_to_char( "Reset: TRAP: bad flag\r\n", ch );
				return NULL;
			}
		}
		if ( IS_SET(extra, TRAP_ROOM) && IS_SET(extra, TRAP_OBJ) )
		{
			send_to_char( "Reset: TRAP: Must specify room OR object, not both!\r\n", ch );
			return NULL;
		}
		if ( IS_SET(extra, TRAP_ROOM) && !get_room_index(val1) )
		{
			send_to_char( "Reset: TRAP: no such room\r\n", ch );
			return NULL;
		}
		if ( IS_SET(extra, TRAP_OBJ)  && val1>0 && !get_obj_index(val1) )
		{
			send_to_char( "Reset: TRAP: no such object\r\n", ch );
			return NULL;
		}
		if (!IS_SET(extra, TRAP_ROOM) && !IS_SET(extra, TRAP_OBJ) )
		{
			send_to_char( "Reset: TRAP: Must specify ROOM or OBJECT\r\n", ch );
			return NULL;
		}
		/* fix order */
		value = val1;
		val1  = val2;
		val2  = value;
		letter = 'T';
	}
	if ( letter == '*' )
		return NULL;
	else
		return make_reset( letter, extra, val1, val3, val2 );
}

void do_astat(CHAR_DATA *ch, const char* argument)
{
    AREA_DATA *tarea;
	char buf[MAX_STRING_LENGTH];
    bool proto, found;
    bool random;
    int i, j;

    random = FALSE;

    if (!str_cmp(argument, "random"))
    {
        random = TRUE;
        tarea = ch->GetInRoom()->area;
    }
    else
    {
        found = FALSE; proto = FALSE;
        for ( tarea = first_area; tarea; tarea = tarea->next )
    	if ( !str_cmp( tarea->filename, argument ) )
    	{
    	  found = TRUE;
    	  break;
    	}

        if ( !found )
          for ( tarea = first_build; tarea; tarea = tarea->next )
    	if ( !str_cmp( tarea->filename, argument ) )
	    {
    	  found = TRUE;
	      proto = TRUE;
    	  break;
    	}

        if ( !found )
        {
          if ( argument && argument[0] != '\0' )
          {
         	send_to_char( "Area not found.  Check 'zones'.\r\n", ch );
        	return;
          }
          else
          {
            tarea = ch->GetInRoom()->area;
          }
        }
    }


    ch_printf( ch, "Name: %s\r\nFilename: %-20s  Prototype: %s\r\n",
			tarea->name,
			tarea->filename,
			proto ? "yes" : "no" );
    if ( !proto )
    {
	ch_printf( ch, "Max players: %d  IllegalPks: %d  Gold Looted: %d\r\n",
			tarea->max_players,
			tarea->illegal_pk,
			tarea->gold_looted );
	if ( tarea->high_economy )
	   ch_printf( ch, "Area economy: %d billion and %d gold coins.\r\n",
			tarea->high_economy,
			tarea->low_economy );
	else
	   ch_printf( ch, "Area economy: %d gold coins.\r\n",
			tarea->low_economy );
	ch_printf( ch, "Mdeaths: %d  Mkills: %d  Pdeaths: %d  Pkills: %d\r\n",
			tarea->mdeaths,
			tarea->mkills,
			tarea->pdeaths,
			tarea->pkills );
    }
    ch_printf( ch, "Author: %s\r\nAge: %d   Number of players: %d\r\n",
			tarea->author,
			tarea->age,
			tarea->nplayer );

	// Ksilyan:
	strftime(buf, MAX_STRING_LENGTH, "%b %d, %Y", gmtime(&tarea->secInstallDate) );
	ch_printf( ch, "Date installed: %s\r\n", buf );

    ch_printf( ch, "Area flags: %s\r\n", flag_string(tarea->flags, area_flags) );
    ch_printf( ch, "low_room: %11s", vnum_to_dotted(tarea->low_r_vnum));
    ch_printf( ch, " hi_room: %11s\r\n", vnum_to_dotted(tarea->hi_r_vnum) );
    ch_printf( ch, "low_obj : %11s", vnum_to_dotted(tarea->low_o_vnum));
    ch_printf( ch, " hi_obj : %11s\r\n", vnum_to_dotted(tarea->hi_o_vnum) );
    ch_printf( ch, "low_mob : %11s", vnum_to_dotted(tarea->low_m_vnum));
    ch_printf( ch, " hi_mob : %11s\r\n", vnum_to_dotted(tarea->hi_m_vnum) );
    ch_printf( ch, "soft range: %d - %d.  hard range: %d - %d.\r\n",
			tarea->low_soft_range,
			tarea->hi_soft_range,
			tarea->low_hard_range,
			tarea->hi_hard_range );
    ch_printf( ch, "Resetmsg: %s\r\n", tarea->resetmsg ? tarea->resetmsg
						: "(default)" ); /* Rennard */
    ch_printf( ch, "Reset frequency: %d minutes.\r\n",
		tarea->reset_frequency ? tarea->reset_frequency : 15 );

    if (random == TRUE)
    {
		char terrain_name[MAX_INPUT_LENGTH];
        for ( i = 0; i <= MAX_RANDOM_DESCRIPTION_TYPE; i++ )
        {
			switch(i)
			{
				default:
					continue;
					break;
				case RANDOM_FOREST:
					strcpy(terrain_name, "forest");
					break;
				case RANDOM_PLAINS:
					strcpy(terrain_name, "plains");
					break;
				case RANDOM_HILL:
					strcpy(terrain_name, "hill");
					break;
				case RANDOM_DESERT:
					strcpy(terrain_name, "desert");
					break;
				case RANDOM_MOUNTAIN:
					strcpy(terrain_name, "mountain");
					break;
				case RANDOM_SWAMP:
					strcpy(terrain_name, "swamp");
					break;
			}
			set_pager_color(AT_CYAN, ch);
			ch_printf(ch, "Random %s descriptions:\r\n", terrain_name);
			for (j = 0; j < MAX_RANDOM_DESCRIPTIONS; j++)
			{
				if (tarea->random_descriptions[i][j])
				{
					set_pager_color(AT_LBLUE, ch);
					ch_printf(ch, "%s", tarea->random_descriptions[i][j]);
					set_pager_color(AT_BLOOD, ch);
					send_to_char("-----------\r\n",ch);
				}
				if (tarea->random_night_descriptions[i][j])
				{
					set_pager_color(AT_BLUE, ch);
					ch_printf(ch, "%s", tarea->random_night_descriptions[i][j]);
					set_pager_color(AT_BLOOD, ch);
					send_to_char("-----------\r\n",ch);
				}
			}
        }
    }
}


void do_aset(CHAR_DATA *ch, const char* argument)
{
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	bool proto, found;
	int vnum, value;
	char * location;
	char nightChar;
	int random;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);

	/*
	 * Ksilyan:
	 * Added in random room descriptions.
	 */
	switch( ch->substate )
	{
		default:
			break;
		case SUB_AREA_RANDOM:
			location = (char *) ch->dest_buf;
			tarea = (AREA_DATA *) ch->spare_ptr;
			if ( !location )
			{
				bug( "aset: sub_area_random: NULL ch->dest_buf", 0 );
				ch->substate =  SUB_NONE;
				return;
			}
			if ( !tarea )
			{
				bug( "aset: sub_area_random: NULL ch->spare_ptr", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			random = location[0];
			if ((random < MIN_RANDOM_DESCRIPTION_TYPE) || (random > MAX_RANDOM_DESCRIPTION_TYPE))
			{
				char buf[MAX_INPUT_LENGTH];
				bug( "aset: sub_area_random: random type unknown.", 0);
				sprintf(buf, "Random type: %d.", random);
				bug( buf, 0);
				bug (location);
				ch->substate = SUB_NONE;
				return;
			}
			value = location[1] - 1;
			if ((value < 0) || (value >= MAX_RANDOM_DESCRIPTIONS))
			{
				bug( "aset: sub_area_random: random description number out of bounds.", 0);
				ch->substate = SUB_NONE;
				return;
			}
			nightChar = location[2];

			STRFREE( location ); // free the string we created that stores random type, desc number and night.

			if (nightChar == 'N')
				location = tarea->random_night_descriptions[random][value];
			else
			{
				location = tarea->random_descriptions[random][value];
			}

			// Free the description, if it already exists.
			if (location)
				STRFREE( location );

			if (nightChar == 'N')
				tarea->random_night_descriptions[random][value] = copy_buffer( ch );
			else
				tarea->random_descriptions[random][value] = copy_buffer( ch );
			stop_editing( ch );
			ch->substate = ch->tempnum;
			return;
	}

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Usage: aset <area filename> <field> <value>\r\n", ch );
		send_to_char( "\r\nField being one of:\r\n", ch );
		send_to_char( "  low_room hi_room low_obj hi_obj low_mob hi_mob\r\n", ch );
		send_to_char( "  name filename low_soft hi_soft low_hard hi_hard\r\n", ch );
		send_to_char( "  author resetmsg resetfreq flags\r\n", ch );
		send_to_char( "  installdate\r\n", ch );
		return;
	}

	found = FALSE; proto = FALSE;
	for ( tarea = first_area; tarea; tarea = tarea->next )
		if ( !str_cmp( tarea->filename, arg1 ) )
		{
			found = TRUE;
			break;
		}

	if ( !found )
		for ( tarea = first_build; tarea; tarea = tarea->next )
			if ( !str_cmp( tarea->filename, arg1 ) )
			{
				found = TRUE;
				proto = TRUE;
				break;
			}

	if ( !found )
	{
		send_to_char( "Area not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "installdate" ) )
	{
		struct tm Time;
		int day = 0;
		int month = 0;
		int year = 0;

		char dayarg[MAX_INPUT_LENGTH];
		char montharg[MAX_INPUT_LENGTH];
		char yeararg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, dayarg);
		argument = one_argument(argument, montharg);
		argument = one_argument(argument, yeararg);

		if ( strlen(dayarg) == 0 || !is_number(dayarg) )
		{
			send_to_char("Please enter a valid day number. (1 to 31, depending on the month).\r\n", ch);
			return;
		}
		if ( strlen(montharg) == 0 || !is_number(montharg) )
		{
			send_to_char("Please enter a valid month. (0 to 11).\r\n", ch);
			return;
		}
		if ( strlen(yeararg) == 0 || !is_number(yeararg) )
		{
			send_to_char("Please enter a valid year. (e.g. 2002, 2003)\r\n", ch);
			return;
		}

		day = atoi(dayarg);
		month = atoi(montharg);
		year = atoi(yeararg) - 1900;

		Time.tm_isdst = 0;
		Time.tm_hour = 1;
		Time.tm_min = 1;
		Time.tm_sec = 1;
		Time.tm_mday = day;
		Time.tm_mon = month;
		Time.tm_year = year;

		tarea->secInstallDate = mktime(&Time);

		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "name" ) )
	{
		DISPOSE( tarea->name );
		tarea->name = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "random" ) )
	{
		int random, value, i;
		char * description;
		bool night;
		char buf[MAX_INPUT_LENGTH];

		random = value = 0;

		night = false;

		argument = one_argument(argument, arg3);
		value = atoi(argument);

		if (!str_cmp(arg3, "plains"))
			random = RANDOM_PLAINS;
		else if (!str_cmp(arg3, "plainsnight")) {
			random = RANDOM_PLAINS; night = true; }

		else if (!str_cmp(arg3, "forest"))
			random = RANDOM_FOREST;
		else if (!str_cmp(arg3, "forestnight")) {
			random = RANDOM_FOREST; night = true; }

		else if (!str_cmp(arg3, "hills"))
			random = RANDOM_HILL;
		else if (!str_cmp(arg3, "hillsnight")) {
			random = RANDOM_HILL; night = true; }

		else if (!str_cmp(arg3, "mountain"))
			random = RANDOM_MOUNTAIN;
		else if (!str_cmp(arg3, "mountainnight")) {
			random = RANDOM_MOUNTAIN; night = true; }

		else if (!str_cmp(arg3, "desert"))
			random = RANDOM_DESERT;
		else if (!str_cmp(arg3, "desertnight")) {
			random = RANDOM_DESERT; night = true; }

		else if (!str_cmp(arg3, "swamp"))
			random = RANDOM_SWAMP;
		else if (!str_cmp(arg3, "swampnight")) {
			random = RANDOM_SWAMP; night = true; }

		else
			random = 0;

		if (random == 0)
		{
			ch->sendText("You must specify which kind of random description to edit.\r\n");
			ch->sendText("(plains, forest, hills, mountain, desert, swamp)\r\n");
			ch->sendText("(to edit a night description, add night: e.g. hillsnight)\r\n");
			return;
		}
		else
		{
			if (value > 0)
			{
				char * desc;

				value--; // because humans use 1 to 6, code uses 0 to 5

				if ((value < 0) || (value >= MAX_RANDOM_DESCRIPTIONS))
				{
					send_to_char( "Invalid description number. Enter a number from 1 to 6.\r\n", ch);
					return;
				}

				if (night)
					desc = tarea->random_night_descriptions[random][value];
				else
					desc = tarea->random_descriptions[random][value];

				if (desc == NULL)
				{
					send_to_char( "You cannot edit a description that does not exist.\r\n", ch);
					return;
				}
				description = desc;
			}
			else
			{
				char * desc;

				value = -1;
				for (i = 0; i < MAX_RANDOM_DESCRIPTIONS; i++)
				{
					if (night)
						desc = tarea->random_night_descriptions[random][i];
					else
						desc = tarea->random_descriptions[random][i];

					if (desc == NULL)
					{
						description = desc;
						value = i;
						tarea->random_description_counts[random]++;
						break;
					}
				}
				if (value == -1)
				{
					send_to_char("The maximum number of random descriptions for that terrain type has already been reached.\r\n", ch);
					return;
				}
			}
		}


		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_AREA_RANDOM;
		if (!description)
			description = STRALLOC("");
		if (night)
		{
			tarea->random_night_descriptions[random][value] = description;
		}
		else
		{
			tarea->random_descriptions[random][value] = description;
		}
		sprintf(buf, "%c%c%c", random, value + 1, (night == TRUE) ? 'N' : 'D');
		/*
		   This is to pass on the information to the random desc setter.
		   We need to +1 value because of the fact that 0 terminates strings...
		   So this is clumsy. Sue me.
		 */
		ch->dest_buf = STRALLOC(buf);
		ch->spare_ptr = tarea;
		start_editing( ch, description );
		return;
	}

	if ( !str_prefix( arg2, "filename" ) )
	{
		DISPOSE( tarea->filename );
		tarea->filename = str_dup( argument );
		write_area_list( );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "low_economy" ) )
	{
		tarea->low_economy = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "high_economy" ) )
	{
		tarea->high_economy = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "low_room" ) )
	{
		tarea->low_r_vnum = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "hi_room" ) )
	{
		tarea->hi_r_vnum = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "low_obj" ) )
	{
		tarea->low_o_vnum = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "hi_obj" ) )
	{
		tarea->hi_o_vnum = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "low_mob" ) )
	{
		tarea->low_m_vnum = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "hi_mob" ) )
	{
		tarea->hi_m_vnum = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "low_soft" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch);
			return;
		}

		tarea->low_soft_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "hi_soft" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch);
			return;
		}

		tarea->hi_soft_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "low_hard" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch);
			return;
		}

		tarea->low_hard_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "hi_hard" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char( "That is not an acceptable value.\r\n", ch);
			return;
		}

		tarea->hi_hard_range = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "author" ) )
	{
		STRFREE( tarea->author );
		tarea->author = STRALLOC( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "resetmsg" ) )
	{
		if ( tarea->resetmsg )
			DISPOSE( tarea->resetmsg );
		if ( str_cmp( argument, "clear" ) )
			tarea->resetmsg = str_dup( argument );
		send_to_char( "Done.\r\n", ch );
		return;
	} /* Rennard */

	if ( !str_prefix( arg2, "resetfreq" ) )
	{
		tarea->reset_frequency = vnum;
		send_to_char( "Done.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "flags" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Usage: aset <filename> flags <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != '\0' )
		{
			argument = one_argument( argument, arg3 );
			value = get_areaflag( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( IS_SET( tarea->flags, 1 << value ) )
					REMOVE_BIT( tarea->flags, 1 << value );
				else
					SET_BIT( tarea->flags, 1 << value );
			}
		}
		return;
	}

	do_aset( ch, "" );
	return;
}

void do_rarelist( CHAR_DATA *ch, const char* argument )
{
    OBJ_INDEX_DATA *obj;
    int vnum;
    AREA_DATA *tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;

    /*
     * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
     */
    if ( IS_NPC(ch) || !ch->pcdata || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_STONE_ADEPT ) )
    {
		send_to_char( "You don't have an assigned area.\r\n", ch );
		return;
    }

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
    	lrange = tarea->low_o_vnum;
      	trange = tarea->hi_o_vnum;
    }
   	else
    {
    	lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
    	trange = ( is_number( arg2 ) ? atoi( arg2 ) : 3500000 );
    }

    for( vnum = lrange; vnum <= trange; vnum++ )
    {
		if ( (obj = get_obj_index( vnum )) == NULL )
	  		continue;

		if( IS_SET( obj->extra_flags_2, ITEM_RARE ) )
		{
			ch_printf( ch, "%5s) %-25s (%s)\r\n", vnum_to_dotted(vnum), obj->name_.c_str(), obj->shortDesc_.c_str() );
		}
		else
			continue;
	}
    return;
}

void do_rlist(CHAR_DATA *ch, const char* argument)
{
	ROOM_INDEX_DATA	*room;
	int			 vnum;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	AREA_DATA		*tarea;
	int lrange;
	int trange;

	if ( IS_NPC(ch) || !ch->pcdata
			|| ( !ch->pcdata->area && get_trust( ch ) < LEVEL_STONE_ADEPT ) )
	{
		send_to_char( "You don't have an assigned area.\r\n", ch );
		return;
	}

	tarea = ch->pcdata->area;
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( tarea )
	{
		if ( arg1[0] == '\0' )		/* cleaned a big scary mess */
			lrange = tarea->low_r_vnum;	/* here.	    -Thoric */
		else
			lrange = dotted_to_vnum(ch->GetInRoom()->vnum, arg1);
		if ( arg2[0] == '\0' )
			trange = tarea->hi_r_vnum;
		else
			trange = dotted_to_vnum(ch->GetInRoom()->vnum, arg2);

		if ( ( lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum )
				&& get_trust( ch ) < LEVEL_STONE_ADEPT )
		{
			send_to_char("That is out of your vnum range.\r\n", ch);
			return;
		}
	}
	else
	{
		lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
		trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
	}

	for ( vnum = lrange; vnum <= trange; vnum++ )
	{
		if ( (room = get_room_index( vnum )) == NULL )
			continue;
		ch_printf( ch, "%5s) %-25s (%s)\r\n", vnum_to_dotted(vnum),
			room->name_.c_str(), flag_string(room->room_flags, r_flags)  );
	}
	return;
}

void do_olist(CHAR_DATA *ch, const char* argument)
{
    OBJ_INDEX_DATA	*obj;
    int			 vnum;
    AREA_DATA		*tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;

    /*
     * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
     */
    if ( IS_NPC(ch)
       || !ch->pcdata
       || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_STONE_ADEPT ) )
    {
	send_to_char( "You don't have an assigned area.\r\n", ch );
	return;
    }
    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
      if ( arg1[0] == '\0' )		/* cleaned a big scary mess */
        lrange = tarea->low_o_vnum;	/* here.	    -Thoric */
      else
        lrange = dotted_to_vnum(ch->GetInRoom()->vnum, arg1);
      if ( arg2[0] == '\0' )
        trange = tarea->hi_o_vnum;
      else
        trange = dotted_to_vnum(ch->GetInRoom()->vnum, arg2);

      if ((lrange < tarea->low_o_vnum || trange > tarea->hi_o_vnum)
      &&   get_trust( ch ) < LEVEL_STONE_ADEPT )
      {
	send_to_char("That is out of your vnum range.\r\n", ch);
	return;
      }
    }
   else
    {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 3 );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
	if ( (obj = get_obj_index( vnum )) == NULL )
	  continue;
	ch_printf( ch, "%5s) %-25s (%s)\r\n", vnum_to_dotted(vnum),
					     obj->name_.c_str(),
					     obj->shortDesc_.c_str() );
    }
    return;
}

void do_mlist(CHAR_DATA *ch, const char* argument)
{
    MOB_INDEX_DATA	*mob;
    int			 vnum;
    AREA_DATA		*tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;

    if ( IS_NPC(ch)
       || !ch->pcdata
       || (!ch->pcdata->area && get_trust(ch)<LEVEL_STONE_ADEPT)
       )
    {
	send_to_char( "You don't have an assigned area.\r\n", ch );
	return;
    }

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
      if ( arg1[0] == '\0' )		/* cleaned a big scary mess */
        lrange = tarea->low_m_vnum;	/* here.	    -Thoric */
      else
        lrange = dotted_to_vnum(ch->GetInRoom()->vnum, arg1);
      if ( arg2[0] == '\0' )
        trange = tarea->hi_m_vnum;
      else
        trange = dotted_to_vnum(ch->GetInRoom()->vnum, arg2);

      if ( ( lrange < tarea->low_m_vnum || trange > tarea->hi_m_vnum )
	&& get_trust( ch ) < LEVEL_STONE_ADEPT )
      {
  	send_to_char("That is out of your vnum range.\r\n", ch);
	return;
      }
    }
    else
    {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
	  if ( (mob = get_mob_index( vnum )) == NULL )
	    continue;
	  ch_printf( ch, "%5s) %-25s (%s)\r\n", vnum_to_dotted(vnum),
					 mob->playerName_.c_str(),
					 mob->shortDesc_.c_str() );
    }
}

/*
 * SKRYPT BUILDING INTERFACE
 * This code is (c) Ksilyan, belongs to him,
 * is his only, all that stuff!
 */

// Kill all events associated with an object.
// (i.e. all owned events owned by object)
void do_killscriptilyanevent(Character * ch, const char* argument)
{
	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't kill scriptilyan events\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	if ( !argument || strlen(argument) == 0 )
	{
		ch->sendText("Syntax: killscriptilyanevent <type> <reference>.\r\n");
		ch->sendText("Where type is the type of the thing, i.e. room, object or mobile.\r\n");
		ch->sendText("Where reference is the name/vnum of the thing.\r\n");
		return;
	}

	// first arg: type of skryptable (room, obj, mob...)
	char skryptableType[MAX_INPUT_LENGTH];

	// the skrytable object whose events we kill
	//cabSkryptable * victim;

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, skryptableType );

	if ( !str_cmp(skryptableType, "room") )
	{
		// Get the room:
	}
	else if ( !str_cmp(skryptableType, "object") )
	{
		// Get the object:
	}
	else if ( !str_cmp(skryptableType, "mobile") )
	{
		// Get the mobile:
	}
	else
	{
		ch->sendText("The first argument must be either room, object or mobile.\r\n");
		return;
	}
}


void do_mscriptilyan(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA * victim;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't mscriptilyan\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
		default:
			break;
		case SUB_MSKRYPT_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_mscriptilyan: sub_mscriptilyan_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			victim = (CHAR_DATA *) ch->dest_buf;

			SkryptContainer * skryptContainer = victim->skryptGetContainer();

			if ( skryptContainer == NULL )
			{
				gTheWorld->LogBugString("do_mscriptilyan: sub_mscriptilyan victim: NULL skryptContainer_.");
				ch->sendText("Fatal error.\r\n");
				ch->substate = SUB_NONE;
				return;
			}

			if ( skryptContainer->skryptProgram_ )
				delete skryptContainer->skryptProgram_;

			skryptContainer->skryptProgram_ = NULL;

			skryptContainer->skryptFromFile_ = false;
			skryptContainer->skryptFileName_ = "";

			skryptContainer->skryptProgramSource_ = copy_buffer_string( ch );
			stop_editing( ch );
			try {
				skryptContainer->skryptProgram_ = SkryptInterpret(skryptContainer->skryptProgramSource_);
				victim->skryptSetContainer(skryptContainer); // reset symbols and such
				ch->sendText("Done!\r\n");
			} catch (eSkryptError E) {
				string buf;
				if (E.Line == 0)
				{
					buf = "Scriptilyan error, mob ";
					buf += string( vnum_to_dotted(victim->pIndexData->vnum) );
					buf += ": " + E.ErrorMessage;
				}
				else
				{
					buf = "Scriptilyan error, mob ";
					buf += string( vnum_to_dotted(victim->pIndexData->vnum) );
					buf += ", line " + E.Line;
					buf += ": " + E.ErrorMessage;
				}
				log_string(buf.c_str());
				if (skryptContainer->skryptProgram_)
					delete skryptContainer->skryptProgram_;
				skryptContainer->skryptProgram_ = NULL;
			}

			return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: mscriptilyan <victim> <command>\r\n", ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Command being one of:\r\n",			ch );
		send_to_char( "  edit show delete setfile reload\r\n",		ch );
		return;
	}

	if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
	{
		if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "No one like that in all the realms.\r\n", ch );
			return;
		}
	}

	if ( get_trust( ch ) < victim->level || !IS_NPC(victim) )
	{
		send_to_char( "You can't do that!\r\n", ch );
		return;
	}

	if ( !can_mmodify( ch, victim ) )
		return;

	if ( !IS_SET( victim->act, ACT_PROTOTYPE ) )
	{
		send_to_char( "A mobile must have a prototype flag to have its scriptilyan edited.\r\n", ch );
		return;
	}

	if ( !victim->skryptGetContainer() )
	{
		ch->sendText("FATAL: mscriptilyan victim has no SkryptContainer!");
		return;
	}

	//skryptProgram = victim->skryptContainer_->skryptProgram_;

	set_char_color( AT_GREEN, ch );

	if ( !str_prefix( arg2, "show" ) )
	{
		if ( !victim->HasSkrypt() )
		{
			ch->sendText( "That mobile has no scriptilyan program.\r\n" );
			return;
		}
		ch_printf( ch, "----------------\r\n%s\r\n", victim->skryptGetContainer()->skryptProgramSource_.c_str() );
		return;
	}

	if ( !str_prefix( arg2, "edit" ) )
	{
		ch->dest_buf = victim;

		start_editing(ch, (char *) victim->skryptGetContainer()->skryptProgramSource_.c_str() );
		ch->substate = SUB_MSKRYPT_EDIT;

		return;
	}

	if ( !str_prefix( arg2, "setfile" ) )
	{
		char arg3[MAX_INPUT_LENGTH];
		argument = one_argument(argument, arg3);

		if ( strlen(arg3) == 0 )
		{
			victim->skryptGetContainer()->skryptFileName_ = "";
			victim->skryptGetContainer()->skryptFromFile_ = false;
			ch->sendText("Removed the mob's scriptilyan file.\r\n");
			return;
		}

		victim->skryptGetContainer()->skryptFromFile_ = true;
		victim->skryptGetContainer()->skryptFileName_ = arg3;

		victim->skryptGetContainer()->skryptReload();
		victim->skryptSetContainer( victim->skryptGetContainer() ); // reset symbols and such

		string buf;
		buf = "Set the mob's sciptilyan source to filename: ";
		buf += victim->skryptGetContainer()->skryptFileName_.str();
		buf += "!\r\n";
		ch->sendText(buf);
		return;
	}

	if ( !str_prefix( arg2, "reload" ) )
	{
		if ( !victim->HasSkrypt() )
		{
			ch->sendText( "That mobile has no scriptilyan program.\r\n" );
			return;
		}

		if ( !victim->skryptGetContainer()->skryptFromFile_ )
		{
			ch->sendText("That mob's scriptilyan does not come from a file.\r\n");
			return;
		}

		victim->skryptGetContainer()->skryptReload();
		victim->skryptSetContainer( victim->skryptGetContainer() ); // reset symbols and such

		// Initialize Skrypt.
		list<Argument*> arguments; // empty
		victim->skryptSendEvent("initMob", arguments);

		ch->sendText("Done.\r\n");
		return;
	}

	if ( !str_prefix( arg2, "delete" ) )
	{
		if ( !victim->HasSkrypt() )
		{
			ch->sendText("That mobile has no scriptilyan program.\r\n");
			return;
		}
		if (victim->skryptGetContainer()->skryptProgram_)
			delete victim->skryptGetContainer()->skryptProgram_;
		victim->skryptGetContainer()->skryptProgram_ = NULL;

		victim->skryptGetContainer()->skryptProgramSource_ = "";

		string buf;
		buf = "Removing scriptilyan program for mobile ";
		buf.append( vnum_to_dotted(victim->pIndexData->vnum) );
		buf += ".\r\n";
		ch->sendText( buf );
		return;
	}

	// Default: show usage.
	do_mscriptilyan( ch, "" );
}

void do_oscriptilyan(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	OBJ_DATA * obj;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't oscriptilyan\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
		default:
			break;
		case SUB_OSKRYPT_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_oscriptilyan: sub_oscriptilyan_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			obj = (OBJ_DATA *) ch->dest_buf;

			SkryptContainer * skryptContainer = obj->skryptGetContainer();

			if ( !skryptContainer )
			{
				gTheWorld->LogBugString("FATAL: sub_oscriptilyan_edit object: NULL SkryptContainer");
				ch->sendText("Fatal error.\r\n");
				ch->substate = SUB_NONE;
				return;
			}

			if ( skryptContainer->skryptProgram_ )
				delete skryptContainer->skryptProgram_;

			skryptContainer->skryptProgram_ = NULL;

			skryptContainer->skryptFromFile_ = false;
			skryptContainer->skryptFileName_ = "";

			skryptContainer->skryptProgramSource_ = copy_buffer_string( ch );
			stop_editing( ch );
			try
			{
				skryptContainer->skryptProgram_ = SkryptInterpret(skryptContainer->skryptProgramSource_);
				obj->skryptSetContainer(skryptContainer); // reload symbols and such
				ch->sendText("Done!\r\n");
			}
			catch (eSkryptError E)
			{
				string buf;
				if (E.Line == 0)
				{
					buf = "Scriptilyan error, object ";
					buf.append(vnum_to_dotted(obj->pIndexData->vnum));
					buf += ": " + E.ErrorMessage;
				}
				else
				{
					buf = "Scriptilyan error, obj ";
					buf.append(vnum_to_dotted(obj->pIndexData->vnum));
					buf += ", line " + E.Line;
					buf += ": " + E.ErrorMessage;
				}
				log_string(buf.c_str());
				if (skryptContainer->skryptProgram_)
					delete skryptContainer->skryptProgram_;
				skryptContainer->skryptProgram_ = NULL;
			}

			return;
	}

	argument = smash_tilde_static ( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: oscriptilyan <victim> <command>\r\n", ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Command being one of:\r\n",			ch );
		send_to_char( "  edit show delete setfile\r\n",		ch );
		return;
	}

	if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
	{
		if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You aren't carrying that.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "Nothing like that in all the realms.\r\n", ch );
			return;
		}
	}

	if ( !can_omodify( ch, obj ) )
		return;

	if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
		send_to_char( "An object must have a prototype flag to have its scriptilyan edited.\r\n", ch );
		return;
	}

	if ( !obj->skryptGetContainer() )
	{
		ch->sendText("Fatal: object has no SkryptContainer.\r\r");
		return;
	}

	set_char_color( AT_GREEN, ch );

	if ( !str_prefix( arg2, "show" ) )
	{
		if ( !obj->HasSkrypt() )
		{
			ch->sendText( "That object has no scriptilyan program.\r\n" );
			return;
		}
		ch_printf( ch, "----------------\r\n%s\r\n", obj->skryptGetContainer()->skryptProgramSource_.c_str() );
		return;
	}

	if ( !str_prefix( arg2, "edit" ) )
	{
		ch->dest_buf = obj;

		start_editing( ch, (char *) obj->skryptGetContainer()->skryptProgramSource_.c_str() );

		ch->substate = SUB_OSKRYPT_EDIT;

		return;
	}

	if ( !str_prefix( arg2, "setfile" ) )
	{
		char arg3[MAX_INPUT_LENGTH];
		argument = one_argument(argument, arg3);

		if ( strlen(arg3) == 0 )
		{
			obj->skryptGetContainer()->skryptFileName_ = "";
			obj->skryptGetContainer()->skryptFromFile_ = false;
			ch->sendText("Removed the object's scriptilyan file.\r\n");
			return;
		}

		obj->skryptGetContainer()->skryptFromFile_ = true;
		obj->skryptGetContainer()->skryptFileName_ = arg3;

		obj->skryptGetContainer()->skryptReload();
		obj->skryptSetContainer( obj->skryptGetContainer() ); // reload symbols and such

		string buf;
		buf = "Set the objects's sciptilyan source to filename: ";
		buf += obj->skryptGetContainer()->skryptFileName_.str();
		buf += "!\r\n";
		ch->sendText(buf);
		return;
	}

	if ( !str_prefix( arg2, "reload" ) )
	{
		if ( !obj->HasSkrypt() )
		{
			ch->sendText( "That object has no scriptilyan program.\r\n" );
			return;
		}

		if ( !obj->skryptGetContainer()->skryptFromFile_ )
		{
			ch->sendText("That object's scriptilyan does not come from a file.\r\n");
			return;
		}

		obj->skryptGetContainer()->skryptReload();
		obj->skryptSetContainer( obj->skryptGetContainer() ); // reload symbols and such

		// Initialize Skrypt.
		list<Argument*> arguments; // empty
		obj->skryptSendEvent("initObj", arguments);

		ch->sendText("Done.\r\n");
		return;
	}

	if ( !str_prefix( arg2, "delete" ) )
	{
		if ( !obj->HasSkrypt() )
		{
			ch->sendText("That object has no scriptilyan program.\r\n");
			return;
		}
		if (obj->skryptGetContainer()->skryptProgram_)
			delete obj->skryptGetContainer()->skryptProgram_;
		obj->skryptGetContainer()->skryptProgram_ = NULL;

		string buf;
		buf = "Removing scriptilyan program for object ";
		buf.append(vnum_to_dotted(obj->pIndexData->vnum));
		buf += ".\r\n";
		ch->sendText( buf );
		return;
	}

	// if nothing found: show usage
	do_oscriptilyan( ch, "" );
}

// NOTE: Rooms are have their own containers.
// The room load routine sets the cabSkryptable's
// SkryptContainer to its internal container.

void do_rscriptilyan(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];

	ROOM_INDEX_DATA * room;


	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't rscriptilyan\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
		default:
			break;
		case SUB_RSKRYPT_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ksilyan.\r\n", ch );
				bug( "do_rscriptilyan: sub_oscriptilyan_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			room = (ROOM_INDEX_DATA *) ch->dest_buf;

			SkryptContainer * skryptContainer = room->skryptGetContainer();

			if ( !skryptContainer )
			{
				gTheWorld->LogBugString("do_rscriptilyan: sub_rscriptilyan_edit: NULL room->RoomSkryptContainer");
				ch->sendText("Fatal erro: no room skrypt container.\r\n");
				ch->substate = SUB_NONE;
				return;
			}

			if ( skryptContainer->skryptProgram_ )
				delete skryptContainer->skryptProgram_;
			skryptContainer->skryptProgram_ = NULL;

			skryptContainer->skryptFileName_ = "";
			skryptContainer->skryptFromFile_ = false;

			skryptContainer->skryptProgramSource_ = copy_buffer_string( ch );
			stop_editing( ch );
			try
			{
				skryptContainer->skryptProgram_ = SkryptInterpret(skryptContainer->skryptProgramSource_);
				room->skryptSetContainer( skryptContainer ); // reload symbols and such
				ch->sendText("Done!\r\n");
			}
			catch (eSkryptError E)
			{
				string buf;
				if (E.Line == 0)
				{
					buf = "Scriptilyan error, room ";
					buf.append(vnum_to_dotted(room->vnum));
					buf += ": " + E.ErrorMessage;
				}
				else
				{
					buf = "Scriptilyan error, room ";
					buf.append(vnum_to_dotted(room->vnum));
					buf += ", line " + E.Line;
					buf += ": " + E.ErrorMessage;
				}
				log_string(buf.c_str());
				if (skryptContainer->skryptProgram_)
					delete skryptContainer->skryptProgram_;
				skryptContainer->skryptProgram_ = NULL;
			}

			return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: rscriptilyan <command>\r\n", ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Command being one of:\r\n",			ch );
		send_to_char( "  edit show delete setfile\r\n",		ch );
		send_to_char( "You should be standing in the room you want to edit.\r\n", ch);
		return;
	}

	room = ch->GetInRoom();
	if ( !can_rmodify( ch, room ) )
		return;

	set_char_color( AT_GREEN, ch );

	if ( !str_prefix( arg1, "show" ) )
	{
		if ( !room->HasSkrypt() )
		{
			ch->sendText( "This room has no scriptilyan program.\r\n" );
			return;
		}
		ch_printf( ch, "----------------\r\n%s\r\n", room->skryptGetContainer()->skryptProgramSource_.c_str() );
		return;
	}

	if ( !str_prefix( arg1, "edit" ) )
	{
		ch->dest_buf = room;

		start_editing(ch, (char *) room->skryptGetContainer()->skryptProgramSource_.c_str() );

		ch->substate = SUB_RSKRYPT_EDIT;

		return;
	}

	if ( !str_prefix( arg1, "setfile" ) )
	{
		char arg2[MAX_INPUT_LENGTH];
		argument = one_argument(argument, arg2);

		if ( strlen(arg2) == 0 )
		{
			room->skryptGetContainer()->skryptFileName_ = "";
			room->skryptGetContainer()->skryptFromFile_ = false;
			ch->sendText("Removed the room's scriptilyan file.\r\n");
			return;
		}

		room->skryptGetContainer()->skryptFromFile_ = true;
		room->skryptGetContainer()->skryptFileName_ = arg2;

		room->skryptGetContainer()->skryptReload();
		room->skryptSetContainer( room->skryptGetContainer() ); // reload symbols and such

		string buf;
		buf = "Set the room's sciptilyan source to filename: ";
		buf += room->skryptGetContainer()->skryptFileName_.str();
		buf += "!\r\n";
		ch->sendText(buf);
		return;
	}

	if ( !str_prefix( arg1, "reload" ) )
	{
		if ( !room->HasSkrypt() )
		{
			ch->sendText( "That room has no scriptilyan program.\r\n" );
			return;
		}

		if ( !room->skryptGetContainer()->skryptFromFile_ )
		{
			ch->sendText("That room's scriptilyan does not come from a file.\r\n");
			return;
		}

		room->skryptGetContainer()->skryptReload();
		room->skryptSetContainer( room->skryptGetContainer() ); // reload symbols and such

		// Initialize Skrypt.
		list<Argument*> arguments; // empty
		room->skryptSendEvent("initRoom", arguments);

		ch->sendText("Done.\r\n");
		return;
	}

	if ( !str_prefix( arg1, "delete" ) )
	{
		if ( !room->HasSkrypt() )
		{
			ch->sendText("That room has no scriptilyan program.\r\n");
			return;
		}
		if (room->skryptGetContainer()->skryptProgram_)
			delete room->skryptGetContainer()->skryptProgram_;
		room->skryptGetContainer()->skryptProgram_ = NULL;

		string buf;
		buf = "Removing scriptilyan program for room ";
		buf.append(vnum_to_dotted(room->vnum));
		buf += ".\r\n";
		ch->sendText( buf );
		return;
	}

	// default: show usage
	do_rscriptilyan( ch, "" );
}

void mpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, const char *argument )
{
	if ( mptype != -1 )
	{
	  mprg->type = ((long long)1) << mptype;
	  if ( mprg->arglist )
	    STRFREE( mprg->arglist );
	  mprg->arglist = STRALLOC( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if ( !mprg->comlist )
	   mprg->comlist = STRALLOC( "" );
	start_editing( ch, mprg->comlist );
	return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
void do_mpedit(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	CHAR_DATA  *victim;
	MPROG_DATA *mprog, *mprg, *mprg_next;
	int value, cnt;
	int mptype;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't mpedit\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
		default:
			break;
		case SUB_MPROG_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			mprog	 = (MPROG_DATA *) ch->dest_buf;
			if ( mprog->comlist )
				STRFREE( mprog->comlist );

			string prog;
			prog = strip_cr( copy_buffer_string(ch).c_str() );
			//mprog->comlist = copy_buffer( ch );
			mprog->comlist = STRALLOC( prog.c_str() );

			stop_editing( ch );
			return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	value = atoi( arg3 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: mpedit <victim> <command> [number] <program> <value>\r\n", ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Command being one of:\r\n",			ch );
		send_to_char( "  add delete insert edit list\r\n",		ch );
		send_to_char( "Program being one of:\r\n",			ch );
		send_to_char( "  act speech rand fight hitprcnt greet allgreet\r\n", ch );
		send_to_char( "  entry give bribe death time hour script\r\n",	ch );
		return;
	}

	if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
	{
		if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "No one like that in all the realms.\r\n", ch );
			return;
		}
	}

	if ( get_trust( ch ) < victim->level || !IS_NPC(victim) )
	{
		send_to_char( "You can't do that!\r\n", ch );
		return;
	}

	if ( !can_mmodify( ch, victim ) )
		return;

	if ( !IS_SET( victim->act, ACT_PROTOTYPE ) )
	{
		send_to_char( "A mobile must have a prototype flag to be mpset.\r\n", ch );
		return;
	}

	mprog = victim->pIndexData->mudprogs;

	set_char_color( AT_GREEN, ch );

	if ( !str_prefix( arg2, "list" ) )
	{
		cnt = 0;
		if ( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		for ( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf( ch, "%d>%s %s\r\n%s\r\n",
					++cnt,
					mprog_type_to_name( mprg->type ),
					mprg->arglist,
					mprg->comlist );
		return;
	}

	if ( !str_prefix( arg2, "edit" ) )
	{
		if ( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if ( arg4[0] != '\0' )
		{
			mptype = get_mpflag( arg4 );
			if ( mptype == -1 )
			{
				send_to_char( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mpedit( ch, mprg, mptype, argument );
				victim->pIndexData->progtypes = 0;
				for ( mprg = mprog; mprg; mprg = mprg->next )
					victim->pIndexData->progtypes |= mprg->type;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "delete" ) )
	{
		int num;
		bool found;

		if ( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0; found = FALSE;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mptype = mprg->type;
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
			if ( IS_SET( mprg->type, mptype ) )
				num++;
		if ( value == 1 )
		{
			mprg_next = victim->pIndexData->mudprogs;
			victim->pIndexData->mudprogs = mprg_next->next;
		}
		else
			for ( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if ( ++cnt == (value - 1) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		STRFREE( mprg_next->arglist );
		STRFREE( mprg_next->comlist );
		DISPOSE( mprg_next );
		if ( num <= 1 )
			REMOVE_BIT( victim->pIndexData->progtypes, mptype );
		send_to_char( "Program removed.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "insert" ) )
	{
		if ( !mprog )
		{
			send_to_char( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		mptype = get_mpflag( arg4 );
		if ( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		if ( value == 1 )
		{
			CREATE( mprg, MPROG_DATA, 1 );
			victim->pIndexData->progtypes |= (((long long)1) << mptype );
			mpedit( ch, mprg, mptype, argument );
			mprg->next = mprog;
			victim->pIndexData->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value && mprg->next )
			{
				CREATE( mprg_next, MPROG_DATA, 1 );
				victim->pIndexData->progtypes |= (((long long)1) << mptype );
				mpedit( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next	= mprg_next;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "add" ) )
	{
		mptype = get_mpflag( arg3 );
		if ( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if ( mprog != NULL )
			for ( ; mprog->next; mprog = mprog->next );
		CREATE( mprg, MPROG_DATA, 1 );
		if ( mprog )
			mprog->next			= mprg;
		else
			victim->pIndexData->mudprogs	= mprg;
		victim->pIndexData->progtypes	|= ( ((long long)1) << mptype );
		mpedit( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}

	do_mpedit( ch, "" );
}

void do_opedit(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	OBJ_DATA   *obj;
	MPROG_DATA *mprog, *mprg, *mprg_next;
	int value, mptype, cnt;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't opedit\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
		default:
			break;
		case SUB_MPROG_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			mprog	 = (MPROG_DATA *) ch->dest_buf;
			if ( mprog->comlist )
				STRFREE( mprog->comlist );

			string prog;
			prog = strip_cr( copy_buffer_string(ch).c_str() );
			//mprog->comlist = copy_buffer( ch );
			mprog->comlist = STRALLOC( prog.c_str() );

			stop_editing( ch );
			return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	value = atoi( arg3 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Syntax: opedit <object> <command> [number] <program> <value>\r\n", ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Command being one of:\r\n",			ch );
		send_to_char( "  add delete insert edit list\r\n",		ch );
		send_to_char( "Program being one of:\r\n",			ch );
		send_to_char( "  act speech rand wear remove sac zap get\r\n",  ch );
		send_to_char( "  drop damage repair greet exa use\r\n",ch );
		send_to_char( "  pull push (for levers,pullchains,buttons)\r\n",ch );
		send_to_char( "\r\n", ch);
		send_to_char( "Object should be in your inventory to edit.\r\n",ch);
		return;
	}

	if ( get_trust( ch ) < LEVEL_STONE_ADEPT )
	{
		if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You aren't carrying that.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
		{
			send_to_char( "Nothing like that in all the realms.\r\n", ch );
			return;
		}
	}

	if ( !can_omodify( ch, obj ) )
		return;

	if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	{
		send_to_char( "An object must have a prototype flag to be opset.\r\n", ch );
		return;
	}

	mprog = obj->pIndexData->mudprogs;

	set_char_color( AT_GREEN, ch );

	if ( !str_prefix( arg2, "list" ) )
	{
		cnt = 0;
		if ( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		for ( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf( ch, "%d>%s %s\r\n%s\r\n",
					++cnt,
					mprog_type_to_name( mprg->type ),
					mprg->arglist,
					mprg->comlist );
		return;
	}

	if ( !str_prefix( arg2, "edit" ) )
	{
		if ( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if ( arg4[0] != '\0' )
		{
			mptype = get_mpflag( arg4 );
			if ( mptype == -1 )
			{
				send_to_char( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mpedit( ch, mprg, mptype, argument );
				obj->pIndexData->progtypes = 0;
				for ( mprg = mprog; mprg; mprg = mprg->next )
					obj->pIndexData->progtypes |= mprg->type;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "delete" ) )
	{
		int num;
		bool found;

		if ( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0; found = FALSE;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mptype = mprg->type;
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
			if ( IS_SET( mprg->type, mptype ) )
				num++;
		if ( value == 1 )
		{
			mprg_next = obj->pIndexData->mudprogs;
			obj->pIndexData->mudprogs = mprg_next->next;
		}
		else
			for ( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if ( ++cnt == (value - 1) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		STRFREE( mprg_next->arglist );
		STRFREE( mprg_next->comlist );
		DISPOSE( mprg_next );
		if ( num <= 1 )
			REMOVE_BIT( obj->pIndexData->progtypes, mptype );
		send_to_char( "Program removed.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "insert" ) )
	{
		if ( !mprog )
		{
			send_to_char( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg4 );
		mptype = get_mpflag( arg4 );
		if ( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		if ( value == 1 )
		{
			CREATE( mprg, MPROG_DATA, 1 );
			obj->pIndexData->progtypes	|= ( ((long long)1) << mptype );
			mpedit( ch, mprg, mptype, argument );
			mprg->next = mprog;
			obj->pIndexData->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value && mprg->next )
			{
				CREATE( mprg_next, MPROG_DATA, 1 );
				obj->pIndexData->progtypes |= ( ((long long)1) << mptype );
				mpedit( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next	= mprg_next;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "add" ) )
	{
		mptype = get_mpflag( arg3 );
		ch_printf(ch, "adding objprog type '%s' value %d\r\n",arg3,mptype);
		if ( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if ( mprog != NULL )
			for ( ; mprog->next; mprog = mprog->next );
		CREATE( mprg, MPROG_DATA, 1 );
		if ( mprog )
			mprog->next			 = mprg;
		else
			obj->pIndexData->mudprogs	 = mprg;
		obj->pIndexData->progtypes	|= ( ((long long)1) << mptype );
		mprg->next = NULL;
		mpedit( ch, mprg, mptype, argument );
		return;
	}

	do_opedit( ch, "" );
}



/*
 * RoomProg Support
 */
void rpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, char *argument )
{
	if ( mptype != -1 )
	{
	  mprg->type = 1 << mptype;
	  if ( mprg->arglist )
	    STRFREE( mprg->arglist );
	  mprg->arglist = STRALLOC( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if(!mprg->comlist)
          mprg->comlist = STRALLOC("");
	start_editing( ch, mprg->comlist );
	return;
}

void do_rpedit(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	MPROG_DATA *mprog, *mprg, *mprg_next;
	int value, mptype, cnt;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mob's can't rpedit\r\n", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		send_to_char( "You have no descriptor\r\n", ch );
		return;
	}

	switch( ch->substate )
	{
		default:
			break;
		case SUB_MPROG_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char( "Fatal error: report to Ydnat.\r\n", ch );
				bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			mprog	 = (MPROG_DATA *) ch->dest_buf;
			if ( mprog->comlist )
				STRFREE( mprog->comlist );

			string prog;
			prog = strip_cr( copy_buffer_string(ch).c_str() );
			//mprog->comlist = copy_buffer( ch );
			mprog->comlist = STRALLOC( prog.c_str() );

			stop_editing( ch );
			return;
	}

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	value = atoi( arg2 );
	/* argument = one_argument( argument, arg3 ); */

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: rpedit <command> [number] <program> <value>\r\n", ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "Command being one of:\r\n",			ch );
		send_to_char( "  add delete insert edit list\r\n",		ch );
		send_to_char( "Program being one of:\r\n",			ch );
		send_to_char( "  act speech rand sleep rest rfight enter\r\n",  ch );
		send_to_char( "  leave death\r\n",                              ch );
		send_to_char( "\r\n",						ch );
		send_to_char( "You should be standing in room you wish to edit.\r\n",ch);
		return;
	}

	if ( !can_rmodify( ch, ch->GetInRoom() ) )
		return;

	mprog = ch->GetInRoom()->mudprogs;

	set_char_color( AT_GREEN, ch );

	if ( !str_prefix( arg1, "list" ) )
	{
		cnt = 0;
		if ( !mprog )
		{
			send_to_char( "This room has no room programs.\r\n", ch );
			return;
		}
		for ( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf( ch, "%d>%s %s\r\n%s\r\n",
					++cnt,
					mprog_type_to_name( mprg->type ),
					mprg->arglist,
					mprg->comlist );
		return;
	}

	if ( !str_prefix( arg1, "edit" ) )
	{
		if ( !mprog )
		{
			send_to_char( "This room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg3 );
		if ( arg3[0] != '\0' )
		{
			mptype = get_mpflag( arg3 );
			if ( mptype == -1 )
			{
				send_to_char( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mpedit( ch, mprg, mptype, argument );
				ch->GetInRoom()->progtypes = 0;
				for ( mprg = mprog; mprg; mprg = mprg->next )
					ch->GetInRoom()->progtypes |= mprg->type;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg1, "delete" ) )
	{
		int num;
		bool found;

		if ( !mprog )
		{
			send_to_char( "That room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg3 );
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0; found = FALSE;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mptype = mprg->type;
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
			if ( IS_SET( mprg->type, mptype ) )
				num++;
		if ( value == 1 )
		{
			mprg_next = ch->GetInRoom()->mudprogs;
			ch->GetInRoom()->mudprogs = mprg_next->next;
		}
		else
			for ( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if ( ++cnt == (value - 1) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		STRFREE( mprg_next->arglist );
		STRFREE( mprg_next->comlist );
		DISPOSE( mprg_next );
		if ( num <= 1 )
			REMOVE_BIT( ch->GetInRoom()->progtypes, mptype );
		send_to_char( "Program removed.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg2, "insert" ) )
	{
		if ( !mprog )
		{
			send_to_char( "That room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument( argument, arg3 );
		mptype = get_mpflag( arg2 );
		if ( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if ( value < 1 )
		{
			send_to_char( "Program not found.\r\n", ch );
			return;
		}
		if ( value == 1 )
		{
			CREATE( mprg, MPROG_DATA, 1 );
			ch->GetInRoom()->progtypes |= ( ((long long)1) << mptype );
			mpedit( ch, mprg, mptype, argument );
			mprg->next = mprog;
			ch->GetInRoom()->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value && mprg->next )
			{
				CREATE( mprg_next, MPROG_DATA, 1 );
				ch->GetInRoom()->progtypes |= ( ((long long)1) << mptype );
				mpedit( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next	= mprg_next;
				return;
			}
		}
		send_to_char( "Program not found.\r\n", ch );
		return;
	}

	if ( !str_prefix( arg1, "add" ) )
	{
		mptype = get_mpflag( arg2 );
		if ( mptype == -1 )
		{
			send_to_char( "Unknown program type.\r\n", ch );
			return;
		}
		if ( mprog )
			for ( ; mprog->next; mprog = mprog->next );
		CREATE( mprg, MPROG_DATA, 1 );
		if ( mprog )
			mprog->next		= mprg;
		else
			ch->GetInRoom()->mudprogs	= mprg;
		ch->GetInRoom()->progtypes |= ( ((long long)1) << mptype );
		mpedit( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}

	do_rpedit( ch, "" );
}

void do_rdelete(CHAR_DATA *ch, const char* argument)
{
       char arg[MAX_INPUT_LENGTH];
       ROOM_INDEX_DATA *location;

       argument = one_argument( argument, arg );

       /* Temporarily disable this command. */
   /*    return;*/

       if ( arg[0] == '\0' )
     {
	send_to_char( "Delete which room?\r\n", ch );
	return;
     }

       /* Find the room. */
       if ( ( location = find_location( ch, arg ) ) == NULL )
     {
	send_to_char( "No such location.\r\n", ch );
	return;
     }

       /* Does the player have the right to delete this room? */
       if ( get_trust( ch ) < sysdata.level_modify_proto
	       && ( location->vnum < ch->pcdata->r_range_lo ||
		            location->vnum > ch->pcdata->r_range_hi ) )
     {
	      send_to_char( "That room is not in your assigned range.\r\n", ch );
	      return;
     }

       /* We could go to the trouble of clearing out the room, but why? */
       /* Delete_room does that anyway, but this is probably safer:) */
       if ( location->first_person || location->first_content  || location->QuestNoteId != 0)
     {
	      send_to_char( "The room must be empty first.\r\n", ch );
	      return;
     }

       /* Ok, we've determined that the room exists, it is empty and the
	*        player has the authority to delete it, so let's dump the thing.
	*        The function to do it is in db.c so it can access the top-room
	*        variable. */
       delete_room( location );

       send_to_char( "Room deleted.\r\n", ch );
       return;
}

void do_odelete(CHAR_DATA *ch, const char* argument)
{
       char arg[MAX_INPUT_LENGTH];
       OBJ_INDEX_DATA *obj;
       OBJ_DATA *temp;

       argument = one_argument( argument, arg );

       /* Temporarily disable this command. */
   /*    return;*/

       if ( arg[0] == '\0' )
     {
	send_to_char( "Delete which object?\r\n", ch );
	return;
     }

       /* Find the object. */
       if (!(obj = get_obj_index(dotted_to_vnum(ch->GetInRoom()->vnum, arg))))
     {
	      if (!(temp = get_obj_here(ch, arg)))
	  {
	             send_to_char( "No such object.\r\n", ch );
	             return;
	  }
	      obj = temp->pIndexData;
     }

       /* Does the player have the right to delete this room? */
       if ( get_trust( ch ) < sysdata.level_modify_proto
	       && ( obj->vnum < ch->pcdata->o_range_lo ||
		            obj->vnum > ch->pcdata->o_range_hi ) )
     {
	      send_to_char( "That object is not in your assigned range.\r\n", ch );
	      return;
     }

       /* Ok, we've determined that the room exists, it is empty and the
	*        player has the authority to delete it, so let's dump the thing.
	*        The function to do it is in db.c so it can access the top-room
	*        variable. */
       delete_obj( obj );

       send_to_char( "Object deleted.\r\n", ch );
       return;
}

void do_mdelete(CHAR_DATA *ch, const char* argument)
{
       char arg[MAX_INPUT_LENGTH];
       MOB_INDEX_DATA *mob;
       CHAR_DATA *temp;

       argument = one_argument( argument, arg );

       /* Temporarily disable this command. */
   /*    return;*/

       if ( arg[0] == '\0' )
     {
	send_to_char( "Delete which mob?\r\n", ch );
	return;
     }

       /* Find the mob. */
       if (!(mob = get_mob_index(dotted_to_vnum(ch->GetInRoom()->vnum, arg))))
     {
	      if (!(temp = get_char_room(ch, arg)) || !IS_NPC(temp))
	  {
	             send_to_char( "No such mob.\r\n", ch );
	             return;
	  }
	      mob = temp->pIndexData;
     }

       /* Does the player have the right to delete this room? */
       if ( get_trust( ch ) < sysdata.level_modify_proto
	       && ( mob->vnum < ch->pcdata->m_range_lo ||
		            mob->vnum > ch->pcdata->m_range_hi ) )
     {
	      send_to_char( "That mob is not in your assigned range.\r\n", ch );
	      return;
     }

       /* Ok, we've determined that the mob exists and the player has the
	*        authority to delete it, so let's dump the thing.
	*        The function to do it is in db.c so it can access the top_mob_index
	*        variable. */
       delete_mob( mob );

       send_to_char( "Mob deleted.\r\n", ch );
       return;
}

void do_rareset(CHAR_DATA* ch, const char* argument)
{
    int vnum;
    int rare;
    char svnum[MAX_INPUT_LENGTH];
    char srare[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA* pObj;
    AREA_DATA* pArea;

    if ( !argument || !argument[0] ) {
        send_to_char("Usage: rareset <vnum> <amt>\r\n", ch);
        return;
    }

    argument = one_argument(argument, svnum);
    vnum = dotted_to_vnum(ch->GetInRoom()->vnum, svnum);
    pObj = get_obj_index(vnum);

    if ( !vnum ) {
        ch_printf(ch, "Invalid vnum: %s\r\n", svnum);
        return;
    }

    if ( !argument || !argument[0] ) {
        ch_printf(ch, "Usage: rareset %s <amt>\r\n", svnum);
        return;
    }

    argument = one_argument(argument, srare);
    rare = atoi(srare);

    if ( rare < 0 ) rare = 0;

    pObj->rare = rare;

    ch_printf(ch, "Rare value for vnum %s (%s) set to %d\n", vnum_to_dotted(vnum), pObj->name_.c_str(), rare);

    for ( pArea = first_area; pArea; pArea = pArea->next ) {
        if ( vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum ) {
            fold_area(pArea, pArea->filename, FALSE);
            return;
        }
    }
    for ( pArea = first_build; pArea; pArea = pArea->next ) {
        if ( vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum ) {
            sprintf(srare, "%s%s", BUILD_DIR, pArea->filename);
            fold_area(pArea, srare, FALSE);
        }
    }
}

void do_renumber(CHAR_DATA* ch, const char* argument) {
    char arg[MAX_INPUT_LENGTH];
    int  ostart, oend;
    int  nstart;
    int  diff;
    int  idx;
    AREA_DATA*       parea;
    MOB_INDEX_DATA*  vmidx;
    CHAR_DATA*       vch;
    OBJ_DATA*        vobj;
    OBJ_INDEX_DATA*  voidx;
    ROOM_INDEX_DATA* vridx;
    RESET_DATA*      preset;
    SHOP_DATA*       pshop;
    STABLE_DATA*     pstable;
    REPAIR_DATA*     prepair;
    TRAIN_DATA*      ptrain;
    BOARD_DATA*      pboard;
    extern MOB_INDEX_DATA*  mob_index_hash [MAX_KEY_HASH];
    extern ROOM_INDEX_DATA* room_index_hash [MAX_KEY_HASH];
    extern OBJ_INDEX_DATA*  obj_index_hash [MAX_KEY_HASH];

    if ( !argument || argument[0] == '\0' ) {
        ch_printf(ch, "Usage: renumber <areaname> <newstartvnum>\r\n");
        return;
    }

    argument = one_argument(argument, arg);

    for ( parea = first_area; parea; parea = parea->next ) {
        if ( !str_cmp(parea->filename, arg) ) {
            send_to_char("Found area.\r\n", ch);
            break;
        }
    }

    if ( !parea ) {
        for ( parea = first_build; parea; parea = parea->next ) {
            if ( !str_cmp(parea->filename, arg) ) {
                send_to_char("Found area (building).\r\n", ch);
                break;
            }
        }
    }

    if ( !parea ) {
        send_to_char("No such area seems to exist.\r\n", ch);
        return;
    }

    ostart = UMIN( UMIN(parea->low_o_vnum, parea->low_r_vnum), parea->low_m_vnum );
    oend   = UMAX( UMAX(parea->hi_o_vnum,  parea->hi_r_vnum),  parea->hi_m_vnum  );

    ch_printf(ch, "Range: %s - ", vnum_to_dotted(ostart));
    ch_printf(ch, "%s\r\n",       vnum_to_dotted(oend  ));

    if ( argument[0] == '\0' ) {
        ch_printf(ch, "Usage: renumber %s <new vnum start>\r\n", parea->filename);
        return;
    }

    nstart = dotted_to_vnum(0, argument);

    ch_printf(ch, "Starting at %s.\r\n", vnum_to_dotted(nstart));


    ch_printf(ch, "Purging active mobiles.");
    for ( vch = first_char; vch; vch = vch->next ) {
        if ( IS_NPC(vch) && vch->pIndexData->vnum != 3 )
        {
            extract_char(vch, TRUE);
        }
    }

    ch_printf(ch, "Purging active objects.");
    for ( vobj = last_object; vobj; vobj = vobj->prev ) {
        if ( !obj_extracted(vobj) ) {
            extract_obj(vobj, TRUE);
        }
    }

    parea->low_o_vnum = parea->low_m_vnum = parea->low_r_vnum = nstart;
    parea->hi_o_vnum  = parea->hi_m_vnum  = parea->hi_r_vnum  = oend - ostart + nstart;
    ch_printf(ch, "Setting new area ranges to %s", vnum_to_dotted(parea->low_o_vnum));
    ch_printf(ch, " - %s.\r\n", vnum_to_dotted(parea->hi_m_vnum));

    ch_printf(ch, "Beginning search of mobile indices.\r\n");

    for ( idx = 0; idx < MAX_KEY_HASH; idx++ ) {
        for ( vmidx = mob_index_hash[idx]; vmidx; vmidx = vmidx->next ) {
            if ( vmidx->vnum >= ostart && vmidx->vnum <= oend ) {
                diff = vmidx->vnum - ostart;
                vmidx->vnum = diff + nstart;

                if ( vmidx->vnum % MAX_KEY_HASH != idx ) {
                    MOB_INDEX_DATA* prev;

                    if ( vmidx == mob_index_hash[idx] ) {
                        mob_index_hash[idx] = vmidx->next;
                    } else {
                        for ( prev = mob_index_hash[idx]; prev; prev = prev->next ) {
                            if ( (prev->next == vmidx) ) {
                                break;
                            }
                        }

                        prev->next = vmidx->next;
                    }
                    vmidx->next = mob_index_hash[vmidx->vnum % MAX_KEY_HASH];
                    mob_index_hash[vmidx->vnum % MAX_KEY_HASH] = vmidx;
                }
            }
        }
    }


    ch_printf(ch, "Beginning search of room indices.\r\n");

    for ( idx = 0; idx < MAX_KEY_HASH; idx++ ) {
        for ( vridx = room_index_hash[idx]; vridx; vridx = vridx->next ) {
            ExitData* pexit;

            if ( vridx->vnum >= ostart && vridx->vnum <= oend ) {
                diff = vridx->vnum - ostart;
                vridx->vnum = diff + nstart;

                if ( (vridx->vnum % MAX_KEY_HASH) != idx ) {
                    ROOM_INDEX_DATA* prev;

                    if ( vridx == room_index_hash[idx] ) {
                        room_index_hash[idx] = vridx->next;
                    } else {
                        for ( prev = room_index_hash[idx]; prev; prev = prev->next ) {
                            if ( prev->next == vridx ) {
                                break;
                            }
                        }

                        if ( prev ) {
                            prev->next = vridx->next;
                        }
                    }

                    vridx->next = room_index_hash[vridx->vnum % MAX_KEY_HASH];
                    room_index_hash[vridx->vnum % MAX_KEY_HASH] = vridx;
                }
            }

            if ( vridx->tele_vnum >= ostart && vridx->tele_vnum <= oend ) {
                vridx->tele_vnum = vridx->tele_vnum - ostart + nstart;
            }

            for ( pexit = vridx->first_exit; pexit; pexit = pexit->next ) {
                if ( pexit->vnum >= ostart && pexit->vnum <= oend )
                    pexit->vnum = pexit->vnum - ostart + nstart;
                if ( pexit->rvnum >= ostart && pexit->vnum <= oend )
                    pexit->rvnum = pexit->vnum - ostart + nstart;
                if ( pexit->key >= ostart && pexit->vnum <= oend )
                    pexit->key = pexit->vnum - ostart + nstart;
            }
        }
    }


    ch_printf(ch, "Beginning search of object indixes.\r\n");

    for ( idx = 0; idx < MAX_KEY_HASH; idx++ ) {
        for ( voidx = obj_index_hash[idx]; voidx; voidx = voidx->next ) {
            if ( voidx->vnum >= ostart && voidx->vnum <= oend ) {
                diff = voidx->vnum - ostart;
                voidx->vnum = diff + nstart;

                if ( voidx->vnum % MAX_KEY_HASH != idx ) {
                    OBJ_INDEX_DATA* prev;

                    if ( voidx == obj_index_hash[idx] ) {
                        obj_index_hash[idx] = voidx->next;
                    } else {
                        for ( prev = obj_index_hash[idx]; prev; prev = prev->next ) {
                            if ( (prev->next == voidx) ) {
                                break;
                            }
                        }

                        prev->next = voidx->next;
                    }

                    voidx->next = obj_index_hash[voidx->vnum % MAX_KEY_HASH];
                    obj_index_hash[voidx->vnum % MAX_KEY_HASH] = voidx;
                }
            }

            switch ( voidx->item_type ) {
                case ITEM_CONTAINER:
                    if ( voidx->value[2] >= ostart && voidx->value[2] <= oend ) {
                        voidx->value[2] = voidx->value[2] - ostart + nstart;
                    }
                    break;
            }
        }
    }


    ch_printf(ch, "Searching through all resets.\r\n");

    for ( parea = first_area; parea; parea = parea->next ) {
        for ( preset = parea->first_reset; preset; preset = preset->next ) {
            switch ( preset->command ) {
                case 'M':
                case 'O':
                case 'P':
                case 'T':
                    if ( preset->arg1 >= ostart && preset->arg1 <= oend )
                        preset->arg1 = preset->arg1 - ostart + nstart;
                    if ( preset->arg3 >= ostart && preset->arg3 <= oend )
                        preset->arg3 = preset->arg3 - ostart + nstart;
                    break;
                case 'G':
                case 'E':
                case 'H':
                case 'B':
                case 'D':
                    if ( preset->arg1 >= ostart && preset->arg1 <= oend )
                        preset->arg1 = preset->arg1 - ostart + nstart;
            }
        }
    }

    for ( parea = first_build; parea; parea = parea->next ) {
        for ( preset = parea->first_reset; preset; preset = preset->next ) {
            switch ( preset->command ) {
                case 'M':
                case 'O':
                case 'P':
                case 'T':
                    if ( preset->arg1 >= ostart && preset->arg1 <= oend )
                        preset->arg1 = preset->arg1 - ostart + nstart;
                    if ( preset->arg3 >= ostart && preset->arg3 <= oend )
                        preset->arg3 = preset->arg3 - ostart + nstart;
                    break;
                case 'G':
                case 'E':
                case 'H':
                case 'B':
                case 'D':
                    if ( preset->arg1 >= ostart && preset->arg1 <= oend )
                        preset->arg1 = preset->arg1 - ostart + nstart;
            }
        }
    }

    ch_printf(ch, "Searching through shops.\r\n");

    for ( pshop = first_shop; pshop; pshop = pshop->next ) {
        if ( pshop->keeper >= ostart && pshop->keeper <= oend ) {
            pshop->keeper = pshop->keeper - ostart + nstart;
        }
    }

    ch_printf(ch, "Searching through stables.\r\n");

    for ( pstable = first_stable; pstable; pstable = pstable->next ) {
        if ( pstable->keeper >= ostart && pstable->keeper <= oend ) {
            pstable->keeper = pstable->keeper - ostart + nstart;
        }
    }

    ch_printf(ch, "Searching through repair shops.\r\n");

    for ( prepair = first_repair; prepair; prepair = prepair->next ) {
        if ( prepair->keeper >= ostart && prepair->keeper <= oend ) {
            prepair->keeper = prepair->keeper - ostart + nstart;
        }
    }

    ch_printf(ch, "Searching through trainers.\r\n");

    for ( ptrain = first_train; ptrain; ptrain = ptrain->next ) {
        if ( ptrain->vnum >= ostart && ptrain->vnum <= oend ) {
            ptrain->vnum = ptrain->vnum - ostart + nstart;
        }
    }

    ch_printf(ch, "Seearching boards.\r\n");
    for ( pboard = first_board; pboard; pboard = pboard->next ) {
        if ( pboard->board_obj >= ostart && pboard->board_obj <= oend ) {
            pboard->board_obj = pboard->board_obj - ostart + nstart;
        }
    }

    ch_printf(ch, "Writing board data.\r\n");
    write_boards_txt();

    ch_printf(ch, "Folding all areas.\r\n");
    for ( parea = first_area; parea; parea = parea->next ) {
        fold_area(parea, parea->filename, FALSE);
    }

    for ( parea = first_build; parea; parea = parea->next ) {
        sprintf(log_buf, "%s%s", BUILD_DIR, parea->filename );
        fold_area(parea, log_buf, FALSE);
    }

    ch_printf(ch, "Done.\r\r\n\n");

    sprintf(log_buf, "Area %s moved from %s - ", arg, vnum_to_dotted(ostart));
    sprintf(log_buf, "%s%s to ",  log_buf, vnum_to_dotted(oend));
    sprintf(log_buf, "%s%s - ",   log_buf, vnum_to_dotted(nstart));
    sprintf(log_buf, "%s%s.\r\n", log_buf, vnum_to_dotted(oend - ostart + nstart));
    log_string(log_buf);
}


/* ---------- playerbuilding ---- */

int is_playbuilding(CHAR_DATA *ch)
{
	int sn=skill_lookup("playbuild");
	int result = is_affected(ch, sn);

	return(result);
}

void do_playbuild(CHAR_DATA *ch, const char* arg)
{
	OBJ_DATA *obj;
	AFFECT_DATA af,*paf,*paf_next;
	int sn = skill_lookup("playbuild");

  (void)arg;

  if(IS_NPC(ch) || !ch->pcdata->area)
  {
    ch_printf(ch,"Huh?\n");
    return;
  }
  if(is_playbuilding(ch))
  {
    ch_printf(ch,"You already think you are a builder!\n");
    return;
  }

  if(!playerbuilderallowedgoto(ch))
    return;

  /* do all the work of quitting... */
  act(AT_BYE,"$n begins to babble senselessly and wanders off.\r\n",ch,NULL,NULL,TO_ROOM);
  act(AT_BYE,"You begin to suffer delusions of immortality...\r\n",ch,NULL,NULL,TO_CHAR);

  quitting_char=ch;
  save_char_obj( ch );
  if( ch->GetMount() )
    extract_char(ch->GetMount(), TRUE);
  saving_char=NULL;
  quitting_char=NULL;

  /* strip all eq from char */
  while ( (obj = ch->last_carrying) != NULL )
  {
    /*  we don't want to decrement total_count! */
    extract_obj( obj, FALSE );
  }

  char_from_room(ch);

  char_to_room(ch,get_room_index(ch->pcdata->area->low_r_vnum));
  ch->gold=0; /* NO GOLD so player cannot create piles of it */


  /* strip all affects */
  for ( paf = ch->first_affect; paf; paf = paf_next )
  {
    paf_next = paf->next;
    affect_remove( ch, paf );
  }

  af.type      = sn;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.duration  = -1; /* forever */
  af.bitvector = 0;
  affect_to_char( ch, &af );

	ch_printf(ch, "Your sn is: %d.", sn);
}


