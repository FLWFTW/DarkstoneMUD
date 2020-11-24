




#include "connection.h"
#include "pager.h"
#include "utility.h"


// STL includes
#include <string>
using namespace std;

/*
  _____                 __                   __               ____   
 / ___/___   ___   ___ / /_ ____ __ __ ____ / /_ ___   ____  / __/___
/ /__ / _ \ / _ \ (_-</ __// __// // // __// __// _ \ / __/  > _/_ _/
\___/ \___//_//_//___/\__//_/   \_,_/ \__/ \__/ \___//_/    |_____/                                                                       
   ___            __                   __            
  / _ \ ___  ___ / /_ ____ __ __ ____ / /_ ___   ____
 / // // -_)(_-</ __// __// // // __// __// _ \ / __/
/____/ \__//___/\__//_/   \_,_/ \__/ \__/ \___//_/   
                                                     
*/


Pager::Pager()
{
	LineLimit = 24; // default
	PagerBuffer = "";
	LineCount = 0;
}
Pager::Pager(short lineLimit)
{
	LineLimit = lineLimit;
	PagerBuffer = "";
	LineCount = 0;
}
Pager::~Pager()
{
	// nothing to do
}

void Pager::SetLineLimit(short lineLimit)
{
	LineLimit = lineLimit;
}

/*
   ____ __        _           
  / __// /_ ____ (_)___  ___ _
 _\ \ / __// __// // _ \/ _ `/
/___/ \__//_/  /_//_//_/\_, / 
                       /___/  
   ___              __   _              
  / _ \ ___  __ __ / /_ (_)___  ___  ___
 / , _// _ \/ // // __// // _ \/ -_)(_-<
/_/|_| \___/\_,_/ \__//_//_//_/\__//___/
                                        
*/



void Pager::AddToBuffer(const char * text)
{
	PagerBuffer.append(text);
}

void Pager::AddToBuffer(const string * text)
{
	AddToBuffer(text->c_str());
}
void Pager::AddToBuffer(const string text)
{
	AddToBuffer(text.c_str());
}

string Pager::GetNextOutput()
{
	// If we're still waiting for user input, don't do anything
	if (LineCount >= LineLimit )
	{
		return "";
	}

	// if we're under the line limit, just send it all.
	if ( CountLines(PagerBuffer.c_str()) + LineCount < LineLimit )
	{
		LineCount += CountLines(PagerBuffer.c_str());
		string tempString;
		tempString.assign(PagerBuffer);
		PagerBuffer.assign("");
		//ReadyForNext = true;
		return tempString;
	}

	// Otherwise, send the next (LineLimit) lines of text
	string resultBuffer = "";

	while ( LineLimit > LineCount )
	{
		resultBuffer.append( RemoveLine(&PagerBuffer) );
		resultBuffer.append("\r\n"); // RemoveLine doesn't keep the newline.
		LineCount++;
	}
	resultBuffer.append("Press any key to continue... ");

	return resultBuffer;
}

bool Pager::WantsToWrite()
{
	return ( !BufferEmpty() && (LineCount <= LineLimit) );
}

bool Pager::BufferEmpty()
{
	return ( PagerBuffer.length() == 0 );
}
short Pager::BufferSize()
{
	return PagerBuffer.length();
}

void Pager::ResetLineCount()
{
	LineCount = 0;
}

void Pager::ProcessLine(const string line, SocketGeneral * socket)
{
	if (line.compare("") == 0)
		return;
	else
	{
		ResetLineCount();
		if ( socket->IsA(estConnection) )
			((SocketConnection*)socket)->SendText("\r"); // clear the (press any key) line
	}
}

