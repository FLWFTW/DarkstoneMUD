/*=-=-=-=-=-=-=-=\***********************************************************\
|  K ~ M U D     |    An online, interactive text-based adventure.          *
\*-=-=-=-=-=-=-=-/                                                          *
 * Written and designed by David "Ksilyan" Haley (c) 2001-2003              *
 * All code below is the property of David Haley. The Legends of the        *
 * Darkstone is granted perpetual usage rights to this code.                *
 ****************************************************************************
 *               Player Connection Code (see socket_connection)             *
\****************************************************************************/

#include "globals.h"
#include "mud.h"

#include "character.h"
#include "connection.h"
#include "mxp.h"
#include "World.h"
#include "connection_manager.h"

#ifdef unix
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <arpa/telnet.h>
#else
	#ifdef WIN32
		#include <io.h>
	#endif
#endif

// STL includes
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
using namespace std;

// Forward declaration
void do_help(Character * ch, const char* argument);

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

PlayerConnection::PlayerConnection(int descriptor)
{
	FileDescriptor = descriptor;
	//OutputBuffer = InputBuffer = "";
	//InputLine = "";
	LastLine = "";
	Port = 0;
	Host = User = "";
	OutputLength = 0;
	OutputBuffer.assign("");
	UsingMXP = InCommand = false;
	Account = NULL;
	ConnectedState = CON_GET_EMAIL;
	PreviousColor = 0x07;

	auth_fd = -1;

	pagecolor = 0;
	auth_inc = auth_state = 0;
	//auth_fd = 0;
	atimes = 0;
	newstate = 0;
	prevcolor = 0;

	InPrompt = false;
	IdleTime = 0;

	OldInputReceiver = NULL;
	pager_ = NULL;

	LastWrite = secCurrentTime;

	Snooping.clear();
}

PlayerConnection::~PlayerConnection()
{
	char log_buf[MAX_INPUT_LENGTH];
	sprintf( log_buf, "Disconnect:  %s", Address.c_str() );
	log_string_plus( log_buf, LOG_COMM, sysdata.log_level );

	if ( Account ) {
		free_account_data(Account);
	}

	//if (Account)
		//delete Account;
	if ( FileDescriptor > 0 )
		close (FileDescriptor);

	//gTheWorld->Connections.remove(this);
}

/*
   ___        __                     _           
  / _ \ ___  / /  __ __ ___ _ ___ _ (_)___  ___ _
 / // // -_)/ _ \/ // // _ `// _ `// // _ \/ _ `/
/____/ \__//_.__/\_,_/ \_, / \_, //_//_//_/\_, / 
                      /___/ /___/         /___/  

*/

void PlayerConnection::Initialize()
{
	BAN_DATA * pban;

	extern HELP_DATA *curr_greeting; // db.cpp
	
	if (Host.length() > 0)
		sprintf( log_buf, "Connection:  %s (%s)", Address.c_str(), Host.c_str() );
	else
		sprintf( log_buf, "Connection:  %s", Address.c_str() );
	log_string_plus( log_buf, LOG_COMM, sysdata.log_level );

	// Kick off the bad boys
	for ( pban = first_ban; pban; pban = pban->next )
	{
		// This used to use str_suffix, but in order to do bans by the 
		//	  first part of the ip, ie "ban 207.136.25" str_prefix must
		//	  be used. -- Narn
		
		if ( (!str_prefix( pban->site, GetHost() )|| !str_suffix(pban->site, GetHost())) )
		{
			SendText(
				"\r\r\n\nYour site has been banned from this Mud.\r\n\r\n" );
			sprintf(log_buf, "Dropping banned site %s (rule: %s).", GetHost(), pban->site);
			gTheWorld->LogCommString(log_buf);
			gConnectionManager->RemoveSocket(this, false);
			//free_desc( dnew );
			set_alarm( 0 );
			return;
		}
	}
	
	/*
	* Send the greeting.
	*/
	if (curr_greeting->text[0] == '.')
		SendText( curr_greeting->text + 1);
	else
		SendText( curr_greeting->text);
	
	curr_greeting = curr_greeting->next;

	/* telnet negotiation to see if they support MXP */
	this->SendText((const char *) will_mxp_str);
	
	ConnectedState = CON_GET_EMAIL;
	SendText("\r\nEnter your email address, or NEW if you are new here: ");
	
	//start_auth( this ); // Start username authorization
	
	set_alarm(0);
	
	gTheWorld->AddConnection(this);
}

