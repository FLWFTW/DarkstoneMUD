

/***************************************************************\
*                                                               *
*                           KSILYAN                             *
*                                                               *
*****************************************************************
*                                                               *
* This code is used to translate all numbers into text, and all *
* text into numbers. Useful for building, mostly...				*
*                                                               *
\***************************************************************/

#define __CPP // I am c++ code

#include "mud.h"
#include "string.h"

const char * exit_number_to_name (int exitflags)
{
	static char buf[MAX_INPUT_LENGTH];

	buf[0] = '\0';

	if (IS_SET(exitflags, EX_ISDOOR) )				strcat(buf, "isdoor ");
	if (IS_SET(exitflags, EX_CLOSED) )				strcat(buf, "closed ");
	if (IS_SET(exitflags, EX_LOCKED) )				strcat(buf, "locked ");
	if (IS_SET(exitflags, EX_SECRET) )              strcat(buf, "secret ");
	if (IS_SET(exitflags, EX_SWIM) )                strcat(buf, "swim ");
	if (IS_SET(exitflags, EX_PICKPROOF) )			strcat(buf, "pickproof ");
	if (IS_SET(exitflags, EX_FLY) )                 strcat(buf, "fly ");
	if (IS_SET(exitflags, EX_CLIMB) )               strcat(buf, "climb ");
	if (IS_SET(exitflags, EX_DIG) )                 strcat(buf, "dig ");
	//if (IS_SET(exitflags, EX_RES1) )                strcat(buf, "reserved1 ");
	if (IS_SET(exitflags, EX_NOMOBWANDER) )			strcat(buf, "nomobwander ");
	if (IS_SET(exitflags, EX_NOPASSDOOR) )          strcat(buf, "nopassdoor ");
	if (IS_SET(exitflags, EX_HIDDEN) )              strcat(buf, "hidden ");
	if (IS_SET(exitflags, EX_PASSAGE) )             strcat(buf, "passage ");
	if (IS_SET(exitflags, EX_PORTAL) )              strcat(buf, "portal ");
	if (IS_SET(exitflags, EX_BARRICADE) )           strcat(buf, "barricade ");
	if (IS_SET(exitflags, EX_RES3) )                strcat(buf, "reserved3 ");
	if (IS_SET(exitflags, EX_xCLIMB) )              strcat(buf, "can_climb ");
	if (IS_SET(exitflags, EX_xENTER) )              strcat(buf, "can_enter ");
	if (IS_SET(exitflags, EX_xLEAVE) )              strcat(buf, "can_leave ");
	if (IS_SET(exitflags, EX_xAUTO) )               strcat(buf, "auto ");
	if (IS_SET(exitflags, EX_RES4) )                strcat(buf, "reserved4 ");
	if (IS_SET(exitflags, EX_xSEARCHABLE) )         strcat(buf, "searchable ");
	if (IS_SET(exitflags, EX_BASHED) )              strcat(buf, "bashed ");
	if (IS_SET(exitflags, EX_BASHPROOF) )           strcat(buf, "bashproof ");
	if (IS_SET(exitflags, EX_NOMOB) )               strcat(buf, "nomob ");
	if (IS_SET(exitflags, EX_WINDOW) )              strcat(buf, "window ");
	if (IS_SET(exitflags, EX_xLOOK) )               strcat(buf, "can_look ");

	if (buf[0] == '\0')								strcat(buf, "none");
	return buf;
}

int exit_name_to_number (char * exittext)
{
	if (!strcmp(exittext, "isdoor") )			return EX_ISDOOR;
	else if (!strcmp(exittext, "closed") )		return EX_CLOSED;
	else if (!strcmp(exittext, "locked") )		return EX_LOCKED;
	else if (!strcmp(exittext, "secret") )		return EX_SECRET;
	else if (!strcmp(exittext, "swim") )		return EX_SWIM;
	else if (!strcmp(exittext, "pickproof") )	return EX_PICKPROOF;
	else if (!strcmp(exittext, "fly") )			return EX_FLY;
	else if (!strcmp(exittext, "climb") )		return EX_CLIMB;
	else if (!strcmp(exittext, "dig") )			return EX_DIG;
	//else if (!strcmp(exittext, "reserved1") )	return EX_RES1;
	else if (!strcmp(exittext, "nomobwander") ) return EX_NOMOBWANDER;
	else if (!strcmp(exittext, "nopassdoor") )	return EX_NOPASSDOOR;
	else if (!strcmp(exittext, "hidden") )		return EX_HIDDEN;
	else if (!strcmp(exittext, "passage") )		return EX_PASSAGE;
	else if (!strcmp(exittext, "portal") )		return EX_PORTAL;
	else if (!strcmp(exittext, "reserved2") )	return EX_BARRICADE;
	else if (!strcmp(exittext, "reserved3") )	return EX_RES3;
	else if (!strcmp(exittext, "can_climb") )	return EX_xCLIMB;
	else if (!strcmp(exittext, "can_enter") )	return EX_xENTER;
	else if (!strcmp(exittext, "can_leave") )	return EX_xLEAVE;
	else if (!strcmp(exittext, "auto") )		return EX_xAUTO;
	else if (!strcmp(exittext, "reserved4") )	return EX_RES4;
	else if (!strcmp(exittext, "searchable") )	return EX_xSEARCHABLE;
	else if (!strcmp(exittext, "bashed") )		return EX_BASHED;
	else if (!strcmp(exittext, "bashproof") )	return EX_BASHPROOF;
	else if (!strcmp(exittext, "nomob") )		return EX_NOMOB;
	else if (!strcmp(exittext, "window") )		return EX_WINDOW;
	else if (!strcmp(exittext, "can_look") )	return EX_xLOOK;
	else {
		char buf[MAX_INPUT_LENGTH];
		sprintf (buf, "Unknown exit text: %s", exittext);
		bug(buf);
		return 0;
	}
}



