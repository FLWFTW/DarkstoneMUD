
/**********************************************
 * The connection socket is used for I/O
 * between the client and the server. It has
 * interfaces for read/writing data.
 *
 * Like the listener socket, the interface is
 * as OS-independent as possible, but the code
 * implementation is (a fortiori) very
 * OS-dependent.
 *
 * (c) 2003 David "Ksilyan" Haley
 **********************************************/

#ifndef __SOCKET_CONNECTION_H_
#define __SOCKET_CONNECTION_H_

#include <string>
using namespace std;

#include "socket_general.h"
#include "InputHandler.h"

class SocketConnection : public SocketGeneral
{
protected:
	short Port;
	string Host;
	string Address;

	string OutputBuffer; // buffer waiting to be written
	short OutputLength; // quick access to length

	

	string InputLine; // line buffer read from InputBuffer
	string LastLine; // last line received
	short RepeatTimes; // how many repetitions

	InputHandler * InputReceiver;
	inline void SendLineToReceiver(string line) { InputReceiver->ProcessLine(line, this); }

public:
	string InputBuffer; // buffer that has been read
	// Class constructor.
	SocketConnection();
	// Create a new socket connection to a given descriptor.
	SocketConnection(int descriptor);
	// Destroy the descriptor.
	virtual ~SocketConnection();

	virtual void Initialize(); // Do whatever setup is needed.

	virtual bool IsA(eSocketTypes type) const { return (type == estConnection); }

	void SetPort(int port) { Port = port; }
	short GetPort() const { return Port; }

	void SetHost(string host) { Host = host; }
	const char * GetHost() const { return Host.c_str(); }
	const string & GetHostString() const { return Host; }

	void SetAddress(string address) { Address = address; }
	void SetInputReceiver(InputHandler * receiver) { InputReceiver = receiver; }

	// defaults to writing buffer to the descriptor.
	virtual bool InOutSet();
	// defaults to reading from descriptor.
	virtual bool InInSet();
	// defaults to . . . not sure.
	virtual bool InExcSet();

	bool FindLine(); // try to move one line into InputLine - true if line found
	virtual void ProcessInputBuffer();

	virtual bool IsWriteTimingOut() { return false; }

	virtual bool WantsToWrite() { return ( OutputLength > 0 ); }
	virtual bool HasInputBuffer() { return (InputBuffer.length() > 0); }

	// Send text to the descriptor.
	virtual void SendText(string text, unsigned int textLength = 0)
	{
		OutputBuffer.append(text); OutputLength += text.length();
	}

	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName(); // Return the name of the class.
	virtual const string codeGetBasicInfo(); // Return a short summary of the object.
	virtual const string codeGetFullInfo(); // Return a complete summary of the object.
};



#endif




