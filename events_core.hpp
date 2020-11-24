



// Thanks to Nick Gammon!

#ifndef __EVENTS_H_
#define __EVENTS_H_

long GetMillisecondsTime(); // comm.cpp

#include "code_object.h"

#include <iostream>
#include <sstream>
#include <functional>
#include <vector>
#include <queue>
using namespace std;

// event object
class Event : public iCodeObject
{
	
public:
	
	Event (const long iMilliseconds = 1000,	// default, 1 second
		const long iRepeat = 1, 			// default, trigger once
		const long iSequence = 0)  :		// default, unsequenced
	m_iInterval (iMilliseconds),			// time interval
		m_iRepeatTimes (iRepeat),					// whether to keep doing it
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
	inline bool Repeat		(void) const
	{
		// -1 = repeat forever
		if ( m_iRepeatTimes == -1 || m_iInstance < m_iRepeatTimes )
			return true;
		else
			return false;
	};
	inline long GetInstance (void) const { return m_iInstance; };

	virtual void Kill (void) // Kill this event (stop from repeating again)
	{
		m_iRepeatTimes = 0;
	}
	
	// derive a class and override this to actually do something
	virtual void OnEvent (void) = 0;

	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName() { return "Event"; }
	virtual const string codeGetBasicInfo() {
		ostringstream os;
		os << "Event at " << GetInterval() << " interval, " << " repeat ";
		if ( m_iRepeatTimes == -1 )
			os << "forever.";
		else
			os << m_iRepeatTimes << " time(s).";
		return os.str();
	} 
	virtual const string codeGetFullInfo() {
		ostringstream os;
		os << "Event at " << GetInterval() << " interval, " << " repeat ";
		if ( m_iRepeatTimes == -1 )
			os << "forever.";
		else
			os << m_iRepeatTimes << " time(s).";
		os << endl;
		os << "Next scheduled for " << GetTime() << ". Currently at instance " << GetInstance() << ".";
		return os.str();
	}
	
protected:
	
	long  m_iInterval;	// seconds until next one
	long  m_iRepeatTimes;	// how many times to repeat, -1 to loop forever
	
private:
	
	long  m_iWhen;		// when event fires
	long  m_iInstance;	// how many times it fired
	long  m_iSequence;	// what order to sequence events with the same fire time
	
};	// end of class Event

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