const char * sector_number_to_name (int sectortype)
{
	switch(sectortype)
	{
		default:
			return "invalid sector";
			break;
		case SECT_INSIDE:
			return "inside";
			break;
		case SECT_CITY:
			return "city";
			break;
		case SECT_FIELD:
			return "field";
			break;
		case SECT_FOREST:
			return "forest";
			break;
		case SECT_HILLS:
			return "hills";
			break;
		case SECT_MOUNTAIN:
			return "mountain";
			break;
		case SECT_WATER_SWIM:
			return "swimmable water";
			break;
		case SECT_WATER_NOSWIM:
			return "unswimmable water";
			break;
		case SECT_UNDERWATER:
			return "underwater";
			break;
		case SECT_AIR:
			return "air";
			break;
		case SECT_DESERT:
			return "desert";
			break;
		case SECT_DUNNO:
			return "unknown";
			break;
		case SECT_OCEANFLOOR:
			return "oceanfloor";
			break;
		case SECT_UNDERGROUND:
			return "underground";
			break;
		case SECT_ROADS:
			return "roads";
			break;
	}
}

int sector_name_to_number (const char * sectorname)
{
	if (!str_prefix(sectorname, "inside"))				return SECT_INSIDE;
	else if (!str_prefix(sectorname, "city"))			return SECT_CITY;
	else if (!str_prefix(sectorname, "field"))			return SECT_FIELD;
	else if (!str_prefix(sectorname, "forest"))			return SECT_FOREST;
	else if (!str_prefix(sectorname, "hills"))			return SECT_HILLS;
	else if (!str_prefix(sectorname, "mountain"))		return SECT_MOUNTAIN;
	else if (!str_prefix(sectorname, "waterswim"))		return SECT_WATER_SWIM;
	else if (!str_prefix(sectorname, "waternoswim"))	return SECT_WATER_NOSWIM;
	else if (!str_prefix(sectorname, "underwater"))		return SECT_UNDERWATER;
	else if (!str_prefix(sectorname, "air"))			return SECT_AIR;
	else if (!str_prefix(sectorname, "desert"))			return SECT_DESERT;
	else if (!str_prefix(sectorname, "unknown"))		return SECT_DUNNO;
	else if (!str_prefix(sectorname, "oceanfloor"))		return SECT_OCEANFLOOR;
	else if (!str_prefix(sectorname, "underground"))	return SECT_UNDERGROUND;
	else if (!str_prefix(sectorname, "roads"))			return SECT_ROADS;
	else											return 0;
}


const char * ammotype_number_to_name(int ammotype)
{
    switch(ammotype)
	{
		default:
		case AMMO_NONE:
			return "none";
			break;
		case AMMO_LONGARROW:
			return "long arrow";
			break;
		case AMMO_SHORTARROW:
			return "short arrow";
			break;
		case AMMO_BOLT:
			return "bolt";
			break;
		case AMMO_STONE:
			return "stone";
			break;
	}
}

int ammotype_name_to_number(const char * ammoname)
{
	if		(!strcmp(ammoname, "long arrow"))	return AMMO_LONGARROW;
	else if (!strcmp(ammoname, "short arrow"))	return AMMO_SHORTARROW;
	else if (!strcmp(ammoname, "bolt"))			return AMMO_BOLT;
	else if (!strcmp(ammoname, "stone"))		return AMMO_STONE;
	else										return AMMO_NONE;
}

const char * weapontype_number_to_name(int weaponnumber)
{
	switch(weaponnumber)
	{
		default:
			return "Unknown";
			break;
		case WEAPON_NONE:
			return "NotWeapon...";
			break;
		case WEAPON_PUGILISM:
			return "pugilism";
			break;
		case WEAPON_LONGBLADE:
			return "long blade";
			break;
		case WEAPON_SHORTBLADE:
			return "short blade";
			break;
		case WEAPON_FLEXIBLE:
			return "flexible";
			break;
		case WEAPON_BLUDGEON:
			return "bludgeon";
			break;
		case WEAPON_POLEARM:
			return "polearm";
			break;
		case WEAPON_LONGBOW:
			return "long bow";
			break;
		case WEAPON_SHORTBOW:
			return "short bow";
			break;
		case WEAPON_CROSSBOW:
			return "crossbow";
			break;
		case WEAPON_SLING:
			return "sling";
			break;
		case WEAPON_THROWINGSPEAR:
			return "throwing spear";
			break;
		case WEAPON_THROWINGKNIFE:
			return "throwing knife";
			break;
	}
}

