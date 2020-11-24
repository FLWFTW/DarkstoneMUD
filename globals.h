
#ifndef __GLOBALS_H_
#define __GLOBALS_H_

// Disable long names warnings under windows.
#ifdef WIN32
  #pragma warning( disable : 4786)
#endif

// handy typedef for 64-bit integers
#ifdef WIN32
  typedef __int64 int64;
  typedef unsigned __int64 uint64;
#else
  typedef long long int64;
  typedef unsigned long long uint64;
#endif

// in milliseconds, the duration between each tick.
#define FRAME_TIME 250

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096  /* buf */
#define MAX_INPUT_LENGTH	 256  /* maximum length for one line */
#define MAX_INBUF_SIZE		 1024
#define MAX_BUFFER_LINES         100
#define MAX_BUFFER_COLS          100
#define MAX_BUFFER_SIZE          10000

#define HASHSTR			 /* use string hashing */


#define MALLOC_CHECK_ 2


extern bool newbielock;
extern bool wizlock;


class World;
extern World * gTheWorld; // comm.cpp

class ConnectionManager;
extern ConnectionManager * gConnectionManager; // comm.cpp

class QuestManager;
extern QuestManager * gQuestManager; // comm.cpp

class DatabaseController;
extern DatabaseController * MasterDatabase; // comm.cpp

extern bool gGameRunning; // comm.cpp
extern bool gBooting; // comm.cpp

#endif


