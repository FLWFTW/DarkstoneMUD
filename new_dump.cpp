/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 * Ever wonder just what the stats on things were in the game? Statting    *
 * one object or mob at a time can be tedious and frequently you have to   *
 * stop and write things down, like hitpoints, armor classes, etc, if you  *
 * are trying to build an area and you want to preserve some continuity.   *
 * Granted, there should probably be tables and such availabe for builders'*
 * use (as there are on Broken Shadows), but you have to base those tables *
 * off something.                                                          *
 *                                                                         *
 * Well... this function is a cross between stat and dump. It loads each   *
 * mob and object briefly and writes its stats (or at least the vital ones)*
 * to a file. I removed a lot of the things from the stat part, mostly     *
 * empty lines where PC data was stored and also a lot of the things that  *
 * returned 0, such as carry weight.                                       *
 *                                                                         *
 * The files are place in the parent directory of the area directory by    *
 * default and are rather large (about 800k each for Shadows). With a      *
 * little work (I wrote a little C++ program to do this), they can be      *
 * converted into a character-delimeted file, so you can import it into    *
 * Access, Excel, or many other popular programs. I could have modified    *
 * This file to write it out in that format, but I was too lazy.           *
 *                                                                         *
 * Oh yeah. There's also a section for rooms. This is straight from rstat  *
 * It hasn't been tweaked at all. The first time I used it, it hit an      *
 * endless loop somewhere in there and I was too lazy to debug it. If you  *
 * want to uncomment it and debug it for me, feel free :)                  *
 *                                                                         *
 * One other thing work noting: Since it does load all the objects and     *
 * mobs in quick succession, CPU and memory usage climbs for about 10-15   *
 * seconds. This might cause a bit of lag for the players. I dunno. I      *
 * haven't used it when players were on.                                   *
 *                                                                         *
 * If you choose to use this code, please retain my name in this file and  *
 * send me an email (dwa1844@rit.edu) saying you are using it. Suggestions *
 * for improvement are welcome                                             *
 ***************************************************************************
 * KSILYAN- I've added a few new Darkstone specific things to this, mostly *
 * basing it off of what was already there. In any case, the csv dump takes*
 * the game information and sticks it into a csv file to be read by the    *
 * Excel chart I've set up. After tweaking item stats, we dump the Excel   *
 * chart into a csv file, and then the game reads it back in. This is only *
 * for weapons and armor.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mud.h"

#include "paths.const.h"

#include "commands.h"

// Forward declarations
void do_say(Character * ch, const char* argument);
void do_ocreate(Character * ch, const char* argument);
void do_purge(Character * ch, const char* argument);

void do_object_backup(CHAR_DATA * ch, const char* argument)
{
    int vnum;
    OBJ_INDEX_DATA * pObjIndex;
    char buf[MAX_INPUT_LENGTH];
    int nMatch;
    
    do_say(ch, "gulp");

    vnum = 0;
    nMatch = 0;

    sprintf(buf, "Object backup with %d objects", top_obj_index);
    log_string(buf);
    
    for (vnum = 0; nMatch < top_obj_index; vnum++)
    {
        sprintf(buf, "Checking vnum %d", vnum);
        log_string(buf);
        if ((pObjIndex = get_obj_index(vnum)) != NULL)
        {
            nMatch++;
            sprintf(buf, "2.%d %d %s", pObjIndex->vnum, pObjIndex->vnum, pObjIndex->name_.c_str());
            do_ocreate(ch, buf);
            dofun_wrapper(do_purge, ch, pObjIndex->name_.c_str());
        }
    }
}


/* KSILYAN
	This is to help with the item fix.
	Dump out all objects so that they fit a comma-separated-value format
	for the item chart.
*/
const char *  const   my_w_flags [] =
{
"take", "Fingers", "Neck", "Chest", "Head", "Legs", "Feet", "Hands", "Arms",
"Shield", "About body", "Waist", "Wrists", "wield", "Held", "_dual_", "Ears", "Cape/Cloak",
"_missile_", "shoulders","Quiver","r3","r4","r5","r6",
"r7","r8","r9","r10","r11","r12","r13"
};

