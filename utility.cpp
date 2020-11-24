


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "utility.h"
#include "utility_objs.hpp"
#include "globals.h"

#include "shared_str.h"

void	bug		( const char *str, ... );

// STL includes
#include <string>
#include <list>
using namespace std;


uint64 unique (void)
{
	static uint64 iNextUnique = 1;
	return iNextUnique++;
} // end of unique


/*
 * Compare strings, case insensitive.
 * Return true if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
	if ( !astr )
	{
		bug( "Str_cmp: null astr." );
		if ( bstr )
			fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
		return true;
	}
	
	if ( !bstr )
	{
		bug( "Str_cmp: null bstr." );
		if ( astr )
			fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
		return true;
	}
	
#if 0 /* BUG found by Testaur */
	for ( ; *astr || *bstr; astr++, bstr++ )
	{
		if ( tolower(*astr) != tolower(*bstr) )
			return true;
	}
#endif
	for ( ; tolower(*astr) == tolower(*bstr); astr++, bstr++ )
	{
		if(*astr=='\0')
			return false;
	}
	
	return true;
}

// compare strings for equality using the binary function above
// returns true is s1 == s2
bool ciStringEqual (const string & s1, const string & s2)
{
	return ci_equal_to () (s1, s2);
}  // end of ciStringEqual



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return true if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
	if ( !astr )
	{
		bug( "Strn_cmp: null astr." );
		return true;
	}
	
	if ( !bstr )
	{
		bug( "Strn_cmp: null bstr." );
		return true;
	}
	
	for ( ; *astr; astr++, bstr++ )
	{
		if ( tolower(*astr) != tolower(*bstr) )
			return true;
	}
	
	return false;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns true is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
	int sstr1;
	int sstr2;
	int ichar;
	char c0;
	
	if ( ( c0 = tolower(astr[0]) ) == '\0' )
		return false;
	
	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);
	
	for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
		if ( c0 == tolower(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
			return false;
		
		return true;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return true if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
	int sstr1;
	int sstr2;
	
	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);
	if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
		return false;
	else
		return true;
}



/*
 * Returns an initial-capped string.
 */
const char *capitalize( const char *str )
{
	static char strcap[MAX_STRING_LENGTH];
	int i;
	
	for ( i = 0; str[i] != '\0'; i++ )
		strcap[i] = tolower(str[i]);
	strcap[i] = '\0';
	strcap[0] = toupper(strcap[0]);
	return strcap;
}

shared_str capitalize_first( const shared_str & str )
{
	if ( str.length() == 0 )
		return str;

	string s = str.str();
	s[0] = toupper(s[0]);
	return shared_str( s );
}

shared_str lowercase_first( const shared_str & str )
{
	if ( str.length() == 0 )
		return str;

	string s = str.str();
	s[0] = tolower(s[0]);
	return shared_str( s );
}

/*
 * Returns a lowercase string.
 */
const char *strlower( const char *str )
{
	static char strlow[MAX_STRING_LENGTH];
	int i;
	
	for ( i = 0; str[i] != '\0'; i++ )
		strlow[i] = tolower(str[i]);
	strlow[i] = '\0';
	return strlow;
}

/*
 * Returns an uppercase string.
 */
const char *strupper( const char *str )
{
	static char strup[MAX_STRING_LENGTH];
	int i;
	
	for ( i = 0; str[i] != '\0'; i++ )
		strup[i] = toupper(str[i]);
	strup[i] = '\0';
	return strup;
}

/*
 * Returns true or false if a letter is a vowel			-Thoric
 */
