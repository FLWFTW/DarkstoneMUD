
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// OS-dependent header files for socket I/O.
#ifdef unix
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/in_systm.h>
	#include <netinet/ip.h>
	#include <arpa/inet.h>
	#include <arpa/telnet.h>
	#include <netdb.h>
#else
	#include <io.h>
	#include <winsock2.h>
#endif

#include "globals.h"
#include "socket_listener.h"
#include "socket_connection.h"
#include "utility.h"

// STL includes
#include <iostream>
#include <sstream>
#include <string>

#include "connection.h"

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

SocketListener::SocketListener( SocketManager * manager )
{
	Error = esleNone;
	Ready = false;
	Port = 0;
	FileDescriptor = 0;
	Manager = manager;
}
SocketListener::~SocketListener()
{
	Disconnect();
	printf("Deleting listener socket.\n");
}

/*
   ___        __                     _           
  / _ \ ___  / /  __ __ ___ _ ___ _ (_)___  ___ _
 / // // -_)/ _ \/ // // _ `// _ `// // _ \/ _ `/
/____/ \__//_.__/\_,_/ \_, / \_, //_//_//_/\_, / 
                      /___/ /___/         /___/  

*/

eSocketListenerErrors SocketListener::GetLastError()
{
	return Error;
}


bool SocketListener::InInSet()
{
	if (!Ready)
	{
		Error = esleNotReady;
		return false;
	}

    char buf[MAX_STRING_LENGTH];
    struct sockaddr_in sock;
    struct hostent * from;
    int desc;

#ifdef unix
    socklen_t size;
#else
	int size;
#endif

	size = sizeof(sock);

	if ( ( desc = accept( FileDescriptor, (struct sockaddr *) &sock, &size) ) < 0 )
    {
		/* According to the Linux manpage for accept:
		 * ERROR HANDLING
		 * Linux accept passes already-pending network errors on the new socket as
		 * an error code from accept.	 This  behaviour  differs  from  other	BSD
		 * socket	implementations.  For reliable operation the application should
		 * detect the network errors defined for the  protocol  after	accept	and
		 * treat  them  like EAGAIN by retrying. In case of TCP/IP these are ENET-
		 * DOWN, EPROTO, ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH, EOPNOTSUPP,
		 * and ENETUNREACH.
		 *****************************
		 * So, it seems wise to me to do like what SMAUG did and just say "whatever",
		 * and if there's an error just ignore it.
		 */

		return true;

		/* Old error handling code...
		// Error...
		Error = esleCouldntAccept;
		return false;
		*/
	}

	SocketConnection * newConnection = new NEW_SOCKET_CONNECTION_TYPE(desc);

	newConnection->SetPort(ntohs(sock.sin_port));

	strcpy( buf, inet_ntoa( sock.sin_addr ) );
	//printf("Incoming connection from %s.\n\n", buf);
	newConnection->SetAddress(buf);
	
	from = gethostbyaddr( (char *) &sock.sin_addr, sizeof(sock.sin_addr), AF_INET );
	newConnection->SetHost( (from ? from->h_name : buf) );

	newConnection->SetInputReceiver(Manager);

	Manager->toAdd.push_back( newConnection->GetId() );

	return true;
}

bool SocketListener::StartListening(int port)
{
	Port = port;

	char hostname[64];
	struct sockaddr_in	 sa;
	struct hostent * hp;
	struct servent * sp;
		
	gethostname(hostname, sizeof(hostname));
	
	
	if ( ( FileDescriptor = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		Error = esleCreateSocket;
		return false;
	}
	
	int value = 1;
	if ( setsockopt( FileDescriptor, SOL_SOCKET, SO_REUSEADDR,
		(char *) &value, sizeof(value) ) < 0 )
	{
		Error = esleSocketOptions;
		close( FileDescriptor );
		return false;
	}

	
#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct	linger	ld;
		
		ld.l_onoff	= 1;
		ld.l_linger = 1000;
		
		if ( setsockopt( FileDescriptor, SOL_SOCKET, SO_DONTLINGER,
			(char *) &ld, sizeof(ld) ) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			close( FileDescriptor );
			exit( 1 );
		}
	}
#endif
	
	hp = gethostbyname( hostname );
	sp = getservbyname( "service", "mud" );
	memset(&sa, '\0', sizeof(sa));
	sa.sin_family	= AF_INET; /* hp->h_addrtype; */
	sa.sin_port 	= htons( port );
	/*	  sa.sin_addr.s_addr = inet_addr("205.243.76.36"); */
	
	if ( bind( FileDescriptor, (struct sockaddr *) &sa, sizeof(sa) ) == -1 )
	{
		Error = esleCouldntBind;
		close( FileDescriptor );
		return false;
	}
	
	if ( listen( FileDescriptor, 50 ) < 0 )
	{
		Error = esleCouldntListen;
		close( FileDescriptor );
		FileDescriptor = -1;
		return false;
	}

	Ready = true;

	return true;
}

////////////////////////
// Code object interface
////////////////////////
const string SocketListener::codeGetClassName() const
{
	return "SocketListener";
}

const string SocketListener::codeGetBasicInfo() const
{
	// for copying results into
	ostringstream os;

	os << "Desc #" << this->GetDescriptor() << ", port #" << this->GetPort() << ".";

	return os.str();
}

const string SocketListener::codeGetFullInfo() const
{
	// for copying results into
	ostringstream os;

	os << "Listener socket:" << endl;

	if (!Ready)
	{
		os << "Socket not yet ready." << endl;
		return os.str();
	}

	os << "Listening on port " << this->GetPort() <<
		", descriptor #" << this->GetDescriptor() << "." << endl;

	return os.str();
}


