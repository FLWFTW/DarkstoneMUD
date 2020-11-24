

#include "globals.h"
#include "object.h"
#include "stored_objs.h"
#include "mud.h"
#include "World.h"

#include "utility_objs.hpp"


// STL includes
#include <iostream>
#include <sstream>


ObjectStorage::ObjectStorage()
{
}

ObjectStorage::~ObjectStorage()
{
	//for_each(StoredObjects.begin(), StoredObjects.end(), fo_DeleteObject());

	// Clear the vault by extracting all the objects.
	list<Object*>::iterator itor;
	for ( itor = StoredObjects.begin(); itor != StoredObjects.end(); itor++ )
	{
		Object * obj = *itor;
		extract_obj(obj, false);
	}
	StoredObjects.clear();
}

/*
 * AddObject takes an object, makes a StoredObject,
 * and adds it to the list. It assumes that whatever
 * called it dereferences the object (so that it's
 * not floating in space in two places)
 */
void ObjectStorage::AddObject(Object * object)
{
	if (!object)
	{
		gTheWorld->LogBugString("ObjectStorage's AddObject called with null object!");
		return;
	}

	StoredObjects.push_back(object);
}

// Retrieves StoredObject #whichOne and creates an
// object out of it. Removes the StoredObject from
// the list.
Object * ObjectStorage::GetObject(unsigned short whichOne)
{
	if ( whichOne > this->StoredObjects.size() || whichOne < 1)
		return NULL; // invalid value

	int count = 1;

	list<Object*>::iterator itor;
	for (itor = StoredObjects.begin(); itor != StoredObjects.end(); itor++)
	{
		if ( count == whichOne )
		{
			Object * newObject = (*itor);
			StoredObjects.remove(newObject);

			return newObject;
		}
		count++;
	}

	// the code should never get here
	return NULL;
}

void ObjectStorage::ShowToCharacter(Character * ch)
{
	if ( StoredObjects.empty() )
	{
		ch->sendText("Your vault is empty.\n\r");
		return;
	}

	
	list<Object*>::iterator itor;
	int count = 1;
	ostringstream os;
	
	for (itor = StoredObjects.begin(); itor != StoredObjects.end(); itor++)
	{
		os << count << ") " << (*itor)->shortDesc_.str() << endl;
		count++;
	}

	ch->sendText(os.str());
}

