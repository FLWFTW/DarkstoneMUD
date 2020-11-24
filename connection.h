

// Copyright 2003 David "Ksilyan" Haley

#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include "mud.h"
#include "socket_connection.h"
#include "pager.h"

#include <list>
#include <stack>
#include <set>
#include <string>


// forward declaration
typedef tId<Character> idCharacter;


/*
 * Connected state for a channel.
 */
enum eConnectedStates {
  CON_GET_EMAIL = -50,
  CON_GET_NEW_EMAIL,
  CON_GET_OLD_PASSWORD,
  CON_GET_MAIN_MENU_CHOICE,
  CON_GET_NEW_PASSWORD,
  CON_CONFIRM_NEW_PASSWORD,
  CON_GET_NEW_ACCOUNT,
  CON_GET_CHAR_NAME,
  CON_GET_NEW_NAME,
  CON_CONFIRM_NEW_NAME,
  CON_GET_NEW_SEX,
  CON_GET_NEW_CLASS,
  CON_GET_ADD_NAME,
  CON_READ_MOTD,
  CON_READ_MOTD_NOCHAR,
  CON_GET_NEW_RACE,
  CON_GET_ADD_PASS,
  CON_CONFIRM_ADD_PASS,
  CON_GET_EMULATION,
  CON_GET_WANT_RIPANSI,
  CON_TITLE,
  CON_PRESS_ENTER,
  CON_WAIT_1,
  CON_WAIT_2,
  CON_WAIT_3,
  CON_ACCEPTED,
  CON_READ_IMOTD,
  CON_COPYOVER_RECOVER,
  CON_DELETE_PROMPT,
  CON_DELETE_PROMPT_2,

  CON_PLAYING = 0,
  CON_EDITING
};

class PlayerConnection : public SocketConnection
{
private:
	short IdleTime;
	time_t LastWrite; // the time at which data was last written sucessfully
	bool UsingMXP;
	unsigned char PreviousColor;
	unsigned char CurrentColor;
	
	list<idCharacter> Snooping; // Who this connection is snooping.

	Pager * pager_; // output pager
	InputHandler * OldInputReceiver;

	stack<short> ColorStack; // stack of color changes

public:
	PlayerConnection(int descriptor);
	virtual ~PlayerConnection();

	///////////////////////
	// Core connection code
	// (SocketConnection)
	///////////////////////

	virtual bool WantsToWrite();
	virtual bool InOutSet();

	virtual bool InInSet();
	virtual void ProcessInputBuffer();

	virtual bool InExcSet();

	virtual void Initialize();

	virtual void SendText(const char * text, unsigned int textLength = 0);
	inline virtual void SendText(std::string text, unsigned int textLength = 0)
	{
		this->SendText(text.c_str(), (textLength <= 0 ? text.length() : textLength) );
	}

	bool IsWriteTimingOut();

	// Stuff

	eConnectedStates ConnectedState;

	bool InCommand;
	bool InPrompt;

	void EnableMXP();

	string User; // not used for now... is looked up through identd

	ACCOUNT_DATA * Account;

	// these should be moved to protected at some point
	idCharacter CurrentCharId;
	idCharacter OriginalCharId;

	/////////////////////
	// Pre-game functions
	// (menus, etc.)
	/////////////////////
	void WriteMainMenu(); // write the main menu
	void ShowMotd(); // show greeting page

	////////////
	// Idle Time
	////////////
	void AddIdle(); // add one tick of idle time
	short GetIdleSeconds() const; // return idle time, in seconds
	short GetIdleTicks() const; // return idle time, in ticks

	///////////
	// Snooping
	///////////
	bool IsSnooping(Character * ch);
	bool IsSnooping(idCharacter id);

	void CancelAllSnoops(); // stop snooping people

	void CancelSnoop(Character * ch);
	bool CancelSnoop(idCharacter id);

	void StartSnooping(Character * ch);
	bool StartSnooping(idCharacter id);

	Character * GetCharacter() const; // returns the currently active character
	Character * GetOriginalCharacter(); // returns the original (i.e. non-switched)

	//void SetCharacter(Character * ch); // set the currently active character
	//void SetOriginalCharacter(Character * ch); // 

	/* NOTE TO SELF
	 * at some point in the future, it would be neat to have a
	 * menu class that would be reusable for any kind of in-game
	 * menu interface.
	 */

	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName() const; // Return the name of the class.
	virtual const string codeGetBasicInfo() const; // Return a short summary of the object.
	virtual const string codeGetFullInfo() const; // Return a complete summary of the object.

	//////OLD STUFF/////
	//PlayerConnection * snoop_by; // who's snooping me

	//sh_int		idle;
	//sh_int		lines;
	//sh_int		scrlen;
	//bool		fcommand;
	//char		inbuf		[MAX_INBUF_SIZE];
	//char		incomm		[MAX_INPUT_LENGTH];
	//char		inlast		[MAX_INPUT_LENGTH];
	//int 		repeat;
	//char *		outbuf;
	//unsigned long	outsize;
	//int 		outtop;
	//char *		pagebuf;
	//unsigned long	pagesize;
	//int 		pagetop;
	//char *		pagepoint;
	//char		pagecmd;
	char		pagecolor;
	int 		auth_inc;
	int 		auth_state;
	char		abuf[ 256 ];
	int 		auth_fd;
	int 		atimes;
	int 		newstate;
	unsigned char	prevcolor;
};

/*
 * Account ban structure.
 */
struct account_ban_data
{
    ACCOUNT_BAN_DATA * next;
    ACCOUNT_BAN_DATA * prev;
    char     * address;
    time_t     secBanTime;
    int        flags;
};

/*
 * A player account
 */
class Account
{
public:
	Account();
	~Account();

	shared_str autoLoginName_; // name of character to log in to automatically

	// old stuff
	ACCOUNT_DATA* next;
	ACCOUNT_DATA* prev;
	
	int 	 iref; /* keep track of how many instances */
	
        std::string password_;
        std::string forumPassword_; // added by Ksilyan
	char * characters;

        std::set<std::string> allCharacters_;
	
	char * email;
	
	int 	 flags;
	time_t	 secCreatedDate;
	
	NoteData * comments;
};

#endif


