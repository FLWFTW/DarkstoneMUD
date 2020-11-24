#include "mud.h"
#include "ScentController.h"
#include "Scent.h"

// Null out the class instance
ScentController * ScentController::classInstance_(NULL);

// Location of scent file
const char * SCENTFILE = "../scents/scent.list";

ScentController * ScentController::instance()
{
	if ( !classInstance_ )
	{
		classInstance_ = new ScentController;
	}

	return classInstance_;
}

void ScentController::deleteScent(Scent *aScent)
{
	// Scent iterator
	list<Scent*>::iterator it;

	// Loop through scent vector
	for ( it = scentData_.begin(); it != scentData_.end(); ++it )
	{
		// If the current scent is equal to the one we want
		if ( *it == aScent )
		{
			// Remove it from the vector
			scentData_.erase(it);

			// Delete it
			delete *it;

			// and return
			return;
		}
	}

	return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

bool ScentController::loadScents()
{
	// Load and save written SMAUG style

	FILE *inFile;
	char buf[MAX_STRING_LENGTH];
	int scentID = 0;
	char * scentName;
	char * scentDesc;
	const char * word;
	bool fMatch;
	Scent * newScent = NULL;

	// Open the scent file
	if ( ( inFile = fopen(SCENTFILE, "r") ) == NULL )
	{
		return false;
	}

	for ( ;; )
	{
		word = feof( inFile ) ? "End" : fread_word(inFile);
		fMatch = false;

		switch(UPPER(word[0]))
		{
			case '#':
			{

				// A new scent, null out the class pointer			
				if ( !str_cmp(word, "#SCENT"))
				{
					newScent = NULL;
					fMatch = true;
					break;
				}
	
				// Finished reading one scent, create the class
				if ( !str_cmp(word, "#ENDSCENT") ) 
				{
					newScent = new Scent(scentID, scentName, scentDesc);
					addScent(newScent);
					fMatch = true;
					break;
				}
			}

			case 'E':
			{
				if ( !str_cmp(word, "End" ) )
				{
					// All done
					return true;
				}
			}

			case 'I':
			{
				KEY("ID", scentID, fread_number(inFile));
			}

			case 'N':
			{
				KEY("Name", scentName, fread_string(inFile));
			}

			case 'D':
			{
				KEY("Desc", scentDesc, fread_string(inFile));
			}
		}

		if ( !fMatch )
		{
			sprintf(buf, "ScentController::loadscents: no match for %s", word );
			bug(buf, 0);
		}
	}
	

	fclose(inFile);

	return true;
}

bool ScentController::saveScents()
{
	FILE *outFile;
	list<Scent*>::iterator it;

	if ( ( outFile = fopen(SCENTFILE, "w" ) ) == NULL )
	{
		return false;
	}

	for ( it = scentData_.begin(); it != scentData_.end(); ++it)
	{
		fprintf(outFile, "#SCENT\n");
		fprintf(outFile, "ID	%i\n", (*it)->getID());
		fprintf(outFile, "Name	%s~\n", (*it)->getName().c_str());
		fprintf(outFile, "Desc	%s~\n", (*it)->getDescription().c_str());
		fprintf(outFile, "#ENDSCENT\n");
	}

	fprintf(outFile, "End\n");

	fclose(outFile);

	return true;
}

Scent * ScentController::findScent(int aScentID)
{
	list<Scent*>::iterator it;

	for ( it = scentData_.begin(); it != scentData_.end(); ++it )
	{
		if ((*it)->getID() == aScentID )
		{
			return *it;
		}
	}

	return NULL;
}

Scent * ScentController::findScent(string aScentName)
{
        list<Scent*>::iterator it;

        for ( it = scentData_.begin(); it != scentData_.end(); ++it )
        {
                if ( !str_prefix((*it)->getName().c_str(), aScentName.c_str()) )
                {
                        return *it;
                }
        }	

	return NULL;
}
