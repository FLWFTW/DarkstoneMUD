


#include "room.h"
#include "mud.h"

tPointerOwner<Room> RoomMap;


Room::Room() : tManagedObject<Room> (RoomMap)
{
	next = next_sort = NULL;

	first_exit = last_exit = NULL;
	first_person	= NULL;
	last_person	= NULL;
	first_content	= NULL;
	last_content	= NULL;
	next = NULL;
	
	first_extradesc = last_extradesc = NULL;
	area = NULL;
	map = NULL;
	mpact = NULL;
	mudprogs = NULL;

	vnum = room_flags = mpactnum = 0;
	mpscriptpos = 0; progtypes = 0;
	light = 0; sector_type = 0;
	tele_vnum = 0;
	tele_delay = 0;

	tunnel = 0;

	random_room_type = random_description = -1;

	// Initialize the (potential) skrypt container.
	this->RoomSkryptContainer = new SkryptContainer();

	// Rooms are their own skrypt containers
	this->skryptSetContainer(this->RoomSkryptContainer);

	// Default scent
	scentID = 0;
}


////////////////////////
// Code object interface
////////////////////////

const string Room::codeGetClassName() const
{
	return "Room";
}
const string Room::codeGetBasicInfo() const
{
	return "Room vnum " + string(vnum_to_dotted(this->vnum)) + ".";
}
const string Room::codeGetFullInfo() const
{
	return "Room \"" + this->name_.str() + "\", vnum " + string(vnum_to_dotted(this->vnum)) + ".";
}

/*
   ____ __                    __ 
  / __// /__ ____ __ __ ___  / /_
 _\ \ /  '_// __// // // _ \/ __/
/___//_/\_\/_/   \_, // .__/\__/ 
                /___//_/         
*/

void Room::skryptSendEvent(const string & eventName, list<Argument*> & arguments)
{
	if ( !this->skryptContainer_ )
		return;

	ArgumentRoom argMe(this);
	skryptContainer_->SendEvent(eventName, argMe, arguments, this);
}