int weapontype_to_ammotype(int weapontype)
{
	switch(weapontype)
	{
		default:
			return AMMO_NONE;
			break;
		case WEAPON_LONGBOW:
			return AMMO_LONGARROW;
			break;
		case WEAPON_SHORTBOW:
			return AMMO_SHORTARROW;
			break;
		case WEAPON_CROSSBOW:
			return AMMO_BOLT;
			break;
		case WEAPON_SLING:
			return AMMO_STONE;
			break;
	}
}

int weapontype_to_damagemsg (int weapontype)
{
	/*
	 * A function used to get a damage message for a weapon that doesn't happen
	 * to have one set.
	*/
	switch (weapontype)
	{
		default:				return DAMAGE_MSG_DEFAULT;
		case WEAPON_LONGBLADE:	return DAMAGE_MSG_SWING;
		case WEAPON_SHORTBLADE:	return DAMAGE_MSG_STAB;
		case WEAPON_FLEXIBLE:	return DAMAGE_MSG_WHIP;
		case WEAPON_BLUDGEON:	return DAMAGE_MSG_POUND;
		case WEAPON_POLEARM:	return DAMAGE_MSG_STAB;
		case WEAPON_THROWINGSPEAR:	return DAMAGE_MSG_SPEAR;
		case WEAPON_THROWINGKNIFE:	return DAMAGE_MSG_KNIFE;
	}
}

int weapontype_to_damagetype(int weapontype)
{
    switch(weapontype)
	{
		default:
			return DAMAGE_DEFAULT;
			break;
		case WEAPON_NONE:
			return DAMAGE_DEFAULT;
			break;
		case WEAPON_PUGILISM:
			return DAMAGE_BLUNT;
			break;
		case WEAPON_LONGBLADE:
			return DAMAGE_SLASH;
			break;
		case WEAPON_SHORTBLADE:
		case WEAPON_POLEARM:
		case WEAPON_THROWINGSPEAR:
		case WEAPON_THROWINGKNIFE:
			return DAMAGE_PIERCE;
			break;
		case WEAPON_FLEXIBLE:
			return DAMAGE_BLUNT;
			break;
		case WEAPON_BLUDGEON:
			return DAMAGE_BLUNT;
			break;
			return DAMAGE_PIERCE;
			break;
		case WEAPON_LONGBOW:
		case WEAPON_SHORTBOW:
		case WEAPON_CROSSBOW:
		case WEAPON_SLING:
			return DAMAGE_DEFAULT;
	}
}

int ammotype_to_weapontype (int ammotype)
{
	switch(ammotype)
	{
		default:
			return WEAPON_NONE;
			break;
		case AMMO_LONGARROW:
			return WEAPON_LONGBOW;
			break;
		case AMMO_SHORTARROW:
			return WEAPON_SHORTBOW;
			break;
		case AMMO_BOLT:
			return WEAPON_CROSSBOW;
			break;
		case AMMO_STONE:
			return WEAPON_SLING;
			break;
	}
}

int ammotype_to_damagetype (int ammotype)
{
	switch(ammotype)
	{
		default:
			return DAMAGE_DEFAULT;
		case AMMO_LONGARROW:
		case AMMO_SHORTARROW:
		case AMMO_BOLT:
			return DAMAGE_PIERCE;
		case AMMO_STONE:
			return DAMAGE_BLUNT;
	}
}

int ammotype_to_damagemessage (int ammotype)
{
	switch(ammotype)
	{
		default:
			return DAMAGE_MSG_DEFAULT;
		case AMMO_LONGARROW:
		case AMMO_SHORTARROW:
			return DAMAGE_MSG_ARROW;
		case AMMO_BOLT:
			return DAMAGE_MSG_BOLT;
		case AMMO_STONE:
			return DAMAGE_MSG_STONE;
	}
}

int weapontype_name_to_number(const char * weaponname)
{
	if 		(!strcmp(weaponname, "pugilism"))		return WEAPON_PUGILISM;
	else if (!strcmp(weaponname, "long blade"))  	return WEAPON_LONGBLADE;
	else if (!strcmp(weaponname, "short blade"))  	return WEAPON_SHORTBLADE;
	else if (!strcmp(weaponname, "flexible")) 		return WEAPON_FLEXIBLE;
	else if (!strcmp(weaponname, "bludgeon")) 		return WEAPON_BLUDGEON;
	else if (!strcmp(weaponname, "polearm")) 		return WEAPON_POLEARM;
	else if (!strcmp(weaponname, "long bow")) 		return WEAPON_LONGBOW;
	else if (!strcmp(weaponname, "short bow")) 		return WEAPON_SHORTBOW;
	else if (!strcmp(weaponname, "crossbow")) 		return WEAPON_CROSSBOW;
	else if (!strcmp(weaponname, "sling")) 			return WEAPON_SLING;
	else if (!strcmp(weaponname, "throwing spear"))	return WEAPON_THROWINGSPEAR;
	else if (!strcmp(weaponname, "throwing knife"))	return WEAPON_THROWINGKNIFE;
	else											return WEAPON_NONE;
}