Character * PlayerConnection::GetCharacter() const
{
	if ( CurrentCharId == 0 )
		return NULL;
	else
	{
		Character * result = CharacterMap[CurrentCharId];
		if ( result )
			return result;
		else
		{
			gTheWorld->LogBugString(
				"PlayerConnection::GetCharacter() invalid CurrentChar reference! Char: " + this->codeGetBasicInfo() );
			return NULL;
		}
	}
}
Character * PlayerConnection::GetOriginalCharacter()
{
	if ( OriginalCharId == 0 )
		return this->GetCharacter(); // no original, so just send current char
	else
	{
		Character * result = CharacterMap[OriginalCharId];
		if ( result )
			return result;
		else
		{
			gTheWorld->LogBugString(
				"PlayerConnection::GetOriginalCharacter() invalid OriginalCharId reference! Char: " + this->codeGetBasicInfo() );
			return NULL;
		}
	}
}

bool PlayerConnection::IsSnooping(Character * ch)
{
	if ( !ch )
		return false;

	idCharacter id = ch->GetId();
	if ( find(Snooping.begin(), Snooping.end(), id) == Snooping.end() )
		return false;
	else
		return true;
}

bool PlayerConnection::IsSnooping(idCharacter id)
{
	if ( find(Snooping.begin(), Snooping.end(), id) == Snooping.end() )
		return false;
	else
		return true;
}

void PlayerConnection::StartSnooping(Character * ch)
{
	if ( !ch )
		return;

	Snooping.push_back(ch->GetId());
}

void PlayerConnection::CancelSnoop(Character * ch)
{
	if ( !ch )
		return;

	itorCharacterId itor;

	list<idCharacter> localList;
	localList = Snooping;

	for ( itor = localList.begin(); itor != localList.end(); itor++ )
	{
		if ( *itor == ch->GetId() )
		{
			ch->StopSnoopedBy(this);
			Snooping.remove( ch->GetId() );
		}
	}
}

void PlayerConnection::CancelAllSnoops()
{
	itorCharacterId itor;
	for ( itor = Snooping.begin(); itor != Snooping.end(); itor++ )
	{
		Character * ch = CharacterMap[*itor];
		if ( ch )
			ch->StopSnoopedBy(this);
	}
	Snooping.clear();
}

void PlayerConnection::SendText(const char * text, unsigned int textLength)
{
	// Create pager, if necessary.
	Character * character;
	character = GetOriginalCharacter();
	if ( character && !IS_NPC(character) && character->pcdata)
	{
		if ( IS_SET(character->pcdata->flags, PCFLAG_PAGERON) )
		{
			// The player wants a pager, so create it if necessary
			if (!pager_)
				pager_ = new Pager(character->pcdata->pagerlen);
			else
				pager_->SetLineLimit(character->pcdata->pagerlen); // update just in case
		}
		else
		{
			// The player does NOT want a pager, so if it exists, delete it.
			if (pager_)
				delete pager_;
			pager_ = NULL;
		}
	}

	// Translate MXP tags if needed.
	char * buf;
	int lengthToSend = (textLength <= 0 ? strlen(text) : textLength);
	int realLength = lengthToSend + count_mxp_tags( UsingMXP, text, lengthToSend );

	// Add 1 for trailing zero.
	buf = (char *) malloc(realLength + 1);
	memset(buf, 0, realLength + 1);

	convert_mxp_tags( UsingMXP, buf, text, lengthToSend );

	// don't send only \r to the pager, send it directly
	// to the connection
	if (pager_ && strcmp(text, "\r") )
	{
		// Add an initial newline, if the buffer is empty
		// and we're not processing a command.
		if (pager_->BufferSize() == 0 && (!InCommand && !InPrompt))
			pager_->AddToBuffer("\r\n");

		// Forward output to pager
		pager_->AddToBuffer(buf);
	}
	else
	{
		// Add an initial newline, if the buffer is empty
		// and we're not processing a command.
		if (OutputLength == 0 && (!InCommand && !InPrompt))
		{
			OutputBuffer.append("\r\n");
			OutputLength += 2;
		}

		OutputBuffer.append(buf);
		OutputLength += realLength;
	}

	// Free the temporary storage
	free(buf);
}

