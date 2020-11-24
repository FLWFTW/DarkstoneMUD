


#ifndef __OBJECT_H_
#define __OBJECT_H_

#include "mud.h"
#include "managed_objects.hpp"



class Object;
extern tPointerOwner<Object> ObjectMap; // object.cpp

typedef tId<Object> idObject;

// forward declarations
typedef tId<Room> idRoom;
typedef tId<Character> idCharacter;


class HolderData
{
public:
	string Host;
	string Name;
	string Time;
	string Account;

	HolderData() { Host = ""; Name = ""; Time = ""; Account = ""; }
};

/*
 * One object.
 */
class Object : public tManagedObject<Object>, public cabSkryptable
{
public:
	Object();
	virtual ~Object();


	// Send a Skrypt event.
	virtual void skryptSendEvent(const string & eventName, list<Argument*> & arguments);
	
	HolderData * Holders[2];

	// old stuff
	
	OBJ_DATA *		next;
	OBJ_DATA *		prev;
	OBJ_DATA *		next_content;
	OBJ_DATA *		prev_content;
	OBJ_DATA *		first_content;
	OBJ_DATA *		last_content;
	
	ExtraDescData *	first_extradesc;
	ExtraDescData *	last_extradesc;
	AFFECT_DATA *	first_affect;
	AFFECT_DATA *	last_affect;
	OBJ_INDEX_DATA *	pIndexData;
	
	OBJ_NPC_DATA *		pObjNPC;
	shared_str name_;
	shared_str shortDesc_;
	shared_str longDesc_; // changed from char* to HashedString for stability - Ksilyan sep-12-2004
	shared_str actionDesc_;
	sh_int		item_type;
	sh_int		mpscriptpos;
	int 		extra_flags;
	int 		extra_flags_2; /* to get more flags - Ksilyan */
	int 		magic_flags; /*Need more bitvectors for spells - Scryn*/
	int 		wear_flags; 
	MPROG_ACT_LIST *	mpact;		/* mudprogs */
	int 		mpactnum;	/* mudprogs */
	sh_int		wear_loc;
	sh_int		weight;
	int 		cost;
#ifdef USE_OBJECT_LEVELS
	sh_int		level;
#endif	 
	sh_int		timer;
	int 		value	[6];
	sh_int		count;		/* support for object grouping */
	int 		serial; 	/* serial number		   */
	
	short			condition;	/* current condition */
	short			max_condition; /* max condition "strength" */

	idObject InObjId;
	idCharacter CarriedById;
	idRoom InRoomId;
	idObject QuestNoteId;
	idRoom	 TelevisionRoomId; // room this television points to

	/////////////
	// ID Lookups
	/////////////

	Object * GetInObj();
	Character * GetCarriedBy();
	Room * GetInRoom();
	Object * GetQuestNote();
	
	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName() const; // Return the name of the class.
	virtual const string codeGetBasicInfo() const; // Return a short summary of the object.
	virtual const string codeGetFullInfo() const; // Return a complete summary of the object.
};

#endif

