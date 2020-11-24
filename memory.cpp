

#include <stdio.h>

#include "memory.h"

#include <list>
using namespace std;

list<MemoryManager*> AllocatedMemory;

eNoLongerAllocated::eNoLongerAllocated()
{
}
eNoLongerAllocated::~eNoLongerAllocated()
{
}


MemoryManager::MemoryManager()
{
	memReferences = 1;
	memAllocations = 1;
	AllocatedMemory.push_back(this);
}
MemoryManager::~MemoryManager()
{
	AllocatedMemory.remove(this);
}


MemoryManager * MemoryManager::memoryCreateReference()
{
	memReferences++;
	return this;
}
MemoryManager * MemoryManager::memoryCreateAllocation()
{
	memReferences++;
	memAllocations++;
	return this;
}

bool MemoryManager::DeleteReference()
{
	memReferences--;
	if (memReferences <= 0)
		return true; // no more references - time to delete object
	return false; // do not delete object
}
bool MemoryManager::DeleteMemory()
{
	memReferences--; memAllocations--;
	if (memReferences <= 0)
		return true; // no more references - time to delete object
	return false; // do not delete object
}

bool MemoryManager::memoryIsAllocated()
{
	if ( memAllocations <= 0 )
		return false;
	else
		return true;
}

////////////////////////
// Code object interface
////////////////////////

const string MemoryManager::codeGetBasicInfo()
{
	return "(TBA)";	
}

const string MemoryManager::codeGetFullInfo()
{
	return "(TBA)";
}

const string MemoryManager::codeGetClassName()
{
	return "MemoryManager";
}

////////////////////////////////////

void DeleteMemoryAllocation(MemoryManager * memory)
{
	// Abort if null pointer.
	if (!memory)
		return;
	
	if ( !memory->memoryIsAllocated() )
	{
		if ( memory->DeleteReference() )
			delete memory;
	}
	else
	{
		if ( memory->DeleteMemory() )
			delete memory;
	}
}

void DeleteMemoryReference(MemoryManager * memory)
{
	// Abort if null pointer.
	if (!memory)
		return;

	if ( memory->DeleteReference() )
		delete memory;
}