bool PlayerConnection::WantsToWrite()
{
	if (pager_)
		return ( OutputLength > 0 || pager_->WantsToWrite() );
	else
		return (OutputLength > 0);
}

bool PlayerConnection::InOutSet()
{
	// Redirect input, if it hasn't been redirected already
	if (pager_)
	{
		if (!OldInputReceiver)
		{
			OldInputReceiver = InputReceiver;
			SetInputReceiver(pager_);
		}

		// Get next set of output, and
		// add it to the connection's buffer
		// If we're waiting for user input, this will be empty.
		OutputBuffer.append(pager_->GetNextOutput());
		OutputLength = OutputBuffer.length();
	}


	// Return false in case output goes wrong.
	// The socket manager will remove the socket,
	// so here we only want to save the character.
	if ( !SocketConnection::InOutSet() )
	{
		if ( GetCharacter()
			&& ( ConnectedState == CON_PLAYING
			||	 ConnectedState == CON_EDITING ) )
			save_char_obj( GetCharacter() );
		
		return false;
	}

	LastWrite = secCurrentTime;

	// reset input back to the old input receiver, if pager has
	// been processed...
	if (pager_)
	{
		if ( pager_->BufferEmpty() )
		{
			if (OldInputReceiver)
			{
				SetInputReceiver(OldInputReceiver);
				OldInputReceiver = NULL;
			}
			pager_->ResetLineCount();
		}
	}

	// normally we only arrive here if there was something to write.
	// so, if the buffer is empty now... that means that it was emptied
	// on this go.
	if ( !WantsToWrite() && (this->GetCharacter() != NULL) )
		GetCharacter()->OutputBufferEmptied();

	return true;
}

bool PlayerConnection::InInSet()
{
	// First thing, reset idle time
	IdleTime = 0;

	if ( GetCharacter() )
		GetCharacter()->timer = 0;

	// Read from the socket into
	// InputBuffer.
	if ( !SocketConnection::InInSet() )
	{
		if ( GetCharacter()
			&& ( ConnectedState == CON_PLAYING
			||	 ConnectedState == CON_EDITING ) )
			save_char_obj( GetCharacter() );

		return false;
	}

	// Look for incoming telnet negotiation.
	const char * buffer = InputBuffer.c_str();
	unsigned char * p;

	for (p = (unsigned char *) buffer; *p; p++)
	{
		if ( *p == IAC )
		{
			if (memcmp (p, do_mxp_str, strlen ((const char*) do_mxp_str)) == 0)
			{
				EnableMXP();
				/* remove string from input buffer */
				InputBuffer.assign( (char *) p + strlen ((const char*) do_mxp_str));
			} /* end of turning on MXP */
			else  if (memcmp (p, dont_mxp_str, strlen ((const char*)dont_mxp_str)) == 0)
			{
				UsingMXP = false;
				/* remove string from input buffer */
				InputBuffer.assign( (char *) p + strlen ((const char*) dont_mxp_str));
			} /* end of turning off MXP */
		}
	}

	return true;
}

