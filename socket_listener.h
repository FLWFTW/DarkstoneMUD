

/**********************************************
 * The listener socket is the one in charge
 * of accepting new connections. When it is
 * polled, it returns a new socket connection
 * which can then be used as an I/O device.
 *
 * This interface is not OS-dependent, but the
 * .cpp code is very likely to be extremely
 * OS-dependent.
 *
 * (c) 2003 David "Ksilyan" Haley
 **********************************************/

#ifndef __SOCKET_LISTENER_H_
#define __SOCKET_LISTENER_H_

#include "socket_general.h"
#include "globals.h"

enum eSocketListenerErrors {
	esleNone, esleNotReady, esleCreateSocket, esleSocketOptions, esleCouldntBind,
	esleCouldntListen, esleCouldntAccept
};

#ifdef KMUD
	typedef class SocketConnection NEW_SOCKET_CONNECTION_TYPE;
#else
	typedef class PlayerConnection NEW_SOCKET_CONNECTION_TYPE;
#endif

class SocketListener : public SocketGeneral
{
private:
	int Port;
	bool Ready;

	SocketManager * Manager; // object to send incomings to.

	eSocketListenerErrors Error;

public:
	SocketListener( SocketManager * manager );
	~SocketListener();

	virtual bool IsA(eSocketTypes type) const { return (type == estListener); }

	// Begin listening on a given port number.
	bool StartListening(int port);

	void SetDescriptor(int descriptor) { FileDescriptor = descriptor; Ready = true; Port = -1; }

	// Accept new connections.
	bool InInSet();
	bool InOutSet() { return true; /* do nothing */ }
	bool InExcSet() { return true; /* do nothing */ }

	short GetPort() const { return Port; }

	virtual bool WantsToWrite() { return false; }
	bool IsWriteTimingOut() { return false; }

	// Return the error number.
	eSocketListenerErrors GetLastError();

	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName() const; // Return the name of the class.
	virtual const string codeGetBasicInfo() const; // Return a short summary of the object.
	virtual const string codeGetFullInfo() const; // Return a complete summary of the object.
};


#endif // include guard