void do_object_csv_dump(CHAR_DATA *ch, const char* argument)
{
	OBJ_INDEX_DATA *pObjIndex;
	FILE *fp;
	int vnum,nMatch = 0;
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	/*    CHAR_DATA *rch; */
	/*    int door; */
	AFFECT_DATA *paf;

	/* open file */
	fclose(fpReserve);
    
	/* start printing out object data */
    fp = fopen("../obj_csv.txt","w");

    fprintf(fp,"\nObject Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
    {
		char	name[MAX_INPUT_LENGTH];
		char	layer[MAX_INPUT_LENGTH];
		char	location[MAX_INPUT_LENGTH];
		char	rare[MAX_INPUT_LENGTH];
		int		ac, ac_or_damage;
		int		min_damage, max_damage, average_damage;
		int		max_condition;
		int		weight;
		int		hp, mana, move;
		int		number_of_specials;
		int		num_stats;
		int		num_res;
		int		num_susc;
		char	area_name[MAX_INPUT_LENGTH];
		char	notes[MAX_STRING_LENGTH];
		char	vnum[MAX_INPUT_LENGTH];
		int		newflags;
		int		flags;
		
		strcpy(notes, ".. ");
		strcpy(area_name, "");
		
        nMatch++;

        obj = create_object( pObjIndex, 0 );

		strcpy(name, obj->name_.c_str());
		ac_or_damage = 0;


		switch(obj->item_type)
		{
			default: continue; break;
			
			case ITEM_ARMOR:
				newflags = obj->wear_flags;
				newflags &= ~(ITEM_TAKE);
				strcpy(location, flag_string(newflags, my_w_flags));

				ac = obj->value[OBJECT_ARMOR_AC];
				ac_or_damage = ac;
		
				switch(obj->pIndexData->layers)
				{
					default:			strcpy(layer, "Unknown!");		break;
					case 0:				strcpy(layer, "none");			break;
					case 1:				strcpy(layer, "1");				break;
					case 2:				strcpy(layer, "2");				break;
					case 4:				strcpy(layer, "3");				break;
					case 8:				strcpy(layer, "4");				break;
					case 16:			strcpy(layer, "5");				break;
					case 32:			strcpy(layer, "6");				break;
					case 64:			strcpy(layer, "7");				break;
					case 128:			strcpy(layer, "8"); 			break;
				}
				
				break;
				
			case ITEM_WEAPON:
				strcpy(location, "Weapon");
				min_damage = 0;
				max_damage = 0;
				min_damage = obj->value[OBJECT_WEAPON_MINDAMAGE];
				max_damage = obj->value[OBJECT_WEAPON_MAXDAMAGE];
				average_damage = (min_damage + max_damage) / 2;
				sprintf(buf, "Min: %d Max: %d ", min_damage, max_damage);
				strcat(notes, buf);

				strcpy(layer, weapontype_number_to_name(obj->value[OBJECT_WEAPON_WEAPONTYPE]));

				ac_or_damage = average_damage;
				
				break;

			case ITEM_PROJECTILE:
				strcpy(location, "Weapon");
				min_damage = 0;
				max_damage = 0;
				min_damage = obj->value[OBJECT_PROJECTILE_MINDAMAGE];
				max_damage = obj->value[OBJECT_PROJECTILE_MAXDAMAGE];
				average_damage = (min_damage + max_damage) / 2;
				sprintf(buf, "Min: %d Max: %d ", min_damage, max_damage);
				strcat(notes, buf);

				switch(obj->value[OBJECT_PROJECTILE_AMMOTYPE])
				{
					default:
						strcpy(layer, ammotype_number_to_name(obj->value[OBJECT_PROJECTILE_AMMOTYPE]));
						break;
					case AMMO_LONGARROW:	case AMMO_BOLT:
						strcpy(layer, "Long arrow / bolt");
						break;
				}
				
				break;

			case ITEM_LIGHT:
				strcpy(location, "Lights");
				strcpy(layer, "1");
				break;

			case ITEM_HOLD:
				strcpy(location, "Held");
				strcpy(layer, "1");
				break;
		}

		strcpy(vnum, vnum_to_dotted(obj->pIndexData->vnum));
		max_condition = obj->max_condition;
		weight = obj->pIndexData->weight;

		switch(obj->pIndexData->rare)
		{
			default:		strcpy(rare, "Unknown!");	break;
			case 0:			strcpy(rare, "21+");		break;
			case 1:			strcpy(rare, "1");			break;
			case 2:			strcpy(rare, "2");			break;
			case 3:			strcpy(rare, "3");			break;
			case 4:			strcpy(rare, "4");			break;
			case 5:			strcpy(rare, "5");			break;
			case 6: case 7: case 8: case 9: case 10: case 11:
			case 12: case 13: case 14: case 15: case 16:
			case 17: case 18: case 19: case 20:
							strcpy(rare, "6 to 20");	break;	
		}

		number_of_specials = 0;
		hp = 0; mana = 0; move = 0;
		num_stats = 0; num_res = 0; num_susc = 0;
		
        for ( paf = obj->pIndexData->first_affect; paf != NULL; paf = paf->next )
        {
			switch(paf->location)
			{
				default:
					sprintf(buf, ". Af %s by %d .", affect_loc_name(paf->location), paf->modifier);
					strcat(notes, buf);
					break;
					
				case APPLY_STR:	case APPLY_INT:	case APPLY_CON:	case APPLY_CHA:
				case APPLY_DEX:	case APPLY_WIS:	case APPLY_LCK:
					number_of_specials++;
					num_stats++;
					sprintf(buf, ". Stat %s: %d .", affect_loc_name(paf->location), paf->modifier);
					strcat(notes, buf);
					
					break;

				case APPLY_HIT:		number_of_specials++;	hp = paf->modifier;		break;
				case APPLY_MANA:	number_of_specials++;	mana = paf->modifier;	break;
				case APPLY_MOVE:	number_of_specials++;	move = paf->modifier;	break;
				
				case APPLY_RESISTANT:
					number_of_specials++;
					num_res++;
					sprintf(buf, ". Res %s .", ris_number_to_name(paf->modifier));
					strcat(notes, buf);
					break;
				case APPLY_SUSCEPTIBLE:
					num_susc++;
					number_of_specials++;
					sprintf(buf, ". Susc %s .", ris_number_to_name(paf->modifier));
					strcat(notes, buf);
					break;
				case APPLY_IMMUNE:
					number_of_specials++;
					sprintf(buf, ". Imm %s .", ris_number_to_name(paf->modifier));
					strcat(notes, buf);
					break;
			}
			
            if ( paf->duration > -1 )
			{
                sprintf( buf, "(%d hours).", paf->duration );
				strcat(notes, buf);
			}
        }

		sprintf(buf, "-- %s", extra_bit_name(obj->pIndexData->extra_flags));
		strcat(notes, buf);

		fprintf(fp, "\"%s\",,%s,0,0,Newbie,%s,%s,", name, layer, location, rare);
		fprintf(fp, "%d,%d,%d,%d,%d,%d,", ac_or_damage, max_condition, weight, hp, mana, move);
		fprintf(fp, "%d,%d,%d,0,0,0,0,0,", num_stats, num_res, num_susc);
		fprintf(fp, "0,%s,%s,%s,%s,", (number_of_specials > 0) ? "Yes" : "No", area_name, notes, vnum);
		if (obj->pIndexData->item_type == ITEM_WEAPON)
			fprintf(fp, "%d,%d,", obj->pIndexData->value[OBJECT_WEAPON_MINDAMAGE], obj->pIndexData->value[OBJECT_WEAPON_MAXDAMAGE]);
		else
			fprintf(fp, "0,0,");
		if (obj->pIndexData->item_type == ITEM_ARMOR)
			fprintf(fp, "%d,", obj->pIndexData->value[OBJECT_ARMOR_CLASSIFICATION]);
		else
			fprintf(fp, "0,");

		flags = obj->pIndexData->extra_flags;
		fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", IS_SET(flags, ITEM_ORGANIC) ? 1 : 0, 	IS_SET(flags, ITEM_METAL) ? 1 : 0,
				IS_SET(flags, ITEM_GLOW) ? 1 : 0, 		IS_SET(flags, ITEM_HUM) ? 1 : 0, 		IS_SET(flags, ITEM_DARK) ? 1 : 0,
				IS_SET(flags, ITEM_EVIL) ? 1 : 0,		IS_SET(flags, ITEM_INVIS) ? 1 : 0,		IS_SET(flags, ITEM_MAGIC) ? 1 : 0,
				IS_SET(flags, ITEM_NODROP) ? 1 : 0,		IS_SET(flags, ITEM_BLESS) ? 1 : 0,		IS_SET(flags, ITEM_INVENTORY) ? 1 : 0);
	
	    fprintf( fp, "\n" );
    	extract_obj( obj , FALSE);

    }       /* if */
	
    /* close file */
    fclose(fp);
    fpReserve = fopen( NULL_FILE, "r" );

    send_to_char( "Done writing file...\n\r", ch );
}