void PlayerConnection::ProcessInputBuffer()
{
	// Check to see if character has wait time...
	// If so, bail out.
	// TODO: round time system controlled by character,
	// not connection
	if (GetCharacter() && GetCharacter()->GetWait() > 0)
	{
		return;
	}

	// Process as long as we have newlines in the input buffer.
	while ( FindLine() )
	{
		if ( InputLine == "" )
			continue; // don't bother treating an empty string

		// Handle input spamming.

		// if length > 1, or if we're trying to repeat the command
		if ( InputLine.length() > 1 || !InputLine.compare("!") )
		{
			// if input line is NOT a repeat request, AND
			// input line is not equal to last line...
			if ( InputLine.compare("!") && InputLine.compare(LastLine) )
			{
				RepeatTimes = 0;
			}
			else
			{
				if ( GetCharacter() && GetCharacter()->IsFighting() )
					--RepeatTimes;
				
				if ( ++RepeatTimes >= 20 )
				{
					sprintf( log_buf, "%s input spamming (%s)!", GetHost(), InputLine.c_str() );
					log_string( log_buf );
					
					SendText("\r\n*** PUT A LID ON IT!!! ***\r\n");

					// Instead of having them quit - which can be exploited -
					// we have them go link-dead, which is arguably worse
					gConnectionManager->RemoveSocket(this, false);

					InputBuffer = ""; // clear the input buffer

					//strcpy( d->incomm, "quit forcemetoquit" );
				}
			}
		}
		
		/*
		 * Do '!' substitution.
		 */
		if ( InputLine.compare("!") == 0 )
			InputLine = LastLine;
		else
			LastLine = InputLine;

		//fcommand = true;
		stop_idling( GetCharacter() );
		
		//strcpy( cmdline, d->incomm );
		//d->incomm[0] = '\0';
		
		if ( GetCharacter() )
		{
			set_cur_char( GetCharacter() );
			trace(GetCharacter(),InputLine.c_str());  /* send to system trace */
		}
		else
		{
			trace(NULL,(ConnectedState == CON_GET_OLD_PASSWORD ? "<password hidden>" : InputLine.c_str()));
		}
		
		// forward the line to the receiver
		InCommand = true;
		SendLineToReceiver(InputLine);
		InCommand = false;

		// Check to see if character has wait time after commands...
		// If so, bail out.
		// TODO: round time system controlled by character,
		// not connection
		if (GetCharacter() && GetCharacter()->GetWait() > 0)
			break;

		// the following should be handled automatically according
		// to what InputHandler we have
		
		/*if ( d->pagepoint )
						set_pager_input(d, cmdline);
					else
					{
						switch( d->connected )
						{
							default:
								nanny( d, cmdline );
								break;
							case CON_PLAYING:
								interpret( d->character, cmdline );
								break;
							case CON_EDITING:
								edit_buffer( d->character, cmdline );
								break;
						}
					}*/
	}
}

bool PlayerConnection::InExcSet()
{
	if ( GetCharacter()
		&& ( ConnectedState == CON_PLAYING || ConnectedState == CON_EDITING )
		)
	{
		save_char_obj( GetCharacter() );
	}
	
	//close_socket( d, TRUE );
	return true;
}

