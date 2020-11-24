#ifndef _SCENT_CONTROLLER_H_
#define _SCENT_CONTROLLER_H_

#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <stdio.h>

using namespace std;

// Forward declaration of scent
class Scent;

// Scent Controller class
class ScentController
{
	private:

	// Vector to hold the scents
	list<Scent*> scentData_;

	// Pointer to one and only instance of ScentController - Singleton method
	static ScentController *classInstance_;

	public:

	// Add a new scent
	void addScent(Scent *aScent) { scentData_.push_back(aScent); }

	// Delete a scent
	void deleteScent(Scent *aScent);

	// Locate a scent, by ID
	Scent * findScent(int aScentID);

	// Locate a scent, by name
	Scent * findScent(string aScentName);

	// Returns the first scent in the vector
	list<Scent*>::iterator firstScent() { return scentData_.begin(); }

	// Returns the last scent in the vector
	list<Scent*>::iterator lastScent() { return scentData_.end(); }
	
	// Returns the last scent in the vector

	//  Loads all the scents from the scent file
	bool loadScents();

	// Saves all current scents loaded into vector
	bool saveScents();

	// Obtains the current instance
	static ScentController * instance();
};

#endif