int weapontype_name_to_number_nospaces(const char * weaponname)
{
	if 		(!strcmp(weaponname, "pugilism"))		return WEAPON_PUGILISM;
	else if (!strcmp(weaponname, "longblade"))  	return WEAPON_LONGBLADE;
	else if (!strcmp(weaponname, "shortblade"))  	return WEAPON_SHORTBLADE;
	else if (!strcmp(weaponname, "flexible")) 		return WEAPON_FLEXIBLE;
	else if (!strcmp(weaponname, "bludgeon")) 		return WEAPON_BLUDGEON;
	else if (!strcmp(weaponname, "polearm")) 		return WEAPON_POLEARM;
	else if (!strcmp(weaponname, "longbow")) 		return WEAPON_LONGBOW;
	else if (!strcmp(weaponname, "shortbow")) 		return WEAPON_SHORTBOW;
	else if (!strcmp(weaponname, "crossbow")) 		return WEAPON_CROSSBOW;
	else if (!strcmp(weaponname, "sling")) 			return WEAPON_SLING;
	else if (!strcmp(weaponname, "throwingspear"))	return WEAPON_THROWINGSPEAR;
	else if (!strcmp(weaponname, "throwingknife"))	return WEAPON_THROWINGKNIFE;
	else											return WEAPON_NONE;
}


const char * ris_number_to_name (int risflags)
{
	static char buf[MAX_INPUT_LENGTH];

	buf[0] = '\0';
	if (IS_SET(risflags, RIS_FIRE) )               strcat(buf, "fire ");
	if (IS_SET(risflags, RIS_COLD) )               strcat(buf, "cold ");
	if (IS_SET(risflags, RIS_ELECTRICITY) )        strcat(buf, "electricity ");
	if (IS_SET(risflags, RIS_ENERGY) )             strcat(buf, "energy ");
	if (IS_SET(risflags, RIS_BLUNT) )              strcat(buf, "blunt ");
	if (IS_SET(risflags, RIS_PIERCE) )             strcat(buf, "pierce ");
	if (IS_SET(risflags, RIS_SLASH) )              strcat(buf, "slash ");
	if (IS_SET(risflags, RIS_ACID) )               strcat(buf, "acid ");
	if (IS_SET(risflags, RIS_POISON) )             strcat(buf, "poison ");
	if (IS_SET(risflags, RIS_DRAIN) )              strcat(buf, "drain ");
	if (IS_SET(risflags, RIS_SLEEP) )              strcat(buf, "sleep ");
	if (IS_SET(risflags, RIS_CHARM) )              strcat(buf, "charm ");
	if (IS_SET(risflags, RIS_HOLD) )               strcat(buf, "hold ");
	if (IS_SET(risflags, RIS_NONMAGIC) )           strcat(buf, "nonmagic ");
	if (IS_SET(risflags, RIS_PLUS1) )              strcat(buf, "plus1 ");
	if (IS_SET(risflags, RIS_PLUS2) )              strcat(buf, "plus2 ");
	if (IS_SET(risflags, RIS_PLUS3) )              strcat(buf, "plus3 ");
	if (IS_SET(risflags, RIS_PLUS4) )              strcat(buf, "plus4 ");
	if (IS_SET(risflags, RIS_PLUS5) )              strcat(buf, "plus5 ");
	if (IS_SET(risflags, RIS_PLUS6) )              strcat(buf, "plus6 ");
	if (IS_SET(risflags, RIS_MAGIC) )              strcat(buf, "magic ");
	if (IS_SET(risflags, RIS_PARALYSIS) )          strcat(buf, "paralysis ");

	if (buf[0] == '\0')                            strcat(buf, "none");
	return buf;
}


int ris_name_to_number (const char * ristext)
{
	if (!strcmp(ristext, "fire") )              return RIS_FIRE;
	else if (!strcmp(ristext, "cold") )         return RIS_COLD;
	else if (!strcmp(ristext, "electricity") )  return RIS_ELECTRICITY;
	else if (!strcmp(ristext, "energy") )       return RIS_ENERGY;
	else if (!strcmp(ristext, "blunt") )        return RIS_BLUNT;
	else if (!strcmp(ristext, "pierce") )       return RIS_PIERCE;
	else if (!strcmp(ristext, "slash") )        return RIS_SLASH;
	else if (!strcmp(ristext, "acid") )         return RIS_ACID;
	else if (!strcmp(ristext, "poison") )       return RIS_POISON;
	else if (!strcmp(ristext, "drain") )        return RIS_DRAIN;
	else if (!strcmp(ristext, "sleep") )        return RIS_SLEEP;
	else if (!strcmp(ristext, "charm") )        return RIS_CHARM;
	else if (!strcmp(ristext, "hold") )         return RIS_HOLD;
	else if (!strcmp(ristext, "nonmagic") )     return RIS_NONMAGIC;
	else if (!strcmp(ristext, "plus1") )        return RIS_PLUS1;
	else if (!strcmp(ristext, "plus2") )        return RIS_PLUS2;
	else if (!strcmp(ristext, "plus3") )        return RIS_PLUS3;
	else if (!strcmp(ristext, "plus4") )        return RIS_PLUS4;
	else if (!strcmp(ristext, "plus5") )        return RIS_PLUS5;
	else if (!strcmp(ristext, "plus6") )        return RIS_PLUS6;
	else if (!strcmp(ristext, "magic") )        return RIS_MAGIC;
	else if (!strcmp(ristext, "paralysis") )    return RIS_PARALYSIS;
	else
	{
		char buf[MAX_INPUT_LENGTH];
		sprintf (buf, "Unknown RIS text: %s", ristext);
		bug(buf);
		return 0;
	}
}