void PlayerConnection::EnableMXP()
{
	UsingMXP = true;

	SendText( (const char*) start_mxp_str );
	SendText( MXPMODE (6) );	/* permanent secure mode */

	// If we're logging in, send a username request.
	if ( ConnectedState == CON_GET_EMAIL )
	{
		// Send the MXP username request.
		SendText( MXPTAG("username") );
	}

	SendText( MXPTAG ("!-- Set up MXP elements --") );
	// Exit tag
	SendText( MXPTAG ("!ELEMENT Ex \"<send hint='Click to follow this exit'>\" FLAG=RoomExit") );

	// Door
	SendText( MXPTAG
		( "!ELEMENT ExDoor \"<send href='"
		"&text;|"
		"close &text;|"
		"open &text;|"
		"lock &text;"
		"unlock &text;"
		"' "
		"hint='Right-click for door options|"
		"Go through door|"
		"Close the door|"
		"Open the door|"
		"Lock the door|"
		"Unlock the door"
		"'>\" FLAG=RoomExit"
		) );

	/* Room description tag */
	SendText( MXPTAG ("!ELEMENT rdesc '<p>' FLAG=RoomDesc") );

        /* Quest tag */
        SendText( MXPTAG(
                         "!ELEMENT ShowQuestItem \"<send href='"
                         "showquest &text;"
                         "' "
                         "hint='More details on this quest'"
                         ">\""
                         ) );

	/* Get an item tag (for things on the ground) */
	SendText( MXPTAG 
		("!ELEMENT Get \"<send href='"
		"get &#39;&name;&#39;|"
		"examine &#39;&name;&#39;|"
		"drink &#39;&name;&#39;"
		"' "
		"hint='RH mouse click to use this object|"
		"Get &desc;|"
		"Examine &desc;|"
		"Drink from &desc;"
		"'>\" ATT='name desc'") );

	/* Drop an item tag (for things in the inventory) */
	SendText( MXPTAG 
		("!ELEMENT Drop \"<send href='"
		"drop &#39;&name;&#39;|"
		"examine &#39;&name;&#39;|"
		"look in &#39;&name;&#39;|"
		"wear &#39;&name;&#39;|"
		"eat &#39;&name;&#39;|"
		"drink &#39;&name;&#39;"
		"' "
		"hint='RH mouse click to use this object|"
		"Drop &desc;|"
		"Examine &desc;|"
		"Look inside &desc;|"
		"Wear &desc;|"
		"Eat &desc;|"
		"Drink &desc;"
		"'>\" ATT='name desc'") );

	/* Bid an item tag (for things in the auction) */
	SendText( MXPTAG 
		("!ELEMENT Bid \"<send href='bid &#39;&name;&#39;' "
		"hint='Bid for &desc;'>\" "
		"ATT='name desc'") );

	/* List an item tag (for things in a shop) */
	SendText( MXPTAG 
		("!ELEMENT List \"<send href='buy &#39;&name;&#39;' "
		"hint='Buy &desc;'>\" "
		"ATT='name desc'") );

	/* Player tag (for who lists, tells etc.) */
	SendText( MXPTAG 
		("!ELEMENT Player \"<send href='tell &#39;&name;&#39; ' "
		"hint='Send a message to &name;' prompt>\" "
		"ATT='name'") );
}


void PlayerConnection::ShowMotd( )
{
	Character *ch;
	
	ch = GetCharacter();
	
	if ( IS_SET(ch->act, PLR_RIP) )
		send_rip_screen(ch);
	if ( IS_SET(ch->act, PLR_ANSI) )
		send_to_pager( "\033[2J", ch ); // this is a clearscreen command.
	else
		send_to_pager( "\014", ch );

	set_pager_color( AT_LBLUE, ch );

	if ( IS_IMMORTAL(ch) )
		do_help( ch, "imotd" );
	if ( IS_HERO(ch) )
		do_help( ch, "amotd" );
	if ( ch->level < LEVEL_HERO_MIN && ch->level > 0 )
		do_help( ch, "motd" );
	if ( ch->level == 0 )
		do_help( ch, "nmotd" );
	send_to_pager( "\r\nPress [ENTER] ", ch );
	ConnectedState = CON_READ_MOTD;
}

void PlayerConnection::WriteMainMenu()
{	
    SendText("Choose your action:\r\n");
    SendText("\r\n");
    SendText("    1) Change your account password.\r\n");
    SendText("    2) Read the MOTD.\r\n");
    SendText("    3) Play an existing character.\r\n");
    SendText("    4) Create a new character.\r\n");
    SendText("    5) Delete an existing character.\r\n");
    SendText("    6) Quit Legends of the Darkstone.\r\n");
    SendText("\r\n");
    SendText("Which sounds good to you? (1,2,3,4,5,6) ");
}


