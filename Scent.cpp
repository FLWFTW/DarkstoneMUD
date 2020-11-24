
#include "Scent.h"

using namespace std;

/* Class Functions */

Scent::Scent(int aScentID, const string & aScentName, const string & aScentDescription)
{
	setID(aScentID);
	setName(aScentName);
	setDescription(aScentDescription);
}

Scent::~Scent()
{
	setID(0);
	setName("");
	setDescription("");
}
