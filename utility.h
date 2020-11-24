
/**********************************************
 * This header file contains utility macros &
 * other functions.
 *
 * (c) 2003 David "Ksilyan" Haley
 **********************************************/


#ifndef __UTILITY_H_
#define __UTILITY_H_

// STL includes
#include <string>
using namespace std;

// forward decl
#include <sharedstr.hpp>
struct HashTableWrapper;
typedef SharedString::SharedString< HashTableWrapper > shared_str;

// Mins, maxes, ranges

#define MIN(a, b) ( (a) > (b) ? (b) : (a) )
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )

// is X inside [lower bound, upper bound]
#define IN_RANGE(X, lb, ub) (  ( (X >= lb) && (X <= ub) ) ? true : false  )

// String utility functions
bool str_cmp( const char *astr, const char *bstr );
bool ciStringEqual (const string & s1, const string & s2);
bool str_prefix( const char *astr, const char *bstr );
bool str_infix( const char *astr, const char *bstr );
bool str_suffix( const char *astr, const char *bstr );
const char *capitalize( const char *str );
shared_str capitalize_first( const shared_str & str );
shared_str  lowercase_first( const shared_str & str );
const char *strlower( const char *str );
const char *strupper( const char *str );
bool isavowel( const char letter );
const char *strip_color_codes( const char *c );

short CountLines(const string text);
short CountLines(const string * text);
short CountLines(const char * text);

string GetLine(const string source);
string GetLine(const string * source);
string GetLine(const char * source);

string RemoveLine(string * source); // Get a line, remove it, and update source

// Take a string, and return a new version centered in width characters.
string CenterString(const string & toCenter, unsigned short width);

/* Function: IsPrefixOf
 *
 * Check if the first argument is a prefix of the second.
 *
 * Parameters:
 *  prefix - The presumed prefix.
 *  str    - The string to search in.
 *
 * Returns:
 *  True if the first argument is a prefix of the second.
 */
bool IsPrefixOf(const std::string & prefix, const std::string & str);

/* Function: IsPrefixOf
 *
 * Check if the first argument is a prefix of the second. This version
 * uses shared strings.
 *
 * Parameters:
 *  prefix - The presumed prefix.
 *  str    - The string to search in.
 *
 * Returns:
 *  True if the first argument is a prefix of the second.
 */
bool IsPrefixOf(const shared_str & prefix, const shared_str & str);

// TODO: make case-insensitive version of IsPrefixOf

#endif