/* KSILYAN
	Now take the Excel chart printed data and make it game data.
*/

void do_object_csv_read(CHAR_DATA *ch, const char* argument)
{
	OBJ_INDEX_DATA *pObjIndex;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;

	extern int  top_affect;

	/* open file */
	fclose(fpReserve);

	/* start printing out object data */
	fp = fopen("../obj_csv_in.txt","r");

	for (;;)
	{
		char LayerText[MAX_INPUT_LENGTH];			int Layer;
		char LocationText[MAX_INPUT_LENGTH];		int Location;
		char RarityText[MAX_INPUT_LENGTH];
		int AcDamPower;
		int Weight, Condition;
		int HPRange, Mana, Move;
		int Stats;
		int Resistances, Susceptibles, LesserAffects, MediumAffects, MajorAffects, Class, Alignment;
		int Cost;
		char Vnum[MAX_INPUT_LENGTH];
		int Min, Max, Classification;
		int Organic, Metal, Glow, Hum, Dark, Evil, Invis, Magic, Nodrop, Bless, Inventory;

		
		if (feof(fp))
			break;
	
		strcpy(LayerText, strlower(fread_word(fp)));
		if (feof(fp))
			break; /* extra security */
		strcpy(LocationText, strlower(fread_word(fp)));
		strcpy(RarityText, strlower(fread_word(fp)));
		AcDamPower = fread_number(fp);
		Condition = fread_number(fp);
		Weight = fread_number(fp);
		HPRange = fread_number(fp);
		Mana = fread_number(fp);
		Move = fread_number(fp);
		Stats = fread_number(fp);
		Resistances = fread_number(fp);
		Susceptibles = fread_number(fp);
		LesserAffects = fread_number(fp);
		MediumAffects = fread_number(fp);
		MajorAffects = fread_number(fp);
		Class = fread_number(fp);
		Alignment = fread_number(fp);
		Cost = fread_number(fp);
		strcpy(Vnum,fread_word(fp));
		Min = fread_number(fp);
		Max = fread_number(fp);
		Classification = fread_number(fp);
		Organic = fread_number(fp);	Metal = fread_number(fp);
		Glow = fread_number(fp);	Hum = fread_number(fp); 	Dark = fread_number(fp);
		Evil = fread_number(fp);	Invis = fread_number(fp);	Magic = fread_number(fp);
		Nodrop = fread_number(fp);	Bless = fread_number(fp);	Inventory = fread_number(fp);

		pObjIndex = get_obj_index(dotted_to_vnum(0, Vnum));

		if (pObjIndex == NULL)
		{
			ch_printf(ch, "Object vnum %s does not exist in the game.\n\r", Vnum);
			continue;
		}

		if (!str_cmp(LocationText, "projectile"))
		{
			pObjIndex->item_type = ITEM_PROJECTILE;
			Location = ITEM_HOLD && ITEM_TAKE;
			pObjIndex->wear_flags = Location;

			if (!str_cmp(LayerText, "longarrow/bolt"))
			{
				ch_printf( ch, "Need to set object vnum %s to either long arrow or bolt.\n\r", Vnum);
				pObjIndex->value[OBJECT_PROJECTILE_AMMOTYPE] = AMMO_LONGARROW;
			}
			else if (!str_cmp(LayerText, "shortarrow"))
			{
				pObjIndex->value[OBJECT_PROJECTILE_AMMOTYPE] = AMMO_SHORTARROW;
			}
			else if (!str_cmp(LayerText, "stone"))
			{
				pObjIndex->value[OBJECT_PROJECTILE_AMMOTYPE] = AMMO_STONE;
			}
			else
			{
				ch_printf( ch, "Warning! Unknown projectile type, object vnum %s.\n\r", Vnum);
				continue;
			}

			pObjIndex->value[OBJECT_PROJECTILE_MINDAMAGE] = Min;
			pObjIndex->value[OBJECT_PROJECTILE_MAXDAMAGE] = Max;
			pObjIndex->value[OBJECT_PROJECTILE_RANGE] = HPRange;
			pObjIndex->value[OBJECT_PROJECTILE_DAMAGETYPE] = ammotype_to_damagetype(pObjIndex->value[OBJECT_PROJECTILE_AMMOTYPE]);
			pObjIndex->value[OBJECT_PROJECTILE_DAMAGEMESSAGE] =
					ammotype_to_damagemessage(pObjIndex->value[OBJECT_PROJECTILE_DAMAGEMESSAGE]);
		}
		else if (!str_cmp(LocationText, "rangedweapon"))
		{
			pObjIndex->item_type = ITEM_WEAPON;
			Location = ITEM_TAKE && ITEM_WIELD;
			pObjIndex->wear_flags = Location;

			if (!str_cmp(LayerText, "longbow"))
			{
				pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_LONGBOW;
			}
			else if (!str_cmp(LayerText, "shortbow"))
			{
				pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_SHORTBOW;
			}
			else if (!str_cmp(LayerText, "crossbow"))
			{
				pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_CROSSBOW;
			}
			else if (!str_cmp(LayerText, "sling"))
			{
				pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_SLING;
			}
			else
			{
				ch_printf( ch, "Warning! Unknown weapon type, object vnum %s.\n\r", Vnum);
			}

			pObjIndex->value[OBJECT_WEAPON_POWER] = AcDamPower;
			pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = weapontype_to_ammotype(pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE]);
		}
		else if (!str_cmp(LocationText, "weapon"))
		{
			int WeaponType;
			pObjIndex->item_type = ITEM_WEAPON;
			Location = ITEM_TAKE && ITEM_WIELD;
			pObjIndex->wear_flags = Location;

			WeaponType = weapontype_name_to_number_nospaces(strlower(LayerText));
			pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WeaponType;
			if (WeaponType == WEAPON_NONE)
			{
				ch_printf(ch, "Warning! Weapon vnum %s has an illegal weapon type.\n\r", Vnum);
			}

			pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = weapontype_to_damagetype(WeaponType);
			pObjIndex->value[OBJECT_WEAPON_DAMAGEMESSAGE] = weapontype_to_damagemsg(WeaponType);
			pObjIndex->value[OBJECT_WEAPON_POWER] = 0;
			pObjIndex->value[OBJECT_WEAPON_MINDAMAGE] = Min;
			pObjIndex->value[OBJECT_WEAPON_MAXDAMAGE] = Max;
		}
		else
		{
			/* All the rest of this stuff SHOULD be armor. */
			if (!str_cmp(LocationText, "held"))
				Location = ITEM_TAKE && ITEM_HOLD;
			else if (!str_cmp(LocationText, "lights")) {
				Location = ITEM_TAKE; pObjIndex->item_type = ITEM_LIGHT; }
			else if (!str_cmp(LocationText, "finger"))
				Location = ITEM_TAKE && ITEM_WEAR_FINGER;
			else if (!str_cmp(LocationText, "neck"))
				Location = ITEM_TAKE && ITEM_WEAR_NECK;
			else if (!str_cmp(LocationText, "head"))
				Location = ITEM_TAKE && ITEM_WEAR_HEAD;
			else if (!str_cmp(LocationText, "hands"))
				Location = ITEM_TAKE && ITEM_WEAR_HANDS;
			else if (!str_cmp(LocationText, "feet"))
				Location = ITEM_TAKE && ITEM_WEAR_FEET;
			else if (!str_cmp(LocationText, "wrist"))
				Location = ITEM_TAKE && ITEM_WEAR_WRIST;
			else if (!str_cmp(LocationText, "ears"))
				Location = ITEM_TAKE && ITEM_WEAR_EARS;
			else if (!str_cmp(LocationText, "chest"))
				Location = ITEM_TAKE && ITEM_WEAR_BODY;
			else if (!str_cmp(LocationText, "aboutbody"))
				Location = ITEM_TAKE && ITEM_WEAR_ABOUT;
			else if (!str_cmp(LocationText, "legs"))
				Location = ITEM_TAKE && ITEM_WEAR_LEGS;
			else if (!str_cmp(LocationText, "arms"))
				Location = ITEM_TAKE && ITEM_WEAR_ARMS;
			else if (!str_cmp(LocationText, "waist"))
				Location = ITEM_TAKE && ITEM_WEAR_WAIST;
			else if (!str_cmp(LocationText, "shield"))
				Location = ITEM_TAKE && ITEM_WEAR_SHIELD;
			else if (!str_cmp(LocationText, "cape/cloak"))
				Location = ITEM_TAKE && ITEM_WEAR_CAPE;
			else
			{
				ch_printf(ch, "Warning! Object vnum %s is not of a proper location.\n\r", Vnum);
				continue;
			}

			pObjIndex->wear_flags = Location;

			Layer = atoi(LayerText);
			pObjIndex->layers = 1 << (Layer - 1);

			if (pObjIndex->item_type != ITEM_LIGHT)
				pObjIndex->item_type = ITEM_ARMOR;

			pObjIndex->value[OBJECT_ARMOR_AC] = AcDamPower;
			pObjIndex->value[OBJECT_ARMOR_CLASSIFICATION] = Classification;
		}

		pObjIndex->max_condition = Condition;
		pObjIndex->weight = Weight;
		if (pObjIndex->item_type != ITEM_PROJECTILE)
		{
			if (pObjIndex->first_affect != NULL)
			{
				for ( paf = pObjIndex->first_affect; paf; paf = pObjIndex->first_affect)
				{
					UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
					DISPOSE( paf );
					--top_affect;
				}
			}

			if (HPRange > 0)
			{
				AFFECT_DATA * paf;
				CREATE( paf, AFFECT_DATA, 1 );
				paf->type       = -1;
				paf->duration       = -1;
				paf->location       = APPLY_HIT;
				paf->modifier       = HPRange;
				paf->bitvector      = 0;
				paf->next       = NULL;
				LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
				top_affect++;
			}
			if ( Mana > 0 )
			{
				AFFECT_DATA * paf;
				CREATE( paf, AFFECT_DATA, 1 );
				paf->type       = -1;
				paf->duration       = -1;
				paf->location       = APPLY_MANA;
				paf->modifier       = Mana;
				paf->bitvector      = 0;
				paf->next       = NULL;
				LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
				top_affect++;
			}
			if ( Move > 0 )
			{
				AFFECT_DATA * paf;
				CREATE( paf, AFFECT_DATA, 1 );
				paf->type       = -1;
				paf->duration       = -1;
				paf->location       = APPLY_MOVE;
				paf->modifier       = Move;
				paf->bitvector      = 0;
				paf->next       = NULL;
				LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
				top_affect++;
			}

			if (!str_cmp(RarityText, "21+"))
				pObjIndex->rare = 0;
			else if (!str_cmp(RarityText, "6to20"))
				ch_printf(ch, "Object vnum %s has rare between 6 and 20; needs to be set.\n\r", Vnum);
			else
			{
				int Rarity;
				Rarity = atoi(RarityText);
				if (Rarity >= 1 && Rarity <= 5)
					pObjIndex->rare = Rarity;
				else
					ch_printf(ch, "Object vnum %s needs to have its rare set.\n\r", Vnum);
			}
		}

		pObjIndex->extra_flags = 0;
		if (Organic == 1) SET_BIT(pObjIndex->extra_flags, ITEM_ORGANIC);
		if (Metal == 1)	SET_BIT(pObjIndex->extra_flags, ITEM_METAL);
		if (Glow == 1)	SET_BIT(pObjIndex->extra_flags, ITEM_GLOW);
		if (Hum == 1)	SET_BIT(pObjIndex->extra_flags, ITEM_HUM);
		if (Dark == 1)	SET_BIT(pObjIndex->extra_flags, ITEM_DARK);
		if (Evil == 1)	SET_BIT(pObjIndex->extra_flags, ITEM_EVIL);
		if (Invis == 1)	SET_BIT(pObjIndex->extra_flags, ITEM_INVIS);
		if (Magic == 1) SET_BIT(pObjIndex->extra_flags, ITEM_MAGIC);
		if (Nodrop == 1) SET_BIT(pObjIndex->extra_flags, ITEM_NODROP);
		if (Bless == 1)	SET_BIT(pObjIndex->extra_flags, ITEM_BLESS);
		if (Inventory == 1) SET_BIT(pObjIndex->extra_flags, ITEM_INVENTORY);

		if ( Stats + Resistances + Susceptibles + LesserAffects + MediumAffects + MajorAffects + Class + Alignment > 0)
		{
			sprintf(buf, "Object %s (vnum %s) needs manual updating.\n\r", pObjIndex->shortDesc_.c_str(), Vnum);
			send_to_char(buf, ch);
		}
	}

	/* close file */
	fclose(fp);
	fpReserve = fopen( NULL_FILE, "r" );

	send_to_char( "Done reading file...\n\r", ch );
	return;
}

