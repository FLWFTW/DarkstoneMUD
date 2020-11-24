


#ifndef __MEMORY_H_
#define __MEMORY_H_

#include "code_object.h"

class eNoLongerAllocated
{
public:
	eNoLongerAllocated();
	virtual ~eNoLongerAllocated();

	const char * GetReason() const { return "Unit of memory is no longer allocated."; }
};

class MemoryManager : iCodeObject
{
protected:
	short memReferences;
	short memAllocations;

	bool DeleteReference(); // decrease reference count by one, and return true if ref = 0
	bool DeleteMemory(); // decrease reference/allocation count by one, return true if ref = 0

public:
	MemoryManager();
	virtual ~MemoryManager();

	MemoryManager * memoryCreateReference(); // increase reference count by one
	MemoryManager * memoryCreateAllocation(); // increase reference + allocation by one
	
	bool memoryIsAllocated();

	short memoryGetReferences() { return memReferences; }
	short memoryGetAllocations() { return memAllocations; }

	friend void DeleteMemoryAllocation(MemoryManager * memory);
	friend void DeleteMemoryReference(MemoryManager * memory);

	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName(); // Return the name of the class.
	virtual const string codeGetBasicInfo(); // Return a short summary of the object.
	virtual const string codeGetFullInfo(); // Return a complete summary of the object.
};

void DeleteMemoryAllocation(MemoryManager * memory);
void DeleteMemoryReference(MemoryManager * memory);

#endif