bool PlayerConnection::IsWriteTimingOut()
{
	if ( this->WantsToWrite() )
	{
		if ( secCurrentTime - this->LastWrite > 180 )
		{
			return true;
		}
	}

	return false;
}

void PlayerConnection::AddIdle()
{
	IdleTime++;

	// If we have data to write, and the last successful write
	// was more than three minutes ago, kill the socket.
	if ( this->IsWriteTimingOut() )
	{
		string buf;
		buf = "Account " + string(Account->email) + " timed out";
		buf += " (last successful write over three minutes ago.)";
		gTheWorld->LogCommString(buf);

		// Remove immediately, since waiting won't
		// clear the buffer
		gConnectionManager->RemoveSocket(this, true);
	}
	else
	{
		// If there is nothing to write, set last
		// time to now so that we don't get disconnected.
		this->LastWrite = secCurrentTime;
	}

	if ( !GetCharacter() )
	{
		if (GetIdleSeconds() > 120) // 2 minutes
		{
			SendText( "Idle timeout... disconnecting.\r\n" );
			string buf;
			if (Account)
			{
				buf = "Account ";
				buf.append(Account->email);
			}
			else
			{
				buf = "Unknown connection";
			}
			buf.append(" timed out.");
			gTheWorld->LogCommString(buf);
			gConnectionManager->RemoveSocket(this, false);
		}
	}
	else
	{
		if ( IS_IMMORTAL(GetCharacter()) )
			return; // immortals can't timeout

		if ( (ConnectedState != CON_PLAYING && GetIdleSeconds() > 300) // 5 minutes
			|| GetIdleSeconds() > 7200) // 2 hours
		{
			SendText( "Idle timeout... disconnecting.\r\n" );
			string buf = "Character ";
			buf.append(GetCharacter()->getName().str());
			buf.append(" (");
			buf.append(Account->email);
			buf.append(") timed out.");
			gTheWorld->LogCommString(buf);
			gConnectionManager->RemoveSocket(this, false);
		}
	}
}

// Returns the idle time, in seconds.
short PlayerConnection::GetIdleSeconds() const
{
	// IdleTime is in clock ticks.
	// Clock ticks come every FRAME_TIME milliseconds...
	// so...
	// seconds = clockticks * (clockticks per second)
	// rearranged order for rounding
	return (short) ( (FRAME_TIME * 0.001) * IdleTime );
}

short PlayerConnection::GetIdleTicks() const
{
	return IdleTime;
}


////////////////////////
// Code object interface
////////////////////////
const string PlayerConnection::codeGetClassName() const
{
	return "PlayerConnection";
}

const string PlayerConnection::codeGetBasicInfo() const
{
	// for copying results into
	ostringstream os;

	os << "Desc #" << this->GetDescriptor() << ".";

	if (GetCharacter())
		os << " Char: " << GetCharacter()->getName().str() << ".";
	else
		os << " No char.";

	if (Account)
		os << " Account: " << Account->email << ".";
	else
		os << " No account.";

	os << " Pending: " << OutputLength << " outbound, " <<
		InputBuffer.length() << " inbound.";

	return os.str();
}

const string PlayerConnection::codeGetFullInfo() const
{
	// for copying results into
	ostringstream os;

	os << "Player connection:" << endl;

	os << "Opened on port " << this->GetPort() <<
		", descriptor #" << this->GetDescriptor() << "." << endl;

	os << "Connection state: " << this->ConnectedState <<
		", idle time: " << this->GetIdleTicks() << " (" << this->GetIdleSeconds() << " seconds)" << endl;

	if (GetCharacter())
		os << "Current character: " << GetCharacter()->getName().str() << endl;
	else
		os << "No current character." << endl;

	if (Account)
		os << "Account: " << Account->email <<
			", with " << Account->iref << " reference(s)." << endl;
	else
		os << "No account loaded." << endl;

	os << OutputLength << " bytes waiting to be written." << endl;
	os << InputBuffer.length() << " bytes in input buffer." << endl;

	return os.str();
}