const char * itemtype_number_to_name (int itemtype)
{
	switch(itemtype)
	{
		default: 				return "unknown";
		case ITEM_NONE: 		return "none";
		case ITEM_LIGHT: 		return "light";
		case ITEM_SCROLL: 		return "scroll";
		case ITEM_WAND: 		return "wand";
		case ITEM_STAFF: 		return "staff";
		case ITEM_WEAPON: 		return "weapon";
		case ITEM_TREASURE: 	return "treasure";
		case ITEM_ARMOR: 		return "armor";
		case ITEM_POTION: 		return "potion";
		case ITEM_WORN: 		return "worn";
		case ITEM_FURNITURE:	return "furniture";
		case ITEM_TRASH: 		return "trash";
		case ITEM_OLDTRAP: 		return "oldtrap";
		case ITEM_CONTAINER: 	return "container";
		case ITEM_NOTE: 		return "note";
		case ITEM_DRINK_CON: 	return "drink container";
		case ITEM_KEY: 			return "key";
		case ITEM_FOOD: 		return "food";
		case ITEM_MONEY: 		return "money";
		case ITEM_PEN: 			return "pen";
		case ITEM_BOAT: 		return "boat";
		case ITEM_CORPSE_NPC: 	return "corpse (npc)";
		case ITEM_CORPSE_PC: 	return "corpse (pc)";
		case ITEM_FOUNTAIN: 	return "fountain";
		case ITEM_PILL: 		return "pill";
		case ITEM_BLOOD: 		return "blood";
		case ITEM_BLOODSTAIN: 	return "bloodstain";
		case ITEM_SCRAPS: 		return "scraps";
		case ITEM_PIPE: 		return "pipe";
		case ITEM_HERB_CON: 	return "herb container";
		case ITEM_HERB: 		return "herb";
		case ITEM_INCENSE: 		return "incense";
		case ITEM_FIRE: 		return "fire";
		case ITEM_BOOK: 		return "book";
		case ITEM_SWITCH: 		return "switch";
		case ITEM_LEVER: 		return "lever";
		case ITEM_PULLCHAIN: 	return "pullchain";
		case ITEM_BUTTON: 		return "button";
		case ITEM_DIAL: 		return "dial";
		case ITEM_COMPONENT: 	return "component";
		case ITEM_MATCH: 		return "match";
		case ITEM_TRAP: 		return "trap";
		case ITEM_MAP: 			return "map";
		case ITEM_PORTAL: 		return "portal";
		case ITEM_PAPER: 		return "paper";
		case ITEM_TINDER: 		return "tinder";
		case ITEM_LOCKPICK: 	return "lockpick";
		case ITEM_SPIKE: 		return "spike";
		case ITEM_DISEASE: 		return "disease";
		case ITEM_OIL: 			return "dial";
		case ITEM_FUEL: 		return "fuel";
		case ITEM_PROJECTILE: 	return "projectile";
		case ITEM_QUIVER: 		return "quiver";
		case ITEM_SCABBARD: 	return "scabbard";
		case ITEM_SHOVEL: 		return "shovel";
		case ITEM_SALVE: 		return "salve";
		case ITEM_MEAT: 		return "meat";
		case ITEM_GEM:			return "gem";
		case ITEM_ORE:			return "ore";
	}
}


