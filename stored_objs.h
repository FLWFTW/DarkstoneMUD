


#ifndef __STORED_OBJS_H_
#define __STORED_OBJS_H_

#include "memory.h"

// STL includes
#include <string>
#include <list>

// forward declarations
class Object;
class Character;

// A class used to store objects.
class ObjectStorage : public MemoryManager
{
protected:
	std::list<Object*> StoredObjects;

public:
	ObjectStorage();
	virtual ~ObjectStorage();

	void AddObject(Object * object);
	Object * GetObject(unsigned short whichOne);

	std::list<Object*>::iterator ListBegin() { return StoredObjects.begin(); }
	std::list<Object*>::iterator ListEnd() { return StoredObjects.end(); }
	unsigned short ListSize() { return StoredObjects.size(); }

	void ShowToCharacter(Character * ch);
};

#endif
