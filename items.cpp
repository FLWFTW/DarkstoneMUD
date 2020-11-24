
/***************************************************************\
*                                                               *
*                           KSILYAN                             *
*                                                               *
*****************************************************************
*                                                               *
* Lots of various code to handle and deal with items in general.*
*                                                               *
\***************************************************************/



#include <stdio.h>
#include <unistd.h>

#include "mud.h"




char * const damage_type_names [] =
{
    "slash",            "pierce",           "whip",             "bludgeon",
    "cold",             "fire",             "electricity",      "energy",
    "acid",             "poison"   
};
/*
    The energy damage type is for all magic that doesn't fall into any category.
*/

char * const weapon_type_names [] = 
{
    "pugilism",         "long blade",		"short blade",		"flexible arm",
    "bludgeon",			"polearm",			"long bow",			"short bow",
	"crossbow",			"sling",			"throwing spear",	"throwing knife"
};


int get_weapon_type( OBJ_INDEX_DATA *obj )
{
    return obj->value[3];
}
int get_weapon_range( OBJ_INDEX_DATA *obj )
{
    return obj->value[4];
}
int get_weapon_damage_type( OBJ_INDEX_DATA *obj )
{
    return obj->value[4];
}


                