int itemtype_name_to_number (const char * itemname)
{
	if (!strcmp(itemname, "none")) 				return ITEM_NONE;
	else if (!strcmp(itemname, "light")) 		return ITEM_LIGHT;
	else if (!strcmp(itemname, "scroll")) 		return ITEM_SCROLL;
	else if (!strcmp(itemname, "wand")) 		return ITEM_WAND;
	else if (!strcmp(itemname, "staff")) 		return ITEM_STAFF;
	else if (!strcmp(itemname, "weapon")) 		return ITEM_WEAPON;
	else if (!strcmp(itemname, "treasure")) 	return ITEM_TREASURE;
	else if (!strcmp(itemname, "armor")) 		return ITEM_ARMOR;
	else if (!strcmp(itemname, "potion")) 		return ITEM_POTION;
	else if (!strcmp(itemname, "worn")) 		return ITEM_WORN;
	else if (!strcmp(itemname, "furniture")) 	return ITEM_FURNITURE;
	else if (!strcmp(itemname, "trash")) 		return ITEM_TRASH;
	else if (!strcmp(itemname, "oldtrap")) 		return ITEM_OLDTRAP;
	else if (!strcmp(itemname, "container")) 	return ITEM_CONTAINER;
	else if (!strcmp(itemname, "note")) 		return ITEM_NOTE;
	else if (!strcmp(itemname, "drink_container")) return ITEM_DRINK_CON;
	else if (!strcmp(itemname, "key")) 			return ITEM_KEY;
	else if (!strcmp(itemname, "food")) 		return ITEM_FOOD;
	else if (!strcmp(itemname, "money")) 		return ITEM_MONEY;
	else if (!strcmp(itemname, "pen")) 			return ITEM_PEN;
	else if (!strcmp(itemname, "boat")) 		return ITEM_BOAT;
	else if (!strcmp(itemname, "corpse_npc")) 	return ITEM_CORPSE_NPC;
	else if (!strcmp(itemname, "corpse_pc")) 	return ITEM_CORPSE_PC;
	else if (!strcmp(itemname, "fountain")) 	return ITEM_FOUNTAIN;
	else if (!strcmp(itemname, "pill")) 		return ITEM_PILL;
	else if (!strcmp(itemname, "blood")) 		return ITEM_BLOOD;
	else if (!strcmp(itemname, "bloodstain")) 	return ITEM_BLOODSTAIN;
	else if (!strcmp(itemname, "scraps")) 		return ITEM_SCRAPS;
	else if (!strcmp(itemname, "pipe")) 		return ITEM_PIPE;
	else if (!strcmp(itemname, "herb_container")) return ITEM_HERB_CON;
	else if (!strcmp(itemname, "herb")) 		return ITEM_HERB;
	else if (!strcmp(itemname, "incense")) 		return ITEM_INCENSE;
	else if (!strcmp(itemname, "fire")) 		return ITEM_FIRE;
	else if (!strcmp(itemname, "book")) 		return ITEM_BOOK;
	else if (!strcmp(itemname, "switch")) 		return ITEM_SWITCH;
	else if (!strcmp(itemname, "lever")) 		return ITEM_LEVER;
	else if (!strcmp(itemname, "pullchain")) 	return ITEM_PULLCHAIN;
	else if (!strcmp(itemname, "button")) 		return ITEM_BUTTON;
	else if (!strcmp(itemname, "dial")) 		return ITEM_DIAL;
	else if (!strcmp(itemname, "component")) 	return ITEM_COMPONENT;
	else if (!strcmp(itemname, "match")) 		return ITEM_MATCH;
	else if (!strcmp(itemname, "trap")) 		return ITEM_TRAP;
	else if (!strcmp(itemname, "map")) 			return ITEM_MAP;
	else if (!strcmp(itemname, "portal")) 		return ITEM_PORTAL;
	else if (!strcmp(itemname, "paper")) 		return ITEM_PAPER;
	else if (!strcmp(itemname, "tinder")) 		return ITEM_TINDER;
	else if (!strcmp(itemname, "lockpick")) 	return ITEM_LOCKPICK;
	else if (!strcmp(itemname, "spike")) 		return ITEM_SPIKE;
	else if (!strcmp(itemname, "disease")) 		return ITEM_DISEASE;
	else if (!strcmp(itemname, "dial")) 		return ITEM_DIAL;
	else if (!strcmp(itemname, "fuel")) 		return ITEM_FUEL;
	else if (!strcmp(itemname, "projectile")) 	return ITEM_PROJECTILE;
	else if (!strcmp(itemname, "quiver")) 		return ITEM_QUIVER;
	else if (!strcmp(itemname, "scabbard")) 	return ITEM_SCABBARD;
	else if (!strcmp(itemname, "shovel")) 		return ITEM_SHOVEL;
	else if (!strcmp(itemname, "salve"))		return ITEM_SALVE;
	else if (!strcmp(itemname, "meat")) 		return ITEM_MEAT;
	else if (!strcmp(itemname, "gem"))			return ITEM_GEM;
	else if (!strcmp(itemname, "ore"))			return ITEM_ORE;
	else 										return ITEM_NONE;
}



