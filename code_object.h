


#ifndef __CODE_OBJECT_H_
#define __CODE_OBJECT_H_

// NOT TO BE CONFUSED WITH IN-GAME OBJECTS!

// The code object interface attempts to
// standardize basic object information.
// It is used to retrieve the class of the
// object, basic information, and extended
// information.

#include <string>
using namespace std;

class iCodeObject
{
public:
	virtual ~iCodeObject() { /* empty */ }
	
	virtual const string codeGetClassName() const = 0; // Return the name of the class.
	virtual const string codeGetBasicInfo() const = 0; // Return a short summary of the object.
	virtual const string codeGetFullInfo() const = 0; // Return a complete summary of the object.
};



#endif