bool isavowel( const char letter )
{
	char c;
	
	c = tolower( letter );
	if ( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
		return true;
	else
		return false;
}




/*
 * Count the number of lines in a string             - Ksilyan
 * (both char * and string versions)
 */

short CountLines(const string text)
{
	return CountLines( text.c_str() );
}
short CountLines(const string * text)
{
	return CountLines( text->c_str() );
}
short CountLines(const char * text)
{
	short count = 0;
	int length = strlen(text);
	int pos;
	
	for (pos = 0; pos < length; pos++)
	{
		if (text[pos] == '\n')
			count++;
	}

	//printf("CountLines found %d lines for string:\n%s\n-------\n", count, text);
	return count;
}


// Get a line, remove it, and update source
string RemoveLine(string * source)
{
	unsigned int inputBufferPos;
	unsigned int lineBufferPos;
	unsigned int length;
	length = source->length();
	if ( length == 0 )
		return ""; // if it's empty, return a blank string

	const char * pInputBuffer = source->c_str();

	char lineBuffer[MAX_STRING_LENGTH];

	for ( inputBufferPos = 0, lineBufferPos = 0; inputBufferPos < length; inputBufferPos++ )
	{
		// leave one off for final 0
		if ( lineBufferPos >= MAX_STRING_LENGTH - 1 )
		{
			// this is a problem...
			lineBuffer[lineBufferPos] = '\0';
			string resultBuf;
			resultBuf.assign(lineBuffer);

			// Remove all processed input buffer.
			if (inputBufferPos >= length)
				source->assign("");
			else
				source->assign(source->substr(inputBufferPos));

			return resultBuf;
		}
		
		if ( pInputBuffer[inputBufferPos] == '\n' || pInputBuffer[inputBufferPos] == '\r' )
		{
			inputBufferPos++;
			if ( inputBufferPos < length )
			{
				// there's at least one character left. Make sure it's not a \n or \r
				if ( pInputBuffer[inputBufferPos] == '\n' || pInputBuffer[inputBufferPos] == '\r' )
					// It is... so we need to cut off that character.
					inputBufferPos++;
			}

			if (lineBufferPos == 0)
			{
				strcpy(lineBuffer, "");
			}
			else
			{
				lineBuffer[lineBufferPos] = '\0'; // finish off the string
			}
			
			// Remove all processed input buffer.
			if (inputBufferPos >= length)
				source->assign("");
			else
				source->assign(source->substr(inputBufferPos));

			string resultBuf;
			resultBuf.assign(lineBuffer);
			return resultBuf;
		}
		else /*if ( isascii(pInputBuffer[inputBufferPos]) && isprint(pInputBuffer[inputBufferPos]) )*/
			lineBuffer[lineBufferPos++] = pInputBuffer[inputBufferPos];
	}

	return "";
}

///////////////////////////////////////////

string CenterString(const string & toCenter, unsigned short width)
{
	unsigned int length;
	unsigned short i;
	string result;

	// If there is no string, or if it's greater than
	// width anyways, then there is nothing to do.
	if ( toCenter.length() == 0 )
		return toCenter;

	length = 0;

	// Do not count color characters in the length
	for (i = 0; i < toCenter.length(); i++)
	{
		char c = toCenter.at(i);
		if (c == '&' && i < toCenter.length() - 1 && toCenter.at(i+1) != '&')
			i++; // skip this character, and the one after it
		else if (c == '^' && i < toCenter.length() - 1 && toCenter.at(i+1) != '^')
			i++; // skip this character, and the one after it
		else
			length++;
	}

	// If, after colors have been removed, the string is
	// >= width, then don't bother processing it
	if ( length >= width )
		return toCenter;

	unsigned short newLength = 0;

	// Leading spaces
	for (i = 0; i < width/2 - length/2; i++)
	{
		result.append(" ");
		newLength++;
	}

	result.append(toCenter);
	newLength += length;
	
	// Trailing spaces
	while (newLength < width)
	{
		result.append(" ");
		newLength++;
	}

	return result;
}

bool IsPrefixOf(const std::string & prefix, const std::string & str)
{
    if (prefix.size() > str.size())
        return false;

    // Check that each character of the prefix matches the corresponding
    // character in the larger string.
    for (std::string::size_type pos = 0; pos < prefix.length(); pos++)
    {
        if (prefix[pos] != str[pos])
            return false;
    }

    return true;
}

bool IsPrefixOf(const shared_str & prefix, const shared_str & str)
{
    return IsPrefixOf(prefix.str(), str.str());
}

const char *strip_color_codes( const char *c )
{
   static char buf[MAX_STRING_LENGTH];
   int i,p;
   if( !c )
      return NULL;
   if( strlen(c)>MAX_STRING_LENGTH ) //don't bust that buffer!
      return NULL;
   for( p = i = 0; i < strlen(c); i++, p++ )
   {
      top:
      if( c[i] == '&' || c[i] == '^' )
      {
         i++;
         if( c[i] != '&' && c[i] != '^' )
            i++;
         goto top;
      }
      buf[p] = c[i];
   }
   return buf;
}