

#ifndef __INPUTHANDLER_H_
#define __INPUTHANDLER_H_

#include <stdio.h>

#include <string>
using namespace std;

class SocketGeneral;

class InputHandler
{
public:
	virtual ~InputHandler() { /* empty*/ }
	virtual void ProcessLine(const string line, SocketGeneral * socket = NULL ) = 0;
};

#endif



