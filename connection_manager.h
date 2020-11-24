

#ifndef __CONNECTION_MANAGER_H_
#define __CONNECTION_MANAGER_H_

#include "socket_general.h"
#include "connection.h"

class ConnectionManager : public SocketManager
{
protected:
public:
	ConnectionManager();
	virtual ~ConnectionManager();

	///////////////////////
	// Base class overrides
	///////////////////////
	virtual void ForceFlushOutput(); // force flush of everyone's output buffer.
	
	virtual void RemoveSocket(SocketGeneral * socket, bool removeNow = false); // add a socket to the cleanup list.
	virtual void RemoveSocket(idSocket id, bool removeNow = false); // add a socket to the cleanup list.

	virtual void ProcessLine(const string line, SocketGeneral * socket); // Nanny

	////////////
	// Additions
	////////////
	bool CheckPlaying(PlayerConnection * d, const char * name, bool kick );
	bool CheckParseEmail(const char *email, PlayerConnection * d);
	bool CheckParseName (const char * name, PlayerConnection * d);
	bool CheckReconnect( PlayerConnection * d, const char *name, bool fConn );
};

class eAlreadyPlaying
{
public:
	eAlreadyPlaying() { ; }
};

bool mail_account_password(PlayerConnection *, const char*, const char*);

/* Matthew */
extern      ACCOUNT_DATA * first_account;
extern      ACCOUNT_DATA * last_account;
extern      ACCOUNT_BAN_DATA * first_account_ban;
extern      ACCOUNT_BAN_DATA * last_account_ban;


#endif



