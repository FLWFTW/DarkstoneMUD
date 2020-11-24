



#ifndef __PAGER_H_
#define __PAGER_H_

#include "InputHandler.h"

// STL includes
#include <string>
using namespace std;

class Pager : public InputHandler
{
protected:
	string PagerBuffer; // buffer waiting to be sent
	short LineLimit; // how many lines to limit display to
	short LineCount; // how many lines have been sent already
	//bool ReadyForNext; // are we ready to display the next page

public:
	Pager();
	Pager(short lineLimit);
	virtual ~Pager();

	void SetLineLimit(short lineLimit);

	string GetNextOutput(); // get the output waiting, or blank if awaiting user input

	void AddToBuffer(const string text);
	void AddToBuffer(const string * text);
	void AddToBuffer(const char * text);

	void ResetLineCount();

	bool WantsToWrite();
	bool BufferEmpty();
	short BufferSize();

	virtual void ProcessLine(const string line, SocketGeneral * socket = NULL ); // user input
};




#endif

