



#ifndef __SOCKET_GENERAL_H_
#define __SOCKET_GENERAL_H_


#ifdef unix
#include <sys/socket.h>
#include <sys/time.h>
#else
#include <winsock2.h>
#include <time.h>
#endif


#include <list>
using namespace std;

#include "InputHandler.h"

#include "managed_objects.hpp"
#include "code_object.h"


class SocketGeneral;
extern tPointerOwner<SocketGeneral> SocketMap; // socket_general.cpp

typedef tId<SocketGeneral> idSocket;

typedef list<idSocket>::iterator itorSocketId;
typedef list<idSocket>::reverse_iterator ritorSocketId;

///////////////////////

enum eSocketTypes {
	estListener, estConnection
};

class SocketGeneral : public tManagedObject<SocketGeneral>, public iCodeObject
{
protected:
	int FileDescriptor; // the descriptor of the socket.

public:
	SocketGeneral();
	virtual ~SocketGeneral();

	int GetDescriptor() const { return FileDescriptor; }

	virtual void Disconnect(); // Disconnect the socket.

	bool FreezeNewOutput; // prevent further adding of output
	bool FreezeNewInput; // prevent further reception of output
	bool RemoveNow; // used during deletion: if false, will try to empty buffer first.

	virtual bool IsA(eSocketTypes type) const = 0;

	virtual bool WantsToWrite() = 0;

	virtual bool IsWriteTimingOut() = 0;

	virtual bool InOutSet() = 0;
	virtual bool InInSet() = 0;
	virtual bool InExcSet() = 0;

	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName() const = 0; // Return the name of the class.
	virtual const string codeGetBasicInfo() const = 0; // Return a short summary of the object.
	virtual const string codeGetFullInfo() const = 0; // Return a complete summary of the object.
};

///////////////////////

enum eSocketManagerErrors {
	esmeNone, esmeCouldntSelect
};

class SocketManager : public InputHandler
{
	friend class SocketListener;

protected:
	fd_set		    in_set;     // Set of descriptors for reading
	fd_set		    out_set;    // Set of descriptors for writing
	fd_set		    exc_set;    // Set of descriptors with errors

	eSocketManagerErrors Error;

	list<idSocket> toDelete; // temporary list of sockets waiting for deletion
	void Cleanup(); // delete sockets from list

	list<idSocket> toAdd; // temporary list of sockets waiting to be added to main list
	void AddWaiting(); // add waiting sockets

	/*
	 * We add sockets to the cleanup list and only remove them
	 * after we finish iterating through the main list, because
	 * if they are deleted during the loop the itor becomes bogus.
	 */

	// List of sockets managed
	list<idSocket> Sockets;

public:
	SocketManager();
	virtual ~SocketManager();

	eSocketManagerErrors GetLastError() { return Error; }

	bool PollSockets(struct timeval timeoutDelay);
	bool ProcessActiveSockets();

	virtual void ForceFlushOutput(); // force flush of everyone's output buffer.

	virtual void RemoveSocket(idSocket id, bool removeNow = false); // add a socket to the cleanup list.
	virtual void RemoveSocket(SocketGeneral * socket, bool removeNow = false); // add a socket to the cleanup list.

	int CreateListener(int port); // create a listener and return its descriptor
	bool AddListener(int fileDescriptor); // add a listener that's already been created

	// Input Handler:
	// send text to all descriptors
	virtual void ProcessLine(string line, SocketGeneral * socket);

	virtual void SendToAll(string line);

	// Accessors
	list<idSocket>::iterator GetSocketsBegin() { return Sockets.begin(); }
	list<idSocket>::iterator GetSocketsEnd() { return Sockets.end(); }
};

#ifdef WIN32
// Define winsock init function.
bool InitializeWinsock();
#endif

#endif


