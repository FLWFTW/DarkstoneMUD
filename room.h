



#ifndef __ROOM_H_
#define __ROOM_H_

#include "mud.h"
#include "managed_objects.hpp"
#include "skrypt/skrypt_public.h"


class Room;
extern tPointerOwner<Room> RoomMap; // room.cpp

typedef tId<Room> idRoom;


// forward declaration
typedef tId<Object> idObject;


/*
 * Room type.
 */

// Rooms contain their own skrypt code, as opposed
// to objects and mobiles which fetch it from their
// prototype.

class Room : public tManagedObject<Room>, public cabSkryptable
{
public:
	Room();
	
	//////////////////
	// Skrypt Handling
	//////////////////
	
	// Skrypt container
	SkryptContainer * RoomSkryptContainer;
	
	// Send a Skrypt event.
	virtual void skryptSendEvent(const string & eventName, list<Argument*> & arguments);
	
	
	////////////
	// Old stuff
	////////////
	
	Room * next;
	Room * next_sort;
	Character * first_person;
	Character * last_person;
	Object * first_content;
	Object * last_content;
	ExtraDescData * first_extradesc;
	ExtraDescData * last_extradesc;
	AREA_DATA * area;
	ExitData * first_exit;
	ExitData * last_exit;
	shared_str name_;
	MAP_DATA * map; 				/* maps */
	shared_str description_;
	int  vnum;
	int room_flags;
	MPROG_ACT_LIST * mpact; 			  /* mudprogs */
	int mpactnum;			 /* mudprogs */
	MPROG_DATA * mudprogs;			  /* mudprogs */
	sh_int mpscriptpos;
	int64 progtypes;		   /* mudprogs */
	sh_int light;
	sh_int sector_type;
	int tele_vnum;
	sh_int tele_delay;
	sh_int tunnel;			 /* max people that will fit */
	idObject QuestNoteId;  /* for quest writer's room descriptions */
	
	/*
	 * Ksilyan's stuff
	 */
	
	// Random room descriptions
	int random_room_type;
	int random_description;
	
	// The crystal ball
	idObject TelevisionObjectId;

	/*
	 * Aiyan's Stuff
	 */

	// The scent of the room
	int scentID;

	// A map of the minable materials in this room
	std::map<int, int> mineMap;
	
	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName() const; // Return the name of the class.
	virtual const string codeGetBasicInfo() const; // Return a short summary of the object.
	virtual const string codeGetFullInfo() const; // Return a complete summary of the object.
};


#endif

