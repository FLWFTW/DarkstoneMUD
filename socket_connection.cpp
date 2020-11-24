


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// OS-dependent header files for socket I/O.
#ifdef unix
	#include <unistd.h>
	#include <sys/socket.h>
	#include <errno.h>
#else
	#ifdef WIN32
	#include <io.h>
	#endif
#endif

#include "globals.h"
#include "socket_connection.h"
#include "utility.h"
#include "World.h"

// STL includes
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
using namespace std;

/********************************************************************/

SocketConnection::SocketConnection()
{
	FileDescriptor = -1;
	OutputBuffer = InputBuffer = InputLine = "";
	Port = 0;
	Host = "";
	OutputLength = 0;
	RepeatTimes = 0;
}

SocketConnection::SocketConnection(int descriptor)
{
	FileDescriptor = descriptor;
	OutputBuffer = InputBuffer = "";
	Port = 0;
	Host = "";
	OutputLength = 0;
}
SocketConnection::~SocketConnection()
{
	Disconnect();
}

/*
   ___        __                     _           
  / _ \ ___  / /  __ __ ___ _ ___ _ (_)___  ___ _
 / // // -_)/ _ \/ // // _ `// _ `// // _ \/ _ `/
/____/ \__//_.__/\_,_/ \_, / \_, //_//_//_/\_, / 
                      /___/ /___/         /___/  

*/

void SocketConnection::Initialize()
{
}

bool SocketConnection::InOutSet()
{
	/*
	 * Short-circuit if nothing to write.
	 */
	if ( OutputLength == 0 )
		return true;

	int bytesWritten = 0;

	while ( OutputLength > 0 )
	{
		// Write as much of buffer as possible.
		#ifdef unix
			bytesWritten = write( FileDescriptor, OutputBuffer.c_str(), /*MIN(OutputLength, 512)*/ OutputLength );
		#else
			#ifdef WIN32
			bytesWritten = send( FileDescriptor, OutputBuffer.c_str(), /*MIN(OutputLength, 512)*/ OutputLength, 0 );
			#endif
		#endif

		if (bytesWritten < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break; // this is normal, and can happen,
				// so just stop for now

			return false; // Something went wrong!
		}

		if (bytesWritten == 0)
			continue;

		if (bytesWritten < OutputLength)
			OutputBuffer = OutputBuffer.substr( bytesWritten );
		else if (bytesWritten == OutputLength)
			OutputBuffer = "";

		OutputLength -= bytesWritten;
	}

	// All good!
	return true;
}

bool SocketConnection::InInSet()
{
	char buf[MAX_INPUT_LENGTH];

	int bytesRead;
	// subtract 1 so that we can always set last to 0
#ifdef unix
	bytesRead = read( FileDescriptor, buf, MAX_INPUT_LENGTH - 1 );
#else
	#ifdef WIN32
	bytesRead = recv( FileDescriptor, buf, MAX_INPUT_LENGTH - 1, 0 );
	#endif
#endif

	if (bytesRead <= 0)
		return false; // Something went wrong!

	buf[bytesRead] = '\0';

	InputBuffer.append(buf);

	// Check for overflow
	if (InputBuffer.length() > MAX_INBUF_SIZE)
	{
		char log_buf[MAX_INPUT_LENGTH];
		sprintf( log_buf, "%s input overflow!", GetHost() );
		gTheWorld->LogString( log_buf );
		SendText("\n\r*** PUT A LID ON IT!!! ***\n\r" );
		return false;
	}

	return true;
}

/*
 * Transfer one line from input buffer to input line.
 * Return true if a line was found.
 * Otherwise, return false.
 */
bool SocketConnection::FindLine()
{
	unsigned int inputBufferPos;
	unsigned int lineBufferPos;
	unsigned int length;
	length = InputBuffer.length();
	if ( length == 0 )
		return false; // if it's empty, just stop now

	const char * pInputBuffer = InputBuffer.c_str();
	char lineBuffer[MAX_INPUT_LENGTH];

	for ( inputBufferPos = 0, lineBufferPos = 0; inputBufferPos < length; inputBufferPos++ )
	{
		// leave one off for final 0
		if ( lineBufferPos >= MAX_INPUT_LENGTH - 1 )
		{
			SendText("Line too long - truncated.\n\r");
			// skip the rest of the line
			/*
			for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
			{
				if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
				break;
			}
			*/
			// Empty the input buffer
			InputBuffer = "";
			InputLine = "";
			return false;
		}
		
		if ( pInputBuffer[inputBufferPos] == '\b' && lineBufferPos > 0 )
		{
			// Move back the pointer one so that the next character
			// overwrites the one that was backspaced.
			--lineBufferPos;
		}
		else if ( pInputBuffer[inputBufferPos] == '\n' || pInputBuffer[inputBufferPos] == '\r' )
		{
			inputBufferPos++;
			if ( inputBufferPos < length )
			{
				// there's at least one character left. Make sure it's not a \n or \r
				if ( pInputBuffer[inputBufferPos] == '\n' || pInputBuffer[inputBufferPos] == '\r' )
					// It is... so we need to cut off that character.
					inputBufferPos++;
			}

			if (lineBufferPos == 0)
			{
				// This needs to be a space so that it's recognized
				// as "input" - even if it's just empty input
				InputLine = " "; 
			}
			else
			{
				lineBuffer[lineBufferPos] = '\0';
				InputLine.assign(lineBuffer);
			}
			
			// Remove all processed input buffer.
			if (inputBufferPos >= length)
				InputBuffer = "";
			else
				InputBuffer = InputBuffer.substr(inputBufferPos);

			return true;
		}
		else if ( isascii(pInputBuffer[inputBufferPos]) && isprint(pInputBuffer[inputBufferPos]) )
			lineBuffer[lineBufferPos++] = pInputBuffer[inputBufferPos];
	}

	return false;
}

void SocketConnection::ProcessInputBuffer()
{
	// Do nothing.
}

bool SocketConnection::InExcSet()
{
	//close ( FileDescriptor );
	//FileDescriptor = -1;
	return true;
}

////////////////////////
// Code object interface
////////////////////////
const string SocketConnection::codeGetClassName()
{
	return "SocketConnection";
}

const string SocketConnection::codeGetBasicInfo()
{
	// for copying results into
	ostringstream os;

	os << "Desc #" << this->GetDescriptor() << ".";

	os << "Pending: " << OutputLength << " outbound, " <<
		InputBuffer.length() << " inbound.";

	return os.str();
}

const string SocketConnection::codeGetFullInfo()
{
	// for copying results into
	ostringstream os;

	os << "Socket connection:" << endl;

	os << "Opened on port " << this->GetPort() <<
		", descriptor #" << this->GetDescriptor() << "." << endl;

	return os.str();
}