int itemtype_old_to_new (int old_type)
{
	switch(old_type)
	{
		default: 					return ITEM_NONE;
		case OLD_ITEM_NONE: 		return ITEM_NONE;
		case OLD_ITEM_LIGHT: 		return ITEM_LIGHT;
		case OLD_ITEM_SCROLL: 		return ITEM_SCROLL;
		case OLD_ITEM_WAND: 		return ITEM_WAND;
		case OLD_ITEM_STAFF: 		return ITEM_STAFF;
		case OLD_ITEM_WEAPON:
		case OLD_ITEM_SHORT_BOW:
		case OLD_ITEM_LONG_BOW:
		case OLD_ITEM_CROSSBOW:
		case OLD_ITEM_MISSILE_WEAPON:
		case OLD_ITEM_FIREWEAPON: 	return ITEM_WEAPON;
		case OLD_ITEM_PROJECTILE:
		case OLD_ITEM_MISSILE: 		return ITEM_PROJECTILE;
		case OLD_ITEM_TREASURE: 	return ITEM_TREASURE;
		case OLD_ITEM_ARMOR: 		return ITEM_ARMOR;
		case OLD_ITEM_POTION: 		return ITEM_POTION;
		case OLD_ITEM_WORN: 		return ITEM_WORN;
		case OLD_ITEM_FURNITURE: 	return ITEM_FURNITURE;
		case OLD_ITEM_TRASH: 		return ITEM_TRASH;
		case OLD_ITEM_OLDTRAP: 		return ITEM_OLDTRAP;
		case OLD_ITEM_CONTAINER:	return ITEM_CONTAINER;
		case OLD_ITEM_NOTE: 		return ITEM_NOTE;
		case OLD_ITEM_DRINK_CON: 	return ITEM_DRINK_CON;
		case OLD_ITEM_KEY: 			return ITEM_KEY;
		case OLD_ITEM_FOOD: 		return ITEM_FOOD;
		case OLD_ITEM_MONEY: 		return ITEM_MONEY;
		case OLD_ITEM_PEN: 			return ITEM_PEN;
		case OLD_ITEM_BOAT: 		return ITEM_BOAT;
		case OLD_ITEM_CORPSE_NPC: 	return ITEM_CORPSE_NPC;
		case OLD_ITEM_CORPSE_PC: 	return ITEM_CORPSE_PC;
		case OLD_ITEM_FOUNTAIN: 	return ITEM_FOUNTAIN;
		case OLD_ITEM_PILL: 		return ITEM_PILL;
		case OLD_ITEM_BLOOD: 		return ITEM_BLOOD;
		case OLD_ITEM_BLOODSTAIN: 	return ITEM_BLOODSTAIN;
		case OLD_ITEM_SCRAPS: 		return ITEM_SCRAPS;
		case OLD_ITEM_PIPE: 		return ITEM_PIPE;
		case OLD_ITEM_HERB_CON: 	return ITEM_HERB_CON;
		case OLD_ITEM_HERB: 		return ITEM_HERB;
		case OLD_ITEM_INCENSE: 		return ITEM_INCENSE;
		case OLD_ITEM_FIRE: 		return ITEM_FIRE;
		case OLD_ITEM_BOOK: 		return ITEM_BOOK;
		case OLD_ITEM_SWITCH: 		return ITEM_SWITCH;
		case OLD_ITEM_LEVER: 		return ITEM_LEVER;
		case OLD_ITEM_PULLCHAIN: 	return ITEM_PULLCHAIN;
		case OLD_ITEM_BUTTON: 		return ITEM_BUTTON;
		case OLD_ITEM_DIAL: 		return ITEM_DIAL;
		case OLD_ITEM_RUNE: 		return ITEM_TREASURE;
		case OLD_ITEM_RUNEPOUCH: 	return ITEM_CONTAINER;
		case OLD_ITEM_MATCH: 		return ITEM_MATCH;
		case OLD_ITEM_TRAP: 		return ITEM_TRAP;
		case OLD_ITEM_MAP: 			return ITEM_MAP;
		case OLD_ITEM_PORTAL: 		return ITEM_PORTAL;
		case OLD_ITEM_PAPER: 		return ITEM_PAPER;
		case OLD_ITEM_TINDER: 		return ITEM_TINDER;
		case OLD_ITEM_LOCKPICK: 	return ITEM_LOCKPICK;
		case OLD_ITEM_SPIKE: 		return ITEM_SPIKE;
		case OLD_ITEM_DISEASE: 		return ITEM_DISEASE;
		case OLD_ITEM_OIL: 			return ITEM_OIL;
		case OLD_ITEM_FUEL: 		return ITEM_FUEL;
		case OLD_ITEM_QUIVER: 		return ITEM_QUIVER;
		case OLD_ITEM_SHOVEL: 		return ITEM_SHOVEL;
		case OLD_ITEM_SALVE: 		return ITEM_SALVE;
		case OLD_ITEM_MEAT: 		return ITEM_MEAT;
	}
}


const char * damagemsg_number_to_name (int damagemsg, int damage)
{
	switch (damagemsg)
	{
		default:
			return "unknown";
		case DAMAGE_MSG_DEFAULT:
			return "hit";
		case DAMAGE_MSG_SWING:
			if (damage)
				return "slash";
			else
				return "swing";
		case DAMAGE_MSG_WHIP:
			return "whip";
		case DAMAGE_MSG_STAB:
			if (damage)
				return "stab";
			else
				return "thrust";
		case DAMAGE_MSG_POUND:
			return "pound";
		case DAMAGE_MSG_CRUSH:
			return "crush";
		case DAMAGE_MSG_CLAW:
			if (damage)
				return "clawing";
			else
				return "swipe";
		case DAMAGE_MSG_BITE:
			return "bite";
		case DAMAGE_MSG_SPEAR:
			if (damage)
				return "spear";
			else
				return "jab";
		case DAMAGE_MSG_KNIFE:
			if (damage)
				return "knife";
			else
				return "thrust";
		case DAMAGE_MSG_ARROW:
			return "arrow";
		case DAMAGE_MSG_BOLT:
			return "bolt";
		case DAMAGE_MSG_STONE:
			return "stone";
		case DAMAGE_MSG_FIRE:
			return "fire";
		case DAMAGE_MSG_COLD:
			return "frost";
		case DAMAGE_MSG_ELECTRICITY:
			return "electricity";
		case DAMAGE_MSG_ACID:
			return "acid";
		case DAMAGE_MSG_ENERGY:
			return "blast";
		case DAMAGE_MSG_POLEARM:
			return "polearm";
		case DAMAGE_MSG_KICK:
			return "kick";
		case DAMAGE_MSG_PUNCH:
			if (damage)
				return "punch";
			else
				return "lunge";
		case DAMAGE_MSG_SHIELDBASH:
			return "shieldbash";
	}
}

