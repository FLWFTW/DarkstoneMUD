


#include "globals.h"
#include "mud.h"
#include "object.h"
#include "World.h"

tPointerOwner<Object> ObjectMap;


obj_index_data::obj_index_data()
{
	next = next_sort = NULL;
	first_extradesc = last_extradesc = NULL;
	first_affect = last_affect = NULL;
	mudprogs = NULL;

	progtypes = 0;
	vnum = 0;
	level = item_type = 0;
	extra_flags = extra_flags_2 = magic_flags = wear_flags = 0;
	count = weight = 0;
	cost = serial = 0;

	for (int i = 0; i < 6; i++)
		value[i] = 0;

    layers = rare = 0;

	total_count = 0;
	rent = 0;
	max_condition = 0;
}


Object::Object() : tManagedObject<Object> (ObjectMap)
{
	next = prev = next_content = prev_content = first_content = last_content = NULL;
    first_extradesc = last_extradesc = NULL;
    first_affect = last_affect = NULL;
    pIndexData = NULL;
	pObjNPC = NULL;
    item_type = mpscriptpos = 0;
    extra_flags = extra_flags_2 = magic_flags = wear_flags = 0;
    mpact = NULL;
    mpactnum = 0;
    wear_loc = weight = 0;
    cost = 0;
    timer = 0;
	for (int i = 0; i < 6; i++)
		value[i] = 0;
    count = 0;
	serial = 0;
	
	condition = max_condition = 0;

	Holders[0] = new HolderData();
	Holders[1] = new HolderData();
}

Object::~Object()
{
	if (this->Holders[0])
		delete this->Holders[0];
	if (this->Holders[1])
		delete this->Holders[1];
}


/*
=======================================
   ____    __
  /  _/___/ /
 _/ / / _  /
/___/ \_,_/
   __              __
  / /  ___  ___   / /__ __ __ ___   ___
 / /__/ _ \/ _ \ /  '_// // // _ \ (_-<
/____/\___/\___//_/\_\ \_,_// .__//___/
                           /_/
=======================================
*/

// lookups will return null if not found

#define C_OBJECT_LOOKUP(name, type, idName, mapName) \
type Object::name() \
{ \
	if ( !idName ) \
		return NULL; /* don't bother looking up */ \
	else \
	{ \
		type result = mapName[this->idName]; \
		if ( result ) \
			return result; \
		else \
		{ \
			gTheWorld->LogCodeString( \
				"Object::" #name "() invalid " #idName " reference! Obj: " + this->codeGetBasicInfo() ); \
			this->idName = 0; \
			return NULL; \
		} \
	} \
}
// #arg preprocessor operator means: make this argument a string ("arg")

C_OBJECT_LOOKUP(GetInRoom, Room *, InRoomId, RoomMap)

C_OBJECT_LOOKUP(GetInObj, Object *, InObjId, ObjectMap)
C_OBJECT_LOOKUP(GetQuestNote, Object *, QuestNoteId, ObjectMap)

C_OBJECT_LOOKUP(GetCarriedBy, Character *, CarriedById, CharacterMap)


/*
==================================
   ____ __                    __ 
  / __// /__ ____ __ __ ___  / /_
 _\ \ /  '_// __// // // _ \/ __/
/___//_/\_\/_/   \_, // .__/\__/ 
                /___//_/         
==================================
*/

void Object::skryptSendEvent(const string & eventName, list<Argument*> & arguments)
{
	if ( !this->skryptContainer_ )
		return;

	ArgumentObject argMe(this);
	skryptContainer_->SendEvent(eventName, argMe, arguments, this);
}

///////////////////////////
// Code object interface //
///////////////////////////

const string Object::codeGetBasicInfo() const
{
	return "(TBA)";
}

const string Object::codeGetFullInfo() const
{
	return "(TBA)";
}

const string Object::codeGetClassName() const
{
	return "Object";
}