/* 
 * new_dump written by Rahl (Daniel Anderson) of Broken Shadows
 */
void do_new_dump(CHAR_DATA *ch, const char* argument)
{
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
/*    ROOM_INDEX_DATA *pRoomIndex; */
    FILE *fp;
    int vnum,nMatch = 0;
    OBJ_DATA *obj;
/*    CHAR_DATA *rch; */
/*    int door; */
    AFFECT_DATA *paf;
    CHAR_DATA *mob;

    /* open file */
    fclose(fpReserve);

    /* start printing out mobile data */
    fp = fopen("../mob.txt","w");

    fprintf(fp,"\nMobile Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_mob_index; vnum++)
    if ((pMobIndex = get_mob_index(vnum)) != NULL)
    {
        nMatch++;
        mob = create_mobile( pMobIndex );
        if (mob->GetInRoom() == NULL)
        {
            mob->InRoomId = get_room_index(ROOM_VNUM_LIMBO)->GetId();
        }
        fprintf( fp, "Name: %s.\n",
            mob->getName().c_str() );

        fprintf( fp, "Vnum: %d  Race: %s  Sex: %s  Room: %d  Count %d\n", 
            IS_NPC(mob) ? mob->pIndexData->vnum : 0,
            race_table[mob->race].race_name,
            mob->sex == SEX_MALE    ? "male"   :
            mob->sex == SEX_FEMALE  ? "female" : "neutral",
            mob->GetInRoom() == NULL    ?        0 : mob->GetInRoom()->vnum, 
            mob->pIndexData->count );
        
        fprintf( fp, 
            "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n",
                mob->perm_str,
                mob->getStr(),
                mob->perm_int,
                mob->getInt(),
                mob->perm_wis,
                mob->getWis(),
                mob->perm_dex,
                mob->getDex(),
                mob->perm_con,
                mob->getCon() );

        fprintf( fp, "Hp: %d  Mana: %d  Move: %d  Hit: %d  Dam: %d\n",
                mob->max_hit,
            mob->max_mana,
            mob->max_move,
            mob->getHitRoll(), mob->getDamRoll() );

        fprintf( fp,
            "Lv: %d  Align: %d  Gold: %d  Damage: %dd%d\n",
                mob->level,                   
                mob->alignment,
                mob->gold,
                mob->barenumdie, mob->baresizedie);

        fprintf(fp,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n",
            mob->getAC(), mob->getAC(),
            mob->getAC(),  mob->getAC() );

        fprintf(fp, "Act: %s\n",flag_string(mob->act, act_flags));
    
        if (IS_NPC(mob) && mob->attacks )
        {
            fprintf(fp, "Offense: %s\n", flag_string(mob->attacks, attack_flags) );
        }

        if (mob->immune)
        {
            fprintf(fp, "Immune: %s\n", ris_number_to_name(mob->immune));
        }
 
        if (mob->resistant)
        {
            fprintf(fp, "Resist: %s\n", ris_number_to_name(mob->resistant));
        }

        if (mob->susceptible)
        {
            fprintf(fp, "Vulnerable: %s\n", ris_number_to_name(mob->resistant));
        }

        fprintf(fp, "Parts: %s\n", 
            flag_string(mob->xflags, part_flags));

        if (mob->affected_by)
        {
            fprintf(fp, "Affected by %s\n", 
                affect_bit_name(mob->affected_by));
        }

        fprintf( fp, "Short description: %s\nLong  description: %s",
            mob->getShort().c_str(),
            mob->longDesc_.length() > 0 ? mob->longDesc_.c_str() : "(none)\n" );

/*        if ( IS_NPC(mob) && mob->spec_fun != 0 )
        {
            sprintf( buf, "Mobile has special procedure. - %s\n", spec_string( mob->spec_fun ) );
            fprintf( fp, buf );
        }*/

        
        for ( paf = mob->first_affect; (paf != NULL); paf = paf->next )
        {
            fprintf( fp,
                "Spell: '%s' modifies %s by %d for %d hours with bits %s.\n",
                skill_table[(int) paf->type]->name_.c_str(),
                affect_loc_name( paf->location ),
                paf->modifier,
                paf->duration,
                affect_bit_name( paf->bitvector )
                );
        }
        fprintf( fp, "\n" );
        extract_char( mob, FALSE );
    }
    fclose(fp);

    /* start printing out object data */
    fp = fopen("../obj.txt","w");

    fprintf(fp,"\nObject Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL)
    {
        nMatch++;

        obj = create_object( pObjIndex, 0 );

        fprintf( fp, "Name(s): %s\n",
            obj->name_.c_str() );

        fprintf( fp, "Vnum: %d  Type: %s  Number: %d/%d  Weight: %d/%d\n",
            obj->pIndexData->vnum, 
            itemtype_number_to_name(obj->item_type), 1, get_obj_number( obj ),
                        obj->weight, get_obj_weight( obj ) );

        fprintf( fp, "Short description: %s\nLong description: %s\n",
            obj->shortDesc_.c_str(), obj->longDesc_.c_str() );

        fprintf( fp, "Wear bits: %s\tExtra bits: %s\n",
            flag_string(obj->wear_flags, w_flags), flag_string(obj->extra_flags, o_flags) );

        fprintf( fp, "Cost: %d  Timer: %d\n",
            obj->cost, obj->timer );

        fprintf( fp,
            "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n",
            obj->GetInRoom()    == NULL    ?        0 : obj->GetInRoom()->vnum,
            obj->GetInObj()     == NULL    ? "(none)" : obj->GetInObj()->shortDesc_.c_str(),
            obj->GetCarriedBy() == NULL    ? "(none)" : obj->GetCarriedBy()->getName().c_str(),
            obj->wear_loc );
    
        fprintf( fp, "Values: %d %d %d %d %d\n",
            obj->value[0], obj->value[1], obj->value[2], obj->value[3],
            obj->value[4] );
    
        /* now give out vital statistics as per identify */
    
        switch ( obj->item_type )
        {
            case ITEM_SCROLL: 
            case ITEM_POTION:
            case ITEM_PILL:
                fprintf( fp, "Level %d spells of:", obj->value[0] );

                if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
                {
                    fprintf( fp, " '" );
                    fprintf( fp, "%s", skill_table[obj->value[1]]->name_.c_str() );
                    fprintf( fp, "'" );
                }

                if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
                {
                    fprintf( fp, " '" );
                    fprintf( fp, "%s", skill_table[obj->value[2]]->name_.c_str() );
                    fprintf( fp, "'" );
                }

                if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
                {
                    fprintf( fp, " '" );
                    fprintf( fp, "%s", skill_table[obj->value[3]]->name_.c_str() );
                    fprintf( fp, "'" );
                }

                fprintf( fp, ".\n" );
                break;

            case ITEM_WAND: 
            case ITEM_STAFF: 
                fprintf( fp, "Has %d(%d) charges of level %d",
                    obj->value[1], obj->value[2], obj->value[0] );
      
                if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
                {
                    fprintf( fp, " '" );
                    fprintf( fp, "%s", skill_table[obj->value[3]]->name_.c_str() );
                    fprintf( fp, "'" );
                }

                fprintf( fp, ".\n" );
                break;
      
            case ITEM_WEAPON:
                fprintf( fp, "Weapon type is " );
                switch (obj->value[3])
                {
//                    case(WEAPON_EXOTIC)     : fprintf(fp, "exotic\n");  break;
                    case(DAM_BLAST)      : fprintf(fp, "pugilism\n");   break;  
                    case(DAM_SLICE):case(DAM_SLASH)     : fprintf(fp, "long blade\n");  break;
                    case(DAM_PIERCE):case(DAM_STAB)  : fprintf(fp, "short blade\n"); break;
                    case(DAM_WHIP)   : fprintf(fp, "flexible arm\n");   break;
                    case(DAM_CLAW)    : fprintf(fp, "talonous arm\n");     break;
                    case(DAM_POUND):case(DAM_CRUSH)  : fprintf(fp, "bludgeon\n");   break;
//                    case(WEAPON_WHIP)   : fprintf(fp, "whip\n");        break;
//                    case(WEAPON_POLEARM)    : fprintf(fp, "polearm\n"); break;
                    default         : fprintf(fp, "unknown\n"); break;
                }
                fprintf( fp, "Damage is %d to %d (average %d)\n",
                    obj->value[1], obj->value[2],
                    ( obj->value[1] + obj->value[2] ) / 2 );
        
                break;

            case ITEM_ARMOR:
                fprintf( fp, 
                    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n", 
                    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
                break;
        }  /* switch */

        for ( paf = obj->first_affect; paf != NULL; paf = paf->next )
        {
            fprintf( fp, "Affects %s by %d",
                affect_loc_name( paf->location ), paf->modifier );
            /* added by Rahl */
            if ( paf->duration > -1 )
                fprintf( fp, ", %d hours.\n", paf->duration );
            else
                fprintf( fp, ".\n" );
            if ( paf->bitvector )
            {
                switch ( paf->location )
                {
                    case APPLY_AFFECT:
                        fprintf( fp, "Adds %s affect.\n", 
                            affect_bit_name( paf->bitvector ) );
                        break;
/*                    case TO_WEAPON:
                        sprintf( buf, "Adds %s weapon flags.\n",
                            weapon_bit_name( paf->bitvector ) );
                            break;
                    case TO_OBJECT:
                        sprintf( buf, "Adds %s object flag.\n",
                            extra_bit_name( paf->bitvector ) );
                        break;*/
                    case APPLY_IMMUNE:
                        fprintf( fp, "Adds immunity to %d.\n",
                            paf->bitvector );
                        break;
                    case APPLY_RESISTANT:
                        fprintf( fp, "Adds resistance to %d.\n",
                            paf->bitvector  );
                        break;
                    case APPLY_SUSCEPTIBLE:
                        fprintf( fp, "Adds vulnerability to %d.\n",
                             paf->bitvector  );
                        break;
                    default:
                        fprintf( fp, "Unknown bit %d %d\n",
                            paf->location, paf->bitvector );
                        break;
                }
            }  /* if */
        }  /* for */

    fprintf( fp, "\n" );
    extract_obj( obj , FALSE);

    }       /* if */
    /* close file */
    fclose(fp);
    

    /* start printing out room data */
 /*   fp = fopen("../room.txt","w");

    fprintf(fp,"\nRoom Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_vnum_room; vnum++)
    if ((pRoomIndex = get_room_index(vnum)) != NULL)
    {
        nMatch++;
        sprintf( buf, "Name: '%s.'\nArea: '%s'.\n",
            pRoomIndex->name,
            pRoomIndex->area->name );
        fprintf( fp, buf );

        sprintf( buf,
            "Vnum: %d.  Sector: %d.  Light: %d.\n",
            pRoomIndex->vnum,
            pRoomIndex->sector_type,
            pRoomIndex->light );
        fprintf( fp, buf );

        sprintf( buf,
            "Room flags: %d.\nDescription:\n%s",
            pRoomIndex->room_flags,
            pRoomIndex->description );
        fprintf( fp, buf );

        if ( pRoomIndex->extra_descr != NULL )
        {
            EXTRA_DESCR_DATA *ed;

            fprintf( fp, "Extra description keywords: '" );
            for ( ed = pRoomIndex->extra_descr; ed; ed = ed->next )
            {
                fprintf( fp, ed->keyword );
                if ( ed->next != NULL )
                    fprintf( fp, " " );
            }
            fprintf( fp, "'.\n" );
        }

        fprintf( fp, "Characters:" );
        for ( rch = pRoomIndex->people; rch; rch = rch->next_in_room )
        {
            fprintf( fp, " " );
            one_argument( rch->name, buf );
            fprintf( fp, buf );
        }
    
        fprintf( fp, ".\nObjects:   " );
        for ( obj = pRoomIndex->contents; obj; obj = obj->next_content )
        {
            fprintf( fp, " " );
            one_argument( obj->name, buf );
            fprintf( fp, buf );
        }
        fprintf( fp, ".\n" );

        for ( door = 0; door <= 5; door++ )
        {
            EXIT_DATA *pexit;

            if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
            {
                sprintf( buf,
                    "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\nKeyword: '%s'.  Description: %s",
                    door,
                    (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
                    pexit->key,
                    pexit->exit_info,
                    pexit->keyword,
                    pexit->description[0] != '\0' ? pexit->description : "(none).\n" );
                fprintf( fp, buf );
            }
        }

    }
*/  
    /* close file */
/*    fclose(fp); */

    fpReserve = fopen( NULL_FILE, "r" );

    send_to_char( "Done writing files...\n\r", ch );
}

/*
 =============================================================================
/   ______ _______ ____   _____   ___ __    _ ______    ____  ____   _____   /
\  |  ____|__   __|  _ \ / ____\ / _ \| \  / |  ____|  / __ \|  _ \ / ____\  \
/  | |__     | |  | |_| | |     | |_| | |\/| | |___   | |  | | |_| | |       /
/  | ___|    | |  | ___/| |   __|  _  | |  | | ____|  | |  | |  __/| |   ___ \
\  | |       | |  | |   | |___| | | | | |  | | |____  | |__| | |\ \| |___| | /
/  |_|       |_|  |_|  o \_____/|_| |_|_|  |_|______|o \____/|_| \_|\_____/  \
\                                                                            /
 ============================================================================

------------------------------------------------------------------------------
ftp://ftp.game.org/pub/mud      FTP.GAME.ORG      http://www.game.org/ftpsite/
------------------------------------------------------------------------------

   This file came from FTP.GAME.ORG, the ultimate source for MUD resources.

------------------------------------------------------------------------------
*/


