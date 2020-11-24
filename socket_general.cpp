

#include <time.h>

// OS-dependent header files for socket I/O.
#ifdef unix
	#include <unistd.h>
	#include <sys/socket.h>
#else
	#ifdef WIN32
	#include <io.h>
	#include <winsock2.h>
	#endif
#endif

#include <errno.h>

#include "globals.h"
#include "World.h"

#include "socket_general.h"
#include "socket_connection.h"
#include "socket_listener.h"

#include "utility.h"

#include <list>
using namespace std;

tPointerOwner<SocketGeneral> SocketMap;


SocketGeneral::SocketGeneral()
: tManagedObject<SocketGeneral>(SocketMap)
{
	FreezeNewOutput = false;
	FreezeNewInput = false;
	RemoveNow = false;
}
SocketGeneral::~SocketGeneral()
{
}

void SocketGeneral::Disconnect()
{
	if (FileDescriptor > 0)
	{
#ifdef WIN32
		shutdown(FileDescriptor, SD_SEND);
		closesocket (FileDescriptor);
#else
		close (FileDescriptor);
#endif
	}
	FileDescriptor = -1;
	FreezeNewInput = true;
	FreezeNewOutput = true;
}

/////////////////////////////////////////////////////////

SocketManager::SocketManager( )
{
	Error = esmeNone;
}
SocketManager::~SocketManager()
{
	itorSocketId itor;
	for ( itor = Sockets.begin(); itor != Sockets.end(); itor++ )
	{
		SocketGeneral * socket = SocketMap[*itor];
		delete socket; // auto-removes from map
	}
}

bool SocketManager::PollSockets(struct timeval timeoutDelay)
{
	FD_ZERO(&in_set);
	FD_ZERO(&out_set);
	FD_ZERO(&exc_set);

	short biggestDesc = 0;

	Cleanup();

	list<idSocket> localList = Sockets;
	itorSocketId itor;

	// Construct the list of descriptors to check.
	for (itor = localList.begin(); itor != localList.end(); itor++)
	{
		SocketGeneral * socket = SocketMap[*itor];

		if ( !socket )
		{
			// Do not set into the FdSets if it's not
			// allocated anymore.
			Sockets.remove(*itor); // *itor is ID, socket is pointer
			continue;
		}

		short desc = socket->GetDescriptor();

		// A negative desc can happen if the socket was
		// disconnected, or closed temporarily, or something...
		// also, don't add it if we want to get rid of it now
		if (desc == -1 || socket->RemoveNow) continue;

		if ( !socket->FreezeNewInput )
			FD_SET(desc, &in_set);
		if ( socket->WantsToWrite() )
			FD_SET(desc, &out_set);

		FD_SET(desc, &exc_set);

#ifdef unix
		// On windows, we don't need to bother with this.
		biggestDesc = MAX(desc, biggestDesc);
#endif
	}

	
	if ( select(biggestDesc + 1, &in_set, &out_set, &exc_set, &timeoutDelay) < 0 )
	{
		switch (errno)
		{
			case EBADF:
				printf("A bad descriptor was given to select.\n");
				break;
			case EINTR:
				printf("A bad descriptor was given to select.\n");
				break;
		}
		Error = esmeCouldntSelect;

		return false;
	}

	Cleanup();
	

	return true;
}

void SocketManager::ForceFlushOutput()
{
	FD_ZERO(&in_set);
	FD_ZERO(&exc_set);

	bool found = false;
	short iterations = 0;

	do
	{
		found = false;
		struct timeval timeoutDelay;
		timeoutDelay.tv_sec = 1;
		timeoutDelay.tv_usec = 0;
		FD_ZERO(&out_set);
		
		itorSocketId itor;

		short biggestDesc = 0;

		// Construct the list of descriptors to check.
		for (itor = Sockets.begin(); itor != Sockets.end(); itor++)
		{
			SocketGeneral * socket = SocketMap[*itor];
			if ( !socket )
				continue;

			short desc = socket->GetDescriptor();
			if (desc == -1) continue;

			if ( socket->IsA(estConnection) && socket->WantsToWrite() )
			{
				found = true;
				FD_SET(desc, &out_set);
			}

			#ifdef unix
				// On windows, we don't need to bother with this.
				biggestDesc = MAX(desc, biggestDesc);
			#endif
		}
	
		if ( select(biggestDesc + 1, NULL, &out_set, NULL, &timeoutDelay) < 0 )
		{
			Error = esmeCouldntSelect;
			return;
		}

		this->ProcessActiveSockets();
		iterations++;

	} while (found && iterations < 60); // one minute at most
	
}

void SocketManager::RemoveSocket(idSocket id, bool removeNow)
{
	if ( id == 0 )
		return;

	// "Further" remove the socket only if it's a valid reference
	SocketGeneral * socket = SocketMap[id];

	if ( socket )
		this->RemoveSocket(socket, removeNow);
	else
		toDelete.push_back( id );
}

