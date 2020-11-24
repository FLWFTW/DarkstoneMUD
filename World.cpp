/*=-=-=-=-=-=-=-=\***********************************************************\
|  K ~ M U D     |    An online, interactive text-based adventure.          *
\*-=-=-=-=-=-=-=-/                                                          *
 * Written and designed by David "Ksilyan" Haley (c) 2001-2003              *
 * All code not part of the below is the property of David Haley. The       *
 * Legends of the Darkstone is granted perpetual usage rights to this code. *
 ****************************************************************************
 * Based on SMAUG:Darkstone, written by Ydnat, Warp and Testaur (c)1997-2001*
 ****************************************************************************
 * SMAUG 1.0 (c) 1994-1996 Derek Snider.                                    *
 * Original SMAUG team: Thoric, Altrag, Blodkai, Narn, Haus, Scryn, Rennard,*
 * Swordbearer, Gorog, Grishnakh and Tricops.                               *
 ****************************************************************************
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 ****************************************************************************
 *               World Controller: World                                   *
\****************************************************************************/


#include "globals.h"
#include "World.h"
#include "connection.h"
#include "mud.h"

#include "utility_objs.hpp"

#include "lua_util.h"

// STL includes
#include <list>
#include <algorithm>
#include <iterator>
#include <functional>
using namespace std;


void save_sysdata ( SYSTEM_DATA sys );

/*
=====================================================================
  _____                 __                   __               ____   
 / ___/___   ___   ___ / /_ ____ __ __ ____ / /_ ___   ____  / __/___
/ /__ / _ \ / _ \ (_-</ __// __// // // __// __// _ \ / __/  > _/_ _/
\___/ \___//_//_//___/\__//_/   \_,_/ \__/ \__/ \___//_/    |_____/                                                                       
   ___            __                   __            
  / _ \ ___  ___ / /_ ____ __ __ ____ / /_ ___   ____
 / // // -_)(_-</ __// __// // // __// __// _ \ / __/
/____/ \__//___/\__//_/   \_,_/ \__/ \__/ \___//_/   
                                                     
=====================================================================
*/


World::World()
{
    // Create the lua state
    luaState_ = lua_open();

    // Initialize the libraries
    luaL_openlibs(luaState_);

}

World::~World()
{

	// Clean up any connections that weren't removed by the Connection manager
	itorSocketId itor;
	for (itor = this->PlayerConnections.begin(); itor != this->PlayerConnections.end(); itor++)
	{
		if ( SocketMap[*itor] )
			delete SocketMap[*itor];
	}

        // close the Lua state
        lua_close(luaState_);
}


/*
=====================================================
  _____                            __   _           
 / ___/___   ___   ___  ___  ____ / /_ (_)___   ___ 
/ /__ / _ \ / _ \ / _ \/ -_)/ __// __// // _ \ / _ \
\___/ \___//_//_//_//_/\__/ \__/ \__//_/ \___//_//_/
   ____                 __   _                
  / __/__ __ ___  ____ / /_ (_)___   ___   ___
 / _/ / // // _ \/ __// __// // _ \ / _ \ (_-<
/_/   \_,_//_//_/\__/ \__//_/ \___//_//_//___/

=====================================================
*/

void World::AddConnection(PlayerConnection * connection)
{
	PlayerConnections.push_back( connection->GetId() );

	if ( PlayerConnections.size() > uint(sysdata.maxplayers) )
		sysdata.maxplayers = PlayerConnections.size();

	if ( sysdata.maxplayers > sysdata.alltimemax )
	{
		char buf[128];
		if ( sysdata.time_of_max )
			DISPOSE(sysdata.time_of_max);
		sprintf(buf, "%24.24s", ctime(&secCurrentTime));
		sysdata.time_of_max = str_dup(buf);
		sysdata.alltimemax = sysdata.maxplayers;
		sprintf( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );
		log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
		to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
		save_sysdata( sysdata );
	}
}

void World::RemoveConnection(PlayerConnection * connection)
{
	if ( !connection ) 
		return;

	PlayerConnections.remove( connection->GetId() );
}

void World::RemoveConnection(idSocket id)
{
	PlayerConnections.remove( id );
}

short World::ConnectionCount()
{
	return PlayerConnections.size();
}

itorSocketId World::GetBeginConnection()
{
	return PlayerConnections.begin();
}
itorSocketId World::GetEndConnection()
{
	return PlayerConnections.end();
}
ritorSocketId World::GetBeginReverseConnection()
{
	return PlayerConnections.rbegin();
}
ritorSocketId World::GetEndReverseConnection()
{
	return PlayerConnections.rend();
}

