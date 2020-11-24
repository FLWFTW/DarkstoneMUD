



// Thanks to Nick Gammon!

#ifndef __EVENTS_H_
#define __EVENTS_H_

long GetMillisecondsTime(); // comm.cpp

#include <functional>
#include <vector>
#include <queue>
using namespace std;

// event object
class Event
{
	
public:
	
	Event (const long iMilliseconds = 1000,	// default, 1 second
		const bool bRepeat = false, 		// default, no repeat
		const long iSequence = 0)  :		// default, unsequenced
	m_iInterval (iMilliseconds),			// time interval
		m_bRepeat (bRepeat),					// whether to keep doing it
		m_iWhen (GetMillisecondsTime ()),		// set last fired time to now
		m_iInstance (0),
		m_iSequence (iSequence)
	{ 
		Reschedule ();	// calculate when it first fires
	};
	
	virtual ~Event () {};	// important - see Scott Meyers
	
	// operator< (for sorting)
	inline bool operator< (const Event & rhs) const
	{
		// we compare > because the sooner events have the
		// higher priority
		if	(m_iWhen == rhs.m_iWhen)
			return	m_iSequence > rhs.m_iSequence;	// sequence order if times same
		else
			return	m_iWhen > rhs.m_iWhen;			// time order
	};
	
	// call to reschedule it - add interval to last fired time
	// so as to avoid event creep
	inline void Reschedule (void) 
	{ 
		m_iWhen += m_iInterval; 
	};
	
	inline void Fired (void)
	{
		m_iInstance++;
	};
	
	// get values
	inline long GetTime 	(void) const { return m_iWhen; };
	inline long GetInterval (void) const { return m_iInterval; };
	inline bool Repeat		(void) const { return m_bRepeat; };
	inline long GetInstance (void) const { return m_iInstance; };
	
	// derive a class and override this to actually do something
	virtual void OnEvent (void) = 0;
	
protected:
	
	long  m_iInterval;	// seconds until next one
	bool  m_bRepeat;	// true if we are to re-enter in queue
	
private:
	
	long  m_iWhen;		// when event fires
	long  m_iInstance;	// how many times it fired
	long  m_iSequence;	// what order to sequence events with the same fire time
	
};	// end of class CEvent

// for adding events to a priority_queue
struct event_less : binary_function<Event*, Event*, bool> 
{
	inline bool operator() (const Event* X, const Event* Y) const
	{
		return (*X < *Y); 
	}
};

typedef priority_queue<Event*, vector<Event*>, event_less > tEvents;

#endif // include guard

