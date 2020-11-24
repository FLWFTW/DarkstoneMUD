

#ifndef __UTILITY_OBJS_HPP_
#define __UTILITY_OBJS_HPP_

#include <string>
#include <utility>
#include <functional>
#include <ctype.h> // for tolower
#include "utility.h"
using namespace std;

/*
  __  __ __   _  __ _  __       
 / / / // /_ (_)/ /(_)/ /_ __ __
/ /_/ // __// // // // __// // /
\____/ \__//_//_//_/ \__/ \_, / 
                         /___/  
 ______                   __       __          
/_  __/___  __ _   ___   / /___ _ / /_ ___  ___
 / /  / -_)/  ' \ / _ \ / // _ `// __// -_)(_-<
/_/   \__//_/_/_// .__//_/ \_,_/ \__/ \__//___/
                /_/                            
*/

template <class listType, class containedType>
bool listContains( listType & container, containedType item )
{
	typename listType::iterator itor;
	
	for (itor = container.begin(); itor != container.end(); itor++)
	{
		if ( (*itor) == item )
			return true;
	}
	return false;
}

/*
   ____                 __   _           
  / __/__ __ ___  ____ / /_ (_)___   ___ 
 / _/ / // // _ \/ __// __// // _ \ / _ \
/_/   \_,_//_//_/\__/ \__//_/ \___//_//_/
                                         
  ____   __      _            __     
 / __ \ / /     (_)___  ____ / /_ ___
/ /_/ // _ \   / // -_)/ __// __/(_-<
\____//_.__/__/ / \__/ \__/ \__//___/
           |___/                     
*/

struct fo_DeleteObject
{
	template <typename T>
		void operator() (const T* ptr) const { delete ptr; }
};

// this function object is used to sort, alphabetically,
// a collection of strings - case insensitive
class fo_StringCompare_i : binary_function <string, string, bool>
{
public:
	bool operator() (const string & firstString, const string & secondString) const
    {
		unsigned int firstLen, secondLen;
		
		if ( (firstLen = firstString.length()) == 0)
			return false;
		if ( (secondLen = secondString.length()) == 0)
			return true;

		unsigned int maxLen = MIN(firstLen, secondLen);
		unsigned int count = 0;

		while ( count < maxLen )
		{
			if ( tolower(firstString.at(count)) != tolower(secondString.at(count)) )
				return ( tolower(firstString.at(count)) < tolower(secondString.at(count)) );
			count++;
		}

		if ( firstLen == secondLen )
			return true;
		else if ( firstLen < secondLen )
			return true; // shorter comes first
		else
			return false;
	};
};

// case-independent (ci) string compare
// returns true if strings are EQUAL
struct ci_equal_to : binary_function <string, string, bool>
{
	
	struct compare_equal 
		: public binary_function <unsigned char, unsigned char,bool>
	{
		bool operator() (const unsigned char& c1, const unsigned char& c2) const
		{ return tolower (c1) == tolower (c2); }
	};	// end of compare_equal
	
	bool operator() (const string & s1, const string & s2) const
	{
		
		pair <string::const_iterator,
			string::const_iterator> result =
			mismatch (s1.begin (), s1.end (),	// source range
			s2.begin (),			  // comparison start
			compare_equal ());	// comparison
		
		// match if both at end
		return result.first == s1.end () &&
			result.second == s2.end ();
		
	}
}; // end of ci_equal_to

#endif