// private utility function
void World_fDoIdleWait(idSocket & id)
{
	PlayerConnection * socket = (PlayerConnection*) SocketMap[id];
	// Abort, just in case connection is null somehow.
	if (!socket)
		return;

	//printf("Running World_fDoIdleWait on connection number %d.\n", connection->GetDescriptor());

	// Add one tick of idle time to the player.
	socket->AddIdle();

	// Reduce character's wait time by one.
	// It checks to make sure it doesn't go negative.
	if ( socket->GetCharacter() )
	{
		socket->GetCharacter()->AddWait(-1);
	}

	//return;

	//printf("Leaving World_fDoIdleWait.\n");
}



/*
   ____                 __ 
  / __/_  __ ___  ___  / /_
 / _/ | |/ // -_)/ _ \/ __/
/___/ |___/ \__//_//_/\__/ 
                           
   __ __               __ __         
  / // /___ _ ___  ___/ // /___  ____
 / _  // _ `// _ \/ _  // // -_)/ __/
/_//_/ \_,_//_//_/\_,_//_/ \__//_/   
                                     
*/

// Thanks go to Nick Gammon for this code!

// Nick's iterator can be used for inserting into the event queue
class event_queue_back_inserter : public iterator <output_iterator_tag, Event*>
{
public:   
	
	event_queue_back_inserter (tEvents & events) : m_events (events) {};
	
	// assignment is used to insert into the queue
	event_queue_back_inserter & operator= (Event* & e)
	{ 
		m_events.push (e);
		return *this;
	};
	
	// dereference and increments are no-ops that return 
	// the iterator itself
	event_queue_back_inserter & operator* () { return *this; };
	event_queue_back_inserter & operator++ () { return *this; };
	event_queue_back_inserter & operator++ (int) { return *this; };
	
private:
	
	tEvents & m_events;
	
	
};	// end of class event_queue_back_inserter

// helper routine to add an event
void World::AddEvent (Event * e)
{
	if (e)	  // being cautious here :)
		EventQueue.push (e);
} // end of AddEvent


// 1. pull out events from the event queue
// 2. keep going until one is due in the future
// 3, if rescheduling wanted keep in temporary vector until all are done

void World::ProcessEvents()
{
	long now = GetMillisecondsTime ();
	vector<Event*> repeated_events;
	
	// pull out event that need doing
	while (!EventQueue.empty ())
	{
		Event * e = EventQueue.top ();
		
		if (e->GetTime () > now)
			break;	// not yet
		
		EventQueue.pop ();	// remove from queue
		
		e->Fired ();	// note it fired
		e->OnEvent ();	// do the event (derived class)
		
		// if repeating it wanted, recalculate and keep in separate list
		if (e->Repeat ())
		{
			e->Reschedule ();
			repeated_events.push_back (e);
		}
		else
			delete e;	// otherwise pointer not needed any more
		
	}  // end of event loop
	
	// put any event we rescheduled back into the queue
	// we do it now so we don't get into a loop if the event is due to
	// fire immediately
	copy (repeated_events.begin (),
		repeated_events.end (),
		event_queue_back_inserter (EventQueue));
	
} // end of ProcessEvents



/*
   __  ___ _                __ __                     
  /  |/  /(_)___ ____ ___  / // /___ _ ___  ___  ___ _
 / /|_/ // /(_-</ __// -_)/ // // _ `// _ \/ -_)/ _ `/
/_/  /_//_//___/\__/ \__//_//_/ \_,_//_//_/\__/ \_,_/ 
                                                      
*/

void World::TimeUnit()
{
	if ( !PlayerConnections.empty() )
	{
		// Make a local list to not screw up the one we're working on.
		// Connections can be removed if they time out.
		list<idSocket> localList = PlayerConnections;
		for_each(localList.begin(), localList.end(), &World_fDoIdleWait);
	}

	/*
	 * Autonomous game motion.
	 */
	update_handler( );

	ProcessEvents();

	/*
	 * Check REQUESTS pipe
	 */
	check_requests( );
}


///////////////////////////////////////////////////////////////////////////////

// Lua

lua_State * World::getLuaState()
{
    return luaState_;
}

bool World::initializeLua()
{
    if (!Lua::InitializeLuaState(luaState_))
    {
        gTheWorld->LogBugString("Failed to set up Lua state "
                                "for Darkstone");
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////

void World::LogString(const char * text)
{
	log_string(text);
}


void World::LogCommString(const char * text, short level)
{
	log_string_plus( text, LOG_COMM, (level == 0 ? sysdata.log_level : level) );
}

void World::LogBugString(const char * text)
{
	bug(text);
}

void World::LogCodeString(const char * text, short level)
{
	to_channel( text, CHANNEL_CODETALK, "CodeInfo", level );
}