int damagemsg_name_to_number (const char * damagetext)
{
	if (!strcmp(damagetext, "hit") )           return DAMAGE_MSG_DEFAULT;
	else if (!strcmp(damagetext, "swing") )      return DAMAGE_MSG_SWING;
	else if (!strcmp(damagetext, "stab") )      return DAMAGE_MSG_STAB;
	else if (!strcmp(damagetext, "whip") )      return DAMAGE_MSG_WHIP;
	else if (!strcmp(damagetext, "pound") )        return DAMAGE_MSG_POUND;
	else if (!strcmp(damagetext, "crush") )   return DAMAGE_MSG_CRUSH;
	else if (!strcmp(damagetext, "claw") )         return DAMAGE_MSG_CLAW;
	else if (!strcmp(damagetext, "bite") )       return DAMAGE_MSG_BITE;
	else if (!strcmp(damagetext, "spear") )         return DAMAGE_MSG_SPEAR;
	else if (!strcmp(damagetext, "knife") )   return DAMAGE_MSG_KNIFE;
	else if (!strcmp(damagetext, "arrow") ) return DAMAGE_MSG_ARROW;
	else if (!strcmp(damagetext, "bolt") ) return DAMAGE_MSG_BOLT;
	else if (!strcmp(damagetext, "stone") ) return DAMAGE_MSG_STONE;
	else if (!strcmp(damagetext, "fire") ) return DAMAGE_MSG_FIRE;
	else if (!strcmp(damagetext, "cold") ) return DAMAGE_MSG_COLD;
	else if (!strcmp(damagetext, "electricity") ) return DAMAGE_MSG_ELECTRICITY;
	else if (!strcmp(damagetext, "acid") ) return DAMAGE_MSG_ACID;
	else if (!strcmp(damagetext, "energy") ) return DAMAGE_MSG_ENERGY;
	else if (!strcmp(damagetext, "polearm") ) return DAMAGE_MSG_POLEARM;
	else if (!strcmp(damagetext, "kick") ) return DAMAGE_MSG_KICK;
	else if (!strcmp(damagetext, "punch") ) return DAMAGE_MSG_PUNCH;
	else if (!strcmp(damagetext, "shieldbash") ) return DAMAGE_MSG_SHIELDBASH;
	else return DAMAGE_MSG_DEFAULT;
}


const char * damagetype_number_to_name (int damagenumber)
{
	switch (damagenumber)
	{
		default:					return "unknown";
		case DAMAGE_DEFAULT:		return "default";
		case DAMAGE_SLASH:			return "slash";
		case DAMAGE_PIERCE:			return "pierce";
		case DAMAGE_BLUNT:			return "blunt";
		case DAMAGE_FIRE:			return "fire";
		case DAMAGE_COLD:			return "cold";
		case DAMAGE_ELECTRICITY:	return "electricity";
		case DAMAGE_ACID:			return "acid";
		case DAMAGE_ENERGY:			return "energy";
	}
}

int damagetype_name_to_number (const char * damagetext)
{
	if (!strcmp(damagetext, "default") )  return DAMAGE_DEFAULT;
	else if (!strcmp(damagetext, "slash") )  return DAMAGE_SLASH;
	else if (!strcmp(damagetext, "pierce") )  return DAMAGE_PIERCE;
	else if (!strcmp(damagetext, "blunt") )  return DAMAGE_BLUNT;
	else if (!strcmp(damagetext, "fire") )  return DAMAGE_FIRE;
	else if (!strcmp(damagetext, "cold") )  return DAMAGE_COLD;
	else if (!strcmp(damagetext, "electricity") )  return DAMAGE_ELECTRICITY;
	else if (!strcmp(damagetext, "acid") )  return DAMAGE_ACID;
	else if (!strcmp(damagetext, "energy") )  return DAMAGE_ENERGY;
	else return DAMAGE_DEFAULT;
}


const char * damage_number_to_name (int damage)
{
	if (	damage < 5						)
		return "extremely small amounts of";		/* Really low damage weapons */
	else if (	damage >= 5		&& damage < 14	)
		return "very small amounts of";				/* Newbie weapon range */
	else if (	damage >= 14	&& damage < 18	)
		return "small amounts of";					/* Low weapon range */
	else if (	damage >= 18	&& damage < 20	)
		return "medium amounts of";					/* Medium weapon range */
	else
		return "unknown";
}

const char * power_number_to_name (int power)
{
	return "nice";
}


const char * gem_size_number_to_name (sh_int size)
{
	if (size <= GEM_TINY)
		return "tiny";
	else if (size <= GEM_SMALL)
		return "small";
	else if (size <= GEM_AVERAGE)
		return "average-sized";
	else if (size <= GEM_LARGE)
		return "large";
	else
		return "huge";
}

const char * gem_quality_number_to_name (sh_int quality)
{
	if (quality <= GEM_DEFORMED)
		return "deformed";
	else if (quality <= GEM_FLAWED)
		return "flawed";
	else if (quality <= GEM_FAIR)
		return "fair";
	else if (quality <= GEM_CLEAR)
		return "clear";
	else
		return "perfect";
}

sh_int bodypart_to_wearloc (sh_int bodypart)
{
	switch(bodypart)
	{
		default:
			return WEAR_NONE;

		case BODYPART_HEAD:     return WEAR_HEAD;
		case BODYPART_NECK:     return WEAR_NECK_1;
		case BODYPART_CHEST:    return WEAR_BODY;
		case BODYPART_WAIST:    return WEAR_WAIST;

		case BODYPART_LEFTARM:
		case BODYPART_RIGHTARM: return WEAR_ARMS;

		case BODYPART_LEFTLEG:
		case BODYPART_RIGHTLEG: return WEAR_LEGS;

		case BODYPART_LEFTFOOT:
		case BODYPART_RIGHTFOOT: return WEAR_FEET;

		case BODYPART_LEFTHAND:
		case BODYPART_RIGHTHAND: return WEAR_HANDS;
	}
}
