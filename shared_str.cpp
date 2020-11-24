
#include "mud.h"

#include <sharedstr_hashtable.h>

SharedString::HashTable * gSharedStringTable;

// Associate manager class
ASSOCIATE_MANAGER_WRAPPER(HashTableWrapper, gSharedStringTable);


size_t SmaugHash(const char * str)
{
	return strlen(str);
}

size_t BetterHash(const char * str)
{
	int i = strlen (str);
	switch (i) {
		case 0:
			return 0;
		case 1:  // hash on the character
			return *str;
		default: 
			/* 
			 * hash on first two characters and length. 
			 * Ensure that "ab" hashes differently than "ba"
			 * Ignore overflows
			 */   
			return (((str[0] & 0xff) << 8) | (str[1] & 0xff)) * i;
	}
}

void InitializeSharedStrings()
{
	gSharedStringTable = new SharedString::HashTable(&SmaugHash);
}

void CloseSharedStrings()
{
	delete gSharedStringTable;
	gSharedStringTable = NULL;
}


