



#ifndef __WORLD_H_
#define __WORLD_H_

#include "globals.h"
#include "events_core.hpp"
#include "managed_objects.hpp"

#include <list>
#include <string>

extern "C" {
    #include <lua.h>
}

// Forward declarations
class SocketGeneral;
class PlayerConnection;
class QuestManager;
typedef tId<SocketGeneral> idSocket;
typedef std::list<idSocket>::iterator itorSocketId;
typedef std::list<idSocket>::reverse_iterator ritorSocketId;

class World
{
protected:
	std::list<idSocket> PlayerConnections;

	// Variable: luaState_
	// The Lua universe object.
	lua_State * luaState_;

	////////////////////////

public:
	World();
	virtual ~World();

	/////////////////////////////////
	// Connection related commands //
	/////////////////////////////////

	// Connections list iterator accessors
	itorSocketId GetBeginConnection(); // begin()
	itorSocketId GetEndConnection(); // end()
	ritorSocketId GetBeginReverseConnection(); // rbegin()
	ritorSocketId GetEndReverseConnection(); // rend()

	short ConnectionCount();

	void AddConnection(PlayerConnection * connection);
	void RemoveConnection(PlayerConnection * connection);
	void RemoveConnection(idSocket id);

	/////////////////////////////
	// Miscellaneous functions //
	/////////////////////////////

	void TimeUnit(); // clock tick

        ///////////////////
        // Lua Functions //
        ///////////////////
        
        lua_State * getLuaState();
        bool initializeLua();

	///////////////////
	// Event Handler //
	///////////////////

	tEvents EventQueue;
	void ProcessEvents ();
	void AddEvent (Event * e);

	///////////////////
	// Log functions //
	///////////////////
	void LogString(const char * text);
	inline void LogString(const std::string & text) { LogString( text.c_str() ); }

	void LogCommString(const char * text, short level = 0); // send a string to the comm log
	inline void LogCommString(const std::string & text, short level = 0) { LogCommString(text.c_str(), level ); }

	void LogBugString(const char * text);
	inline void LogBugString(const std::string & text) { LogBugString(text.c_str()); }

	void LogCodeString(const char * text, short level = 0);
	inline void LogCodeString(const std::string & text, short level = 0) { LogCodeString(text.c_str(), level ); }
};




#endif