void SocketManager::RemoveSocket(SocketGeneral * socket, bool removeNow)
{
	if ( !socket )
		return;

	//gTheWorld->LogString("Removing socket: " + socket->codeGetFullInfo());

	toDelete.push_back( socket->GetId() );

	socket->FreezeNewInput = true;
	socket->FreezeNewOutput = true;
	socket->RemoveNow = removeNow;
}
void SocketManager::Cleanup()
{
	if (toDelete.empty())
		return;

	list<idSocket> localToDelete = toDelete;

	itorSocketId itor;

	for (itor = localToDelete.begin(); itor != localToDelete.end(); itor++)
	{
		SocketGeneral * socket = SocketMap[*itor];
		
		if ( !socket )
		{
			gTheWorld->LogBugString("A null socket was found during cleanup... removing from lists.");

			Sockets.remove(*itor);
			toDelete.remove(*itor);
			continue; // go to next in loop
		}

		if ( !(socket->WantsToWrite()) || socket->RemoveNow || socket->IsWriteTimingOut() )
		{
			// delete the socket if it's marked for immediate deletion,
			// or if there's nothing left to write
			socket->Disconnect();

			Sockets.remove(*itor);
			toDelete.remove(*itor);

			delete socket; // auto-removes from the map
		}
	}
}
void SocketManager::AddWaiting()
{
	if (toAdd.empty())
		return;
	
	itorSocketId itor;

	for (itor = toAdd.begin(); itor != toAdd.end(); itor++)
	{
		SocketGeneral * socket = SocketMap[*itor];

		if ( !socket )
			continue; // abort... but this should never ever happen

		Sockets.push_back(*itor);

		if ( socket->IsA(estConnection) )
			((SocketConnection*) socket)->Initialize(); // initialize the socket
	}
	toAdd.clear();
}

void SocketManager::ProcessLine(string line, SocketGeneral * socket)
{
	if (line.compare("shutdown") == 0)
	{
		printf("Received shutdown order.\n");
		gGameRunning = false;
	}
	else if (line.compare("quit") == 0)
	{
		printf("Received quit request from socket!\n");
		RemoveSocket(socket);
	}
	else
	{
		printf("String: %s. Length: %d.\n",line.c_str(), line.length() );
	}

	itorSocketId itor;
	for (itor = Sockets.begin(); itor != Sockets.end(); itor++)
	{
		SocketGeneral * socket = SocketMap[*itor];
		if (!socket)
			continue;

		if (socket->IsA(estConnection))
		{
			line.append("\n");
			( (SocketConnection *) socket )->SendText(line);
		}
	}
}

bool SocketManager::ProcessActiveSockets()
{
	// Bail out just in case there are no sockets to check
	if (Sockets.empty())
		return true;

	itorSocketId itor;

	// Loop through list of sockets.
	for (itor = Sockets.begin(); itor != Sockets.end(); itor++)
	{
		SocketGeneral * socket = SocketMap[*itor];

		if ( !socket )
		{
			gTheWorld->LogBugString("A null socket was found during socket processing!");
			continue;
		}

		int desc = socket->GetDescriptor();
		if ( desc == -1) continue;

		if (FD_ISSET(desc, &exc_set))
		{
			FD_CLR( desc, &in_set );
			FD_CLR( desc, &out_set );
			socket->InExcSet();

			this->RemoveSocket(socket, true);


			if (socket->IsA(estListener))
				gTheWorld->LogBugString("Exception found and disposed of on listener!\n");
			else
				gTheWorld->LogBugString("Exception found and disposed of!\n");

			continue;
		}

		if (FD_ISSET(desc, &out_set))
		{
			if ( !socket->InOutSet() )
			{
				// Something went wrong.
				RemoveSocket(socket, true);
				continue;
			}
		}

		if (FD_ISSET(desc, &in_set))
		{
			if ( !socket->InInSet() )
			{
				// Something went wrong. Dispose of the socket, and move on.
				RemoveSocket(socket, true);
				continue;
			}
		}

		if ( socket->IsA(estConnection))
		{
			if ( ((SocketConnection*) socket )->HasInputBuffer() )
			{
				((SocketConnection*) socket )->ProcessInputBuffer();
			}
		}
	}

	// The listener adds sockets to the waiting list...
	// Add them to the main list here.
	AddWaiting();

	Cleanup();

	return true;
}


int SocketManager::CreateListener(int port)
{
	SocketListener * listener = new SocketListener(this);

	if (!listener->StartListening(port))
	{
		switch (listener->GetLastError())
		{
			case esleCreateSocket:
				printf("Couldn't create socket.");
				break;
			case esleSocketOptions:
				printf("Couldn't set options.");
				break;
			case esleCouldntBind:
				printf("Couldn't bind.");
				break;
			case esleCouldntListen:
				printf("Couldn't listen.");
				break;
			default:
				break;
		}
		return -1;
	}

	Sockets.push_front( listener->GetId() );

	return listener->GetDescriptor();

}

bool SocketManager::AddListener(int fileDescriptor)
{
	SocketListener * listener = new SocketListener(this);
	listener->SetDescriptor(fileDescriptor);
	Sockets.push_front( listener->GetId() );
	return true;
}

void SocketManager::SendToAll(string line)
{
	itorSocketId itor;

	for (itor = Sockets.begin(); itor != Sockets.end(); itor++)
	{
		SocketGeneral * socket = SocketMap[*itor];
		if ( socket && socket->IsA(estConnection))
			((SocketConnection *) socket)->SendText(line);
	}
}


#ifdef WIN32
// Function to initialize Winsock.
bool InitializeWinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	
	wVersionRequested = MAKEWORD( 2, 2 );
	
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL. 								 */
		return false;
	}
	
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater	 */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we 	 */
	/* requested.										 */
	
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL. 								 */
		WSACleanup( );
		return false; 
	}
	
	/* The WinSock DLL is acceptable. Proceed. */
	return true;
}
#endif // win32





