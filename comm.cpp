/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *           Low-level communication module             *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <crypt.h>
#include "mud.h"
#include "mxp.h"
#include "quests.h"

#include "paths.const.h"


/*
    * Socket and TCP/IP stuff.
	 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

#include <assert.h>

#include "connection_manager.h"
#include "connection.h"
#include "World.h"
#include "db_public.h"

// STL includes
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

const unsigned char echo_off_str  [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const unsigned char echo_on_str   [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const unsigned char go_ahead_str  [] = { IAC, GA, '\0' };

const unsigned char will_mxp_str  [] = { IAC, WILL, TELOPT_MXP, '\0' };
const unsigned char start_mxp_str [] = { IAC, SB, TELOPT_MXP, IAC, SE, '\0' };
const unsigned char do_mxp_str    [] = { IAC, DO, TELOPT_MXP, '\0' };
const unsigned char dont_mxp_str  [] = { IAC, DONT, TELOPT_MXP, '\0' };

void    send_auth  ( struct descriptor_data *d ) ;
void    read_auth  ( struct descriptor_data *d ) ;
void    start_auth ( struct descriptor_data *d );
void    save_sysdata  ( SYSTEM_DATA sys ) ;


/*
    * Global variables.
	 */
ACCOUNT_DATA *   first_account;
ACCOUNT_DATA *   last_account;

//DESCRIPTOR_DATA *   first_descriptor; /* First descriptor     */
//DESCRIPTOR_DATA *   last_descriptor;  /* Last descriptor      */
//DESCRIPTOR_DATA *   d_next;       /* Next descriptor in loop  */
int ListenerDescriptor; // the main listener socket
int ListenerPort; // the main listener port
FILE *          fpReserve;      /* Reserved file handle     */
bool            gGameRunning;        // Is the game running?
bool gBooting; // Are we currently booting?
bool            shell_shutdown = FALSE;  /* was it killed via a HUP? */
bool            wizlock;        /* Game is wizlocked        */
bool                newbielock;         /* Game does not accept newbies */
time_t              secBootTime;
HOUR_MIN_SEC        set_boot_time_struct;
HOUR_MIN_SEC *      set_boot_time;
struct tm *         new_boot_time;
struct tm           new_boot_struct;
char            str_boot_time[MAX_INPUT_LENGTH];
char            lastplayercmd[MAX_INPUT_LENGTH*2];
time_t          secCurrentTime; // Time of this pulse
long nextActionTime; // time for the next action to happen, in milliseconds

//int                 port;               /* Port number to be used       */
//int           control;        /* Controlling descriptor   */
//int           control2;       /* Controlling descriptor #2    */
//int           conclient;      /* MUDClient controlling desc   */
//int           conjava;        /* JavaMUD controlling desc */
//int           newdesc;        /* New descriptor       */
//fd_set            in_set;     /* Set of desc's for reading    */
//fd_set            out_set;        /* Set of desc's for writing    */
//fd_set            exc_set;        /* Set of desc's with errors    */
//int           maxdesc;

ConnectionManager * gConnectionManager;
World * gTheWorld;
QuestManager * gQuestManager;
DatabaseController * MasterDatabase;


bool fCopyOver = !FALSE; /* Are we doing a copyover type operation? */

/*
 * OS-dependent local functions.
 */
void	game_loop		( );


/*
 * Other local functions (OS-independent).
 */
char*   vnum_to_dotted (int vnum);
//bool    mail_account_password  (DESCRIPTOR_DATA* d, char* pword, char* file) ;
//bool	check_parse_name	 ( char *name, DESCRIPTOR_DATA *d ) ;
//bool    check_parse_email    ( char *email, DESCRIPTOR_DATA *d ) ;
//bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
//				    bool fConn ) );
//bool	check_playing		 ( DESCRIPTOR_DATA *d, char *name, bool kick ) ;
int	main			 ( int argc, char **argv ) ;
//void	nanny			 ( DESCRIPTOR_DATA *d, char *argument ) ;
//bool	flush_buffer		 ( DESCRIPTOR_DATA *d, bool fPrompt ) ;
//void	read_from_buffer	 ( DESCRIPTOR_DATA *d ) ;
void	stop_idling		 ( CHAR_DATA *ch ) ;
//void	free_desc		 ( DESCRIPTOR_DATA *d ) ;
//void	display_prompt		 ( DESCRIPTOR_DATA *d ) ;
//int	make_color_sequence	args( ( const char *col, char *buf,
//					DESCRIPTOR_DATA *d ) );
//void	set_pager_input		args( ( DESCRIPTOR_DATA *d,
//					char *argument ) );
//bool	pager_output		 ( DESCRIPTOR_DATA *d ) ;



void	mail_count		 ( CHAR_DATA *ch ) ;

// forward declarations
void do_save(Character * ch, const char* argument);
void do_help(Character * ch, const char* argument);

CHAR_DATA * lastplayer; /* who typed lastplayercmd? */

/* ***********  */

/* debug trace by Testaur - respects privacy of communication  */
/* also avoids spammy direction commands */

int tracelines=0;
int tracenum=0;
FILE *tracefile=NULL;

void traceclose(void)
{
	fclose(tracefile); /* close old tracefile */
}

void traceopen(void)
{
	char cmd[20];
	strcpy(cmd,"../tracef0.txt");
	cmd[9] |= (tracenum & 1); /* build name of new file */
	tracefile=fopen(cmd,"w");
	tracelines=0; /* no lines so far */
	fprintf(tracefile,"file %d\n",tracenum);
	fflush(tracefile);
	if(++tracenum==10)
		tracenum=0;
}


#define notrace(str) { if( strcmp(cmd,str)==0 ){traceit=0;} }

void trace(CHAR_DATA *ch, const char *cmdline)
{
	int traceit,c,i,j;
	char cmd[40];  /* no command of interest is this long */

	for (i=0; (c=cmdline[i]) > 0 && c < '!'; i++ )
		;  /* find start of command */
	j=0;
	while( (c=cmdline[i++]) > ' ' )
	{
		if(j<19)
		{
			if(c>='A' && c<='Z')
				c|=0x20; /* to lower case */
			cmd[j++]=c;
		}
	}
	cmd[j]=0;
	c=cmd[0];

	traceit=1;
	if((c>='a' && c<='z') || (c>='A' && c<='Z'))
	{/* alpha command */
		switch(c | 0x20) /* test lower case */
		{
			case 'c':
				notrace("clan");
				break;
			case 'd':
				notrace("d");
				break;
			case 'e':
				notrace("e");
				notrace("emote");
				break;
			case 'g':
				notrace("gtalk");
				break;
			case 'i':
				notrace("immtalk");
				break;
			case 'n':
				notrace("n");
				notrace("ne");
				notrace("nw");
				break;
			case 'p':
				break;
			case 'r':
				notrace("reply");
				notrace("retell");
				break;
			case 's':
				notrace("s");
				notrace("se");
				notrace("sw");
				notrace("say");
				break;
			case 't':
				notrace("tell");
				notrace("think");
				break;
			case 'u':
				notrace("u");
				break;
			case 'w':
				notrace("w");
				break;
		}
	}
	else
	{/* other command */
		if(
				c==0x27  /* single quote is 'say' */
				|| c==','
				|| c==';'
				|| c=='/'
				|| c==':'
				|| c=='>'
		  )
			traceit=0;
	}
	if(traceit)
	{/* since this is none of the above commands, it is safe to trace */

		strcpy(cmd, ((ch && ch->getName().length() > 0) ?
			ch->getName().c_str(): "<null>" ));
		if(tracefile)
		{
			fprintf(tracefile,"%s: %s\n",cmd,cmdline);
			fflush(tracefile);
		}
		if(++tracelines>=100)
		{/* full enough */
			traceclose();
			traceopen();
		}
	}
}
/* end of debug trace */


/* ***********  */


void SanityFail( Character * ch, const string & msg )
{
	assert(0);
}

void SanityObject( Character * ch, Object * obj )
{
	// Check the object contents

	Object * content = obj->first_content;
	while ( content )
	{
		Object * next = content->next_content;
		if ( next )
		{
			// make sure next's prev is us
			if ( next->prev_content != content )
			{
				SanityFail(ch, "next->prev != me (content)");
				return;
			}
		}
		else
		{
			// no next, make sure I'm the last
			if ( obj->last_content != content )
			{
				SanityFail(ch, "obj with null next is NOT last_content (content)");
				return;
			}
		}

		SanityObject(ch, content);
		content = content->next_content;
	}
}

// Sanity check function, implemented by Ksilyan
void SanityCheck( Character * ch )
{
	// if we're not debugging... don't do this stuff.
	#ifndef DEBUG
		return;
	#endif


	// OK... check this player's objects

	Object * obj = ch->first_carrying;
	while ( obj )
	{
		Object * next = obj->next_content;
		if ( next )
		{
			// make sure next's prev is us
			if ( next->prev_content != obj )
			{
				SanityFail(ch, "next->prev != me");
				return;
			}
		}
		else
		{
			// no next, so make sure I'm the last
			if ( ch->last_carrying != obj )
			{
				SanityFail(ch, "obj with null next is NOT last_carry");
				return;
			}
		}

		SanityObject(ch, obj);
		obj = obj->next_content;
	}
}


int main( int argc, char **argv )
{
	struct timeval now_time;

	/*
	 * Kill any outstanding alarm (copyover)
	 */
	set_alarm (0);

	/*
	 * Memory debugging if needed.
	 */
#if defined(MALLOC_DEBUG)
	malloc_debug( 2 );
#endif

	sysdata.NO_NAME_RESOLVING	= TRUE;
	sysdata.WAIT_FOR_AUTH	= TRUE;

	/*
	* Init time.
	*/
	gettimeofday( &now_time, NULL );
	secCurrentTime = (time_t) now_time.tv_sec;
	secBootTime = time(0);
	strcpy( str_boot_time, ctime( &secCurrentTime ) );

	/*
	* Init boot time.
	*/
	set_boot_time = &set_boot_time_struct;
	set_boot_time->manual = 0;

	new_boot_time = update_time(localtime(&secCurrentTime));
	/* Copies *new_boot_time to new_boot_struct, and then points
	new_boot_time to new_boot_struct again. -- Alty */
	new_boot_struct = *new_boot_time;
	new_boot_time = &new_boot_struct;
	new_boot_time->tm_mday += 1;
	if(new_boot_time->tm_hour > 12)
		new_boot_time->tm_mday += 1;
	new_boot_time->tm_sec = 0;
	new_boot_time->tm_min = 0;
	new_boot_time->tm_hour = 6;

	/* Update new_boot_time (due to day increment) */
	new_boot_time = update_time(new_boot_time);
	new_boot_struct = *new_boot_time;
	new_boot_time = &new_boot_struct;

	/* Set reboot time string for do_time */
	get_reboot_string();

	/*
	 * Reserve two channels for our use.
	 */
	if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
	{
		perror( NULL_FILE );
		exit( 1 );
	}
	if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
	{
		perror( NULL_FILE );
		exit( 1 );
	}

	/*
	 * Get the port number.
	 */
	ListenerPort = 5555;
	if ( argc > 1 )
	{
		if ( !is_number( argv[1] ) )
		{
			fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
			exit( 1 );
		}
		else if ( ( ListenerPort = atoi( argv[1] ) ) <= 1024 )
		{
			fprintf( stderr, "Port number must be above 1024.\n" );
			exit( 1 );
		}
	}

	// Initialize shared strings.
	log_string("Initializing shared strings...");
	extern void InitializeSharedStrings();
	InitializeSharedStrings();
	log_string("...done.");

	/*
	 * Run the game.
	 */

	gConnectionManager = new ConnectionManager();

	if ( argv[2] && argv[2][0] ) {
		//int control, control2;

		fCopyOver = TRUE;
		ListenerDescriptor = atoi(argv[3]);
		//control2 = atoi(argv[4]);

		gConnectionManager->AddListener(ListenerDescriptor);
		//gConnectionManager->AddListener(control2);
	} else {
		fCopyOver = FALSE;
	}

	first_account = last_account = NULL;

	log_string("Creating World Object");
	gTheWorld = new World();

        log_string("Initializing World Lua");
        if (gTheWorld->initializeLua() == false) {
            log_string("Failed to initialize world Lua. Aborting.");
            exit(1);
        }

        // Load up the quest manager
        log_string("Creating Quest Manager");
        gQuestManager = new QuestManager();

        if (!gQuestManager->initialize(QUESTS_FILE)) {
            log_string("Failed to initialize quest manager. Aborting.");
            exit(1);
        }

	/*#ifndef DEBUG
		log_string("Loading DBXML Database Controller");
		MasterDatabase = new DatabaseController();
		MasterDatabase->Initialize();
	#endif*/

	log_string("Booting Database");
	gBooting = true;
	boot_db();
	gBooting = false;
	log_string("Initializing socket");

	if ( !fCopyOver )
	{
		ListenerDescriptor = gConnectionManager->CreateListener( ListenerPort );
		if ( ListenerDescriptor == -1 )
		{
			log_string("Could not initiate listener sockets. Aborting.");
			exit(1);
		}
		//gConnectionManager->CreateListener( port + 1 );
	}

	sprintf( log_buf, "Legends of the Darkstone is ready on port %d.", ListenerPort );
	log_string( log_buf );

	/* Ok, it served its purpose */
	fCopyOver = FALSE;

	gGameRunning = true;

	traceopen( );
	game_loop( );
	traceclose( );

	/*
	* That's all, folks.
	*/

	log_string("Flushing output.");
	gConnectionManager->ForceFlushOutput();
	log_string("Output flushed.");

	// there's no reason for these to not exist - but hey, whatever
	if (gConnectionManager)
	{
		gTheWorld->LogString("Closing connection manager.");
		delete gConnectionManager;
	}
	gConnectionManager = NULL;
	if (MasterDatabase)
	{
		gTheWorld->LogString("Shutting down database.");
		delete MasterDatabase;
	}
	MasterDatabase = NULL;
	if (gTheWorld)
	{
		gTheWorld->LogString("Closing connection manager.");
		delete gTheWorld;
	}
	gTheWorld = NULL;

	log_string( "Closing down shared strings..." );
	extern void CloseSharedStrings();
	CloseSharedStrings();
	log_string( "...done." );

	log_string( "Normal termination of game." );

	exit( 0 );
	return 0;
}

#if 0 /* not used by anyone */
static void SegVio()
{
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    sprintf( buf, "%PC: %-20s room: %s", IS_NPC(ch) ? 'N' : ' ',
    		ch->name, vnum_to_dotted(ch->GetInRoom()->vnum) );
    log_string( buf );
  }
  exit(0);
}
#endif

/*
 * LAG alarm!							-Thoric
 */
static void caught_alarm(int temp)
{
    char buf[MAX_STRING_LENGTH];
    bug( "ALARM CLOCK!" );
    strcpy( buf, "Alas, the hideous malevolent entity known only as 'Lag' rises once more!\n\r" );
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
    /*if ( newdesc )
    {
	FD_CLR( newdesc, &in_set );
	FD_CLR( newdesc, &out_set );
	log_string( "clearing newdesc" );
    }*/
/*    game_loop( );
    close( control );

    log_string( "Normal termination of game." );
    exit( 0 );*/
}


long GetMillisecondsTime()
{
#ifdef unix
	struct timeval resultTimeval;
	gettimeofday( &resultTimeval, NULL );
	// First off... tv_sec is seconds since the Epoch
	// this is generally Jan 1st 1970.
	// now we don't want to multiply this by 1000, since
	// it might overflow... so first, subtract the seconds
	// from jan-1-1970 to jan-1-2000

	resultTimeval.tv_sec -= 946080000; // 30 years

	// convert to milliseconds
	return resultTimeval.tv_sec * 1000 + resultTimeval.tv_usec / 1000;
#else
	#ifdef WIN_32
	// Under Windows, the time is the time since system startup.
	// Knowing Billysoft, this'll never be more than 5 minutes *wink*
	// but seriously, we don't need to worry about overflow here
		return timeGetTime();
	#endif
#endif
}

void game_loop( )
{
	struct timeval	  last_time;
	time_t	last_check = 0;

	signal( SIGPIPE, SIG_IGN	  );
	signal( SIGALRM, caught_alarm );
	gettimeofday( &last_time, NULL );
	secCurrentTime = (time_t) last_time.tv_sec;

	nextActionTime = GetMillisecondsTime() + FRAME_TIME;

	/* Main loop */
	while ( gGameRunning )
	{
		long currentTime;

		currentTime = GetMillisecondsTime();

		struct timeval delayTime;
		long timeDifference = nextActionTime - currentTime;

		if (timeDifference > 0)
		{
			delayTime.tv_sec = 0;
			delayTime.tv_usec = 0;
			while (timeDifference >= 1000)
			{
				delayTime.tv_sec += 1;
				timeDifference -= 1000;
			}
			delayTime.tv_usec = timeDifference * 1000;
		}
		else
		{
			delayTime.tv_sec = 0;
			delayTime.tv_usec = 0;
		}

		// Only process sockets if the poll succeeded.
		if ( gConnectionManager->PollSockets( delayTime ) == false )
		{
			gTheWorld->LogBugString("There was an error polling the sockets!");
		}
		else
		{
			if ( gConnectionManager->ProcessActiveSockets() == false )
				gTheWorld->LogBugString("There was an error processing the selected sockets!");
		}

		// need to handle waiting input lines here

		currentTime = GetMillisecondsTime();

		while ( currentTime >= nextActionTime )
		{
			/*
			 * Run the game logic.
			 */
			//printf("Pulse\n\r");
			gTheWorld->TimeUnit();

			// Update next tick time.
			nextActionTime += FRAME_TIME;

			// Update current time.
			currentTime = GetMillisecondsTime();

			// a little kludgy, but whatever
			gettimeofday( &last_time, NULL );
			secCurrentTime = (time_t) last_time.tv_sec;
		}


		if ( last_check+(10*60) < secCurrentTime )
		{ /* check every 5 minutes, or so */
			ACCOUNT_BAN_DATA* pban;

			last_check = secCurrentTime;

			for ( pban = first_account_ban; pban; pban = pban->next )
			{
				if ( !IS_SET(pban->flags, ACCOUNT_WAITING) )
					continue;
				if ( pban->secBanTime + (60 * 60 * 24) < secCurrentTime)
				{ /* 24 hours, or so */
					ACCOUNT_BAN_DATA* ptmp;

					sprintf(log_buf, "Waiting account %s died of old age.  Not too late to approve.",
						pban->address);
					log_string(log_buf);

					UNLINK(pban, first_account_ban, last_account_ban, next, prev);
					ptmp = pban->prev;
					DISPOSE(pban);
					pban = ptmp;
					save_account_banlist();
				}
			}
		}
	}

	if ( shell_shutdown )
	{
		itorSocketId itor;
		for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
		{
			// we know that the world's player connection list only holds player connections IDs,
			// so we can safely cast it to PlayerConnection*
			PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

			d->SendText("\n\n\n******\n\n\nMUD HAS BEEN REBOOTED FROM THE SHELL\n\n\n******\n\n\n");
		}

		/* Save all characters before booting. */
		{
			CHAR_DATA* vch;
			AREA_DATA* tarea;
			for ( vch = first_char; vch; vch = vch->next )
				if ( !IS_NPC( vch ) )
					save_char_obj( vch );

				for ( tarea = first_build; tarea; tarea = tarea->next )
				{
					char buf[MAX_STRING_LENGTH];

					if ( !IS_SET(tarea->status, AREA_LOADED) )
						continue;

					sprintf(buf, "%s%s", BUILD_DIR, tarea->filename);
					fold_area(tarea, buf, FALSE);
				}
		}
	}
	return;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( !ch
    ||   !ch->GetConnection()
    ||    ch->GetConnection()->ConnectedState != CON_PLAYING
    ||   !ch->WasInRoomId
    ||    ch->GetInRoom() != get_room_index( ROOM_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, RoomMap[ch->WasInRoomId] );
    ch->WasInRoomId = 0;
    act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char_nocolor( const char *txt, CHAR_DATA *ch )
{
	if ( !ch )
	{
		bug( "Send_to_char: NULL *ch" );
		return;
	}
	ch->sendText(txt, false);

	return;
}
/*
* Same as above, but converts &color codes to ANSI sequences..
*/
void send_to_char_color( const char *txt, Character *ch )
{
	ch->sendText(txt, true);
}
void write_to_pager( PlayerConnection *d, const char *txt, int length )
{
	/*if ( length <= 0 )
		length = strlen(txt);
	if ( length == 0 )
		return;
	if ( !d->pagebuf )
	{
		d->pagesize = MAX_STRING_LENGTH;
		CREATE( d->pagebuf, char, d->pagesize );
	}
	if ( !d->pagepoint )
	{
		d->pagepoint = d->pagebuf;
		d->pagetop = 0;
		d->pagecmd = '\0';
	}
	if ( d->pagetop == 0 && !d->fcommand )
	{
		d->pagebuf[0] = '\n';
		d->pagebuf[1] = '\r';
		d->pagetop = 2;
	}
	while ( d->pagetop + length >= d->pagesize )
	{
		if ( d->pagesize > 32000 )
		{
			bug( "Pager overflow.  Ignoring.\n\r" );
			d->pagetop = 0;
			d->pagepoint = NULL;
			DISPOSE(d->pagebuf);
			d->pagesize = MAX_STRING_LENGTH;
			return;
		}
		d->pagesize *= 2;
		RECREATE(d->pagebuf, char, d->pagesize);
	}
	strncpy(d->pagebuf+d->pagetop, txt, length);
	d->pagetop += length;
	d->pagebuf[d->pagetop] = '\0';
	return;*/
}

void send_to_pager_nocolor( const char *txt, CHAR_DATA *ch )
{
	if ( !ch )
	{
		bug( "Send_to_pager: NULL *ch" );
		return;
	}
	if ( txt && ch->GetConnection() )
	{
		PlayerConnection *d = ch->GetConnection();

		ch = d->GetOriginalCharacter();
		if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
		{
			ch->sendText(txt, false);
			return;
		}
		write_to_pager(d, txt, 0);
	}
	return;
}
void send_to_pager_color( const char *txt, CHAR_DATA *ch )
{
	PlayerConnection *d;
	const char *colstr;
	const char *prevstr = txt;
	char colbuf[20];
	int ln;

	if ( !ch )
	{
		bug( "Send_to_pager_color: NULL *ch" );
		return;
	}
	if ( !txt || !ch->GetConnection() )
		return;
	d = ch->GetConnection();
	ch = d->GetOriginalCharacter();

	// Ksilyan: it goes to ch no matter what... pager is later
	//if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
	//{
		ch->sendText(txt, true);
		//send_to_char_color(txt, ch);
		return;
	//}
	while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
	{
		if ( colstr > prevstr )
			write_to_pager(d, prevstr, (colstr-prevstr));
		ln = make_color_sequence(colstr, colbuf, d);
		if ( ln < 0 )
		{
			prevstr = colstr+1;
			break;
		}
		else if ( ln > 0 )
			write_to_pager(d, colbuf, ln);
		prevstr = colstr+2;
	}
	if ( *prevstr )
		write_to_pager(d, prevstr, 0);
	return;
}

/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.     -Thoric
 */
const char *myobj( OBJ_DATA *obj )
{
	if ( !str_prefix("a ", obj->shortDesc_.c_str()) )
		return obj->shortDesc_.c_str() + 2;
	if ( !str_prefix("an ", obj->shortDesc_.c_str()) )
		return obj->shortDesc_.c_str() + 3;
	if ( !str_prefix("the ", obj->shortDesc_.c_str()) )
		return obj->shortDesc_.c_str() + 4;
	if ( !str_prefix("some ", obj->shortDesc_.c_str()) )
		return obj->shortDesc_.c_str() + 5;
	return obj->shortDesc_.c_str();
}

#if 0 /* Moved to color.h */
void set_char_color( sh_int AType, CHAR_DATA *ch )
{
    char buf[16];
    CHAR_DATA *och;

    if ( !ch || !ch->desc )
      return;

    och = (ch->desc->original ? ch->desc->original : ch);
    if ( !IS_NPC(och) && IS_SET(och->act, PLR_ANSI) )
    {
	if ( AType == 7 )
	  strcpy( buf, "\033[m" );
	else
	  sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
	        (AType > 15 ? "5;" : ""), (AType & 7)+30);
	write_to_buffer( ch->desc, buf, strlen(buf) );
    }
    return;
}

void set_pager_color( sh_int AType, CHAR_DATA *ch )
{
    char buf[16];
    CHAR_DATA *och;

    if ( !ch || !ch->desc )
      return;

    och = (ch->desc->original ? ch->desc->original : ch);
    if ( !IS_NPC(och) && IS_SET(och->act, PLR_ANSI) )
    {
	if ( AType == 7 )
	  strcpy( buf, "\033[m" );
	else
	  sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
	        (AType > 15 ? "5;" : ""), (AType & 7)+30);
	send_to_pager( buf, ch );
	ch->desc->pagecolor = AType;
    }
    return;
}
#endif /* moved to color.c */

/* source: EOD, by John Booth <???> */

void ch_printf_nocolor(CHAR_DATA *ch, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_char_nocolor(buf, ch);
}
void ch_printf(CHAR_DATA *ch, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_char(buf, ch);
}
void pager_printf(CHAR_DATA *ch, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_pager(buf, ch);
}

/*  From Erwin  */

void log_printf(const char *fmt, ...)
{
  char buf[MAX_STRING_LENGTH*2];
  va_list args;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  log_string(buf);
}

const char *obj_short( OBJ_DATA *obj )
{
	static char buf[MAX_STRING_LENGTH];

	if ( obj->count > 1 )
	{
		sprintf( buf, "%s (%d)", obj->shortDesc_.c_str(), obj->count );
		return buf;
	}
	return obj->shortDesc_.c_str();
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
		 const void *arg1, const void *arg2)
{
  static const char * const he_she  [] = { "it",  "he",  "she" };
  static const char * const him_her [] = { "it",  "him", "her" };
  static const char * const his_her [] = { "its", "his", "her" };
  static char buf[MAX_STRING_LENGTH];
  char fname[MAX_INPUT_LENGTH];
  char *point = buf;
  const char *str = format;
  const char *i;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;

  int capitalize_mode = 0;
  char c;

  while ( (c=*str) != '\0' )
  {
    if ( c != '$' )
    {
      if(capitalize_mode<2) /* still undecided about capitalization */
      {
        if(capitalize_mode == 1)
        {/* expecting color code */
          capitalize_mode = 0; /* colorcode passed */
        }
        else
        {
          if(c=='&')
            capitalize_mode=1; /* color code follows */
          else
          {
            if(c>' ')
              capitalize_mode=2; /* no caps needed */
          }
        }
      }

      *point++ = *str++;
      continue;
    }
    ++str;
    c=*str;
    if ( !arg2 && *str >= 'A' && *str <= 'Z' )
    {
      bug( "Act: missing arg2 for code $%c:", *str );
      bug( format );
      i = " <@@@> ";
    }
    else
    {
      switch ( c )
      {
      default:  bug( "Act: bad code %c.", c );
		i = " <@@@> ";						break;
      case 't': i = (char *) arg1;					break;
      case 'T': i = (char *) arg2;					break;
      case 'n': i = (to ? PERS( ch, to) : NAME( ch));			break;
      case 'N': i = (to ? PERS(vch, to) : NAME(vch));			break;
      case 'e': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->getName().c_str(),
		      ch->sex);
		  i = "it";
		}
		else
		  i = he_she [URANGE(0,  ch->sex, 2)];
		break;
      case 'E': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->getName().c_str(),
		      vch->sex);
		  i = "it";
		}
		else
		  i = he_she [URANGE(0, vch->sex, 2)];
		break;
      case 'm': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->getName().c_str(),
		      ch->sex);
		  i = "it";
		}
		else
		  i = him_her[URANGE(0,  ch->sex, 2)];
		break;
      case 'M': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->getName().c_str(),
		      vch->sex);
		  i = "it";
		}
		else
		  i = him_her[URANGE(0, vch->sex, 2)];
		break;
      case 's': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->getName().c_str(),
		      ch->sex);
		  i = "its";
		}
		else
		  i = his_her[URANGE(0,  ch->sex, 2)];
		break;
      case 'S': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->getName().c_str(),
		      vch->sex);
		  i = "its";
		}
		else
		  i = his_her[URANGE(0, vch->sex, 2)];
		break;
      case 'q': i = (to == ch) ? "" : "s";				break;
      case 'Q': i = (to == ch) ? "your" :
		    his_her[URANGE(0,  ch->sex, 2)];			break;
      case 'p': i = (!to || can_see_obj(to, obj1)
		  ? obj_short(obj1) : "something");			break;
      case 'P': i = (!to || can_see_obj(to, obj2)
		  ? obj_short(obj2) : "something");			break;
      case 'd':
        if ( !arg2 || ((char *) arg2)[0] == '\0' )
          i = "door";
        else
        {
          one_argument((char *) arg2, fname);
          i = fname;
        }
        break;
      }
    }
    if(i==NULL) /* Testaur -  be sure that referent was actually supplied! */
    {
      bug("act_string: no referent for $%c",*str);
      i="<\?\?\?>";
    }
    ++str;
    while ( (c = *i) != '\0' )
    {
      if(capitalize_mode < 2)
      {
        if(capitalize_mode==1)
        {
          capitalize_mode=0;
        }
        else
        {
          if(c=='&')
            capitalize_mode=1; /* colorcode follows */
          else
          {
            if(c>' ')
            {
              c=UPPER(c);
              capitalize_mode=2; /* resolved! */
            }
          }
        }
      }
      *point=c;
      ++point, ++i;
    }
  }
  strcpy(point, "\n\r");
  return buf;
}
#undef NAME

int act_check_sneak = 0;

void act( sh_int AType, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
	char *txt;
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *)arg2;
	CHAR_DATA *targ = (CHAR_DATA *)arg1;

	/*
	 * Discard null and zero-length messages.
	 */
	if ( !format || format[0] == '\0' )
		return;

	if ( !ch )
	{
		bug( "Act: null ch. (%s)", format );
		return;
	}

	if ( !ch->GetInRoom() )
	{
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "Null in_room for char %s acting %s.", ch->getName().c_str(), format);
		log_string(buf);
		to = NULL;
	}
	else if ( type == TO_CHAR )
		to = ch;
	else if ( type == TO_TARG )
		to = targ;
	else
		to = ch->GetInRoom()->first_person;

	/*
	 * ACT_SECRETIVE handling
	 */
	if ( IS_NPC(ch) && IS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR )
		return;

	if ( type == TO_VICT )
	{
		if ( !vch )
		{
			bug( "Act: null vch with TO_VICT." );
			bug( "%s (%s)", ch->getName().c_str(), format );
			return;
		}
		if ( !vch->GetInRoom() )
		{
			bug( "Act: vch in NULL room!" );
			bug( "%s -> %s (%s)", ch->getName().c_str(), vch->getName().c_str(), format );
			return;
		}
		to = vch;
		/*to = vch->GetInRoom()->first_person;*/
	} else if (type == TO_WORLD) {
		to = first_char;
	}

	if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && type != TO_WORLD && to && type != TO_TARG )
	{
		OBJ_DATA *to_obj;

		txt = act_string(format, NULL, ch, arg1, arg2);
		if ( IS_SET(to->GetInRoom()->progtypes, ACT_PROG) )
			rprog_act_trigger(txt, to->GetInRoom(), ch, (OBJ_DATA *)arg1, (void *)arg2);
		for ( to_obj = to->GetInRoom()->first_content; to_obj;
		to_obj = to_obj->next_content )
			if ( to_obj->pIndexData && IS_SET(to_obj->pIndexData->progtypes, ACT_PROG) )
				oprog_act_trigger(txt, to_obj, ch, (OBJ_DATA *)arg1, (void *)arg2);
	}

	/* Anyone feel like telling me the point of looping through the whole
	room when we're only sending to one char anyways..? -- Alty */
	for ( ; to; to = (type == TO_CHAR || type == TO_VICT || type == TO_TARG)
		? NULL : (type == TO_WORLD) ? to->next : to->next_in_room )
	{
		if ( type == TO_WORLD && IS_NPC(to) ) {
			continue;
		}

		if ((!to->GetConnection()
			&& (  IS_NPC(to) && !IS_SET(to->pIndexData->progtypes, ACT_PROG) ))
			||	 !IS_AWAKE(to) || to->position == POS_MEDITATING )
			continue;

		if ( type == TO_CHAR && to != ch )
			continue;
		if ( type == TO_TARG && ( to != targ || to == targ ) )
			continue;
		if ( type == TO_VICT && ( to != vch || to == ch ) )
			continue;
		if ( type == TO_ROOM && to == ch )
			continue;
		if ( type == TO_NOTVICT && (to == ch || to == vch) )
			continue;
		if ( type == TO_WORLD && (to == ch || to == vch) )
			continue;

		if ( act_check_sneak  /* kludgy external parameter! */
			&& to != ch
			&& IS_AFFECTED( ch, AFF_SNEAK )
			&& get_trust(to) < LEVEL_IMMORTAL
			)
			continue;

		txt = act_string(format, to, ch, arg1, arg2);
		if (to->GetConnection())
		{
			set_char_color(AType, to);
			to->sendText(txt, false);
			/*send_to_char(txt, ch); */
		}
		if (MOBtrigger)
		{
			/* Note: use original string, not string with ANSI. -- Alty */
			mprog_act_trigger( txt, to, ch, (OBJ_DATA *)arg1, (void *)arg2 );
		}
	}

	/*
	 * Crystal ball television stuff.
	 * - Ksilyan
	*/

	if ( ( ( type == TO_ROOM ) || ( type == TO_NOTVICT ) )
		&& ( ch->GetInRoom() && ch->GetInRoom()->TelevisionObjectId != 0 ) )
	{
		char *txt;

		txt = act_string(format, to, ch, arg1, arg2);

		television_talk(ObjectMap[ch->GetInRoom()->TelevisionObjectId], txt);
	}

	MOBtrigger = TRUE;
	return;
}

void do_namechange(CHAR_DATA *ch, const char* argument)
{
    char fname[1024];
    char bname[1024];
    struct stat fst;
    CHAR_DATA *vch;
    CHAR_DATA *tmp;
    char person[MAX_INPUT_LENGTH];
    std::string newName;

    if ( !argument || argument[0] == '\0' ) {
        send_to_char("Usage: namechange <old> <new>\r\n", ch);
        return;
    }

    newName = one_argument(argument, person);

    if ( !(vch = get_char_world(ch, person)) ) {
        ch_printf(ch, "Can't find %s anywhere.\r\n", person);
        return;
    }
#if 0
    else if ( vch->level > 5 ) {
        send_to_char("namechange only works for players level 1 - 5.\r\n", ch);
        return;
    }
#endif
    else if ( IS_NPC(vch) ) {
        send_to_char("That's a mob...\r\n", ch);
    }

    if ( newName.length() == 0 ) {
        do_namechange(ch, "");
        return;
    }

    newName[0] = UPPER(newName[0]);

    if (!gConnectionManager->CheckParseName(newName.c_str(), NULL))
    {
        send_to_char("Illegal name, try another.\n\r", ch);
        return;
    }

    for ( tmp = first_char; tmp; tmp = tmp->next )
    {
        if (tmp->getName() == newName)
            break;
    }

    if ( tmp )
    {
        send_to_char("That name is already taken.  Please choose another.\n\r", ch);
        return;
    }

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(newName[0]),
                        capitalize( newName.c_str() ) );

    if ( stat( fname, &fst ) != -1 )
    {
        send_to_char("That name is already taken.  Please choose another.\n\r", ch);
        return;
    }

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(vch->getName().c_str()[0]),
                        capitalize( vch->getName().c_str() ) );
    sprintf( bname, "%s%c/%s", BACKUP_DIR, tolower(vch->getName().c_str()[0]),
                        capitalize( vch->getName().c_str() ) );


    if ( rename(fname, bname) == -1 ) {
        send_to_char("Strange error in rename -- aborting!\n", ch);
        return;
    }

	if (vch->GetConnection() && vch->GetConnection()->Account) /* KSILYAN- */
	{   /* small thing to update account's character list */
		char buf[MAX_INPUT_LENGTH];
		// remove old name from account list
		//strcpy(buf, remove_name(vch->getName().c_str(), vch->desc->account->characters));

		// add new name to account list
		sprintf(buf, "%s %s", remove_name(vch->getName().c_str(), vch->GetConnection()->Account->characters), newName.c_str());

		// assign new account list
		STRFREE(vch->GetConnection()->Account->characters);
		vch->GetConnection()->Account->characters = STRALLOC(buf);
		write_account_data(vch->GetConnection()->Account);
	}
	else
	{
		send_to_char("Could not change their account character list (no descriptor!)\n\r", ch);
	}

    vch->setName(newName);
    send_to_char("Their name has been changed.\n\r", ch);
    ch_printf(vch, "Your name has been changed to [%s].\r\n", vch->getName().c_str());
    do_save(vch, "");
    return;
}

char *default_prompt( CHAR_DATA *ch )
{
	static char buf[60];

	strcpy(buf, "&w<&Y%hhp ");
	if ( IS_VAMPIRE(ch) )
		strcat(buf, "&R%bbp");
	else
		strcat(buf, "&C%mm");
	strcat(buf, " &G%vmv&w> ");
	if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
		strcat(buf, "%i%R");
	return buf;
}

int getcolor(char clr)
{
	static const char colors[17] = "xrgObpcwzRGYBPCW";
	int r;

	for ( r = 0; r < 16; r++ )
		if ( clr == colors[r] )
			return r;
	return -1;
}

void display_prompt( PlayerConnection *d )
{
	CHAR_DATA *victim;
	CHAR_DATA *ch = d->GetCharacter();
	CHAR_DATA *och = d->GetOriginalCharacter();
	bool ansi;
	const char *prompt;
	int percent;

	if ( !ch )
	{
		bug( "display_prompt: NULL ch" );
		return;
	}

	ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));

	if ( !IS_NPC(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt
			&&	 ch->pcdata->subprompt[0] != '\0' )
		prompt = ch->pcdata->subprompt;
	else
	{
		if ( IS_NPC(ch) || ch->pcdata->prompt_.length() == 0 )
			prompt = default_prompt(ch);
		else
			prompt = ch->pcdata->prompt_.c_str();
	}

	ostringstream promptBuf;

	if ( ansi )
	{
		promptBuf << "\033[m";
		d->prevcolor = 0x07;
	}

	for ( ; *prompt; prompt++ )
	{
		/*
		 * '&' = foreground color/intensity bit
		 * '^' = background color/blink bit
		 * '%' = prompt commands
		 * Note: foreground changes will revert background to 0 (black)
		 */
		if ( *prompt != '&' && *prompt != '^' && *prompt != '%' )
		{
			promptBuf << *prompt;
			continue;
		}

		++prompt;

		// Are we at the end of the prompt?
		if ( !*prompt )
			break;

		if ( *prompt == *(prompt-1) )
		{
			promptBuf << *prompt;
			continue;
		}

		switch(*(prompt-1))
		{
			default:
				bug( "Display_prompt: bad command char '%c'.", *(prompt-1) );
				break;
			case '&':
			case '^':
			{
				int len = make_color_sequence(&prompt[-1], promptBuf, d);
				if ( len < 0 )
					--prompt;
				/*else if ( stat > 0 )
					pbuf += stat;*/
				break;
			}

			case '%':
				switch(*prompt)
				{
					case '%':
						promptBuf << '%';
						break;
					case 'a':
						if ( ch->level >= 10 )
							promptBuf << ch->alignment;
						else
						{
							if ( IS_GOOD(ch) )
								promptBuf << "good";
							else if ( IS_EVIL(ch) )
								promptBuf << "evil";
							else
								promptBuf << "neutral";
						}
						break;

					case 'A':
						if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AFK) )
							promptBuf << "[afk]";
						break;

					case 'b':
						if ( IS_VAMPIRE(ch) )
							promptBuf << ch->pcdata->condition[COND_BLOODTHIRST];
						else
							promptBuf << 0;
						break;
					case 'B':
						if ( IS_VAMPIRE(ch) )
							promptBuf << ch->level + 10;
						else
							promptBuf << 0;
						break;
                                        case 'd':
                                                {
                                                   int drunk =  ch->pcdata->condition[COND_DRUNK];
                                                   if( drunk > 90 )
                                                   {
                                                      promptBuf << "blacked out";
                                                   }
                                                   else if( drunk > 80 )
                                                   {
                                                      promptBuf << "plastered";
                                                   }
                                                   else if( drunk > 70 )
                                                   {
                                                      promptBuf << "sloshed";
                                                   }
                                                   else if( drunk > 60 )
                                                   {
                                                      promptBuf << "crocked";
                                                   }
                                                   else if( drunk > 50 )
                                                   {
                                                      promptBuf << "wasted";
                                                   }
                                                   else if( drunk > 40 )
                                                   {
                                                      promptBuf << "drunk";
                                                   }
                                                   else if( drunk > 30 )
                                                   {
                                                      promptBuf << "glazed";
                                                   }
                                                   else if( drunk > 20 )
                                                   {
                                                      promptBuf << "tipsy";
                                                   }
                                                   else if( drunk > 10 )
                                                   {
                                                      promptBuf << "buzzed";
                                                   }
                                                   else if( drunk > 0 )
                                                   {
                                                      promptBuf << "flushed";
                                                   }
                                                   else
                                                   {
                                                      promptBuf << "sober";
                                                   }
                                                   break;
                                                }
					case 'f':
						if ( ch->pcdata->deity )
							promptBuf << ch->pcdata->favor;
						else
						{
							promptBuf << "no favor";
						}
						break;
					case 'g':
						promptBuf << ch->gold;
						break;
                                        case 'G'://hunger
                                                {
                                                   if ( IS_VAMPIRE(ch) )
                                                      break;
                                                   int hunger = ch->pcdata->condition[COND_FULL];
                                                   if( hunger > 47 ) //peckish famished hungry ravenous starved
                                                      promptBuf << "satiated";
                                                   else if( hunger > 40 )
                                                      promptBuf << "peckish";
                                                   else if( hunger > 30 )
                                                      promptBuf << "famished";
                                                   else if( hunger > 20 )
                                                      promptBuf << "hungry";
                                                   else if( hunger > 10 )
                                                      promptBuf << "ravenous";
                                                   else
                                                      promptBuf << "starved";
                                                   break;
                                                }
					case 'h':
						promptBuf << ch->hit;
						break;
					case 'H':
						promptBuf << ch->max_hit;
						break;
					case 'i':
						if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_WIZINVIS)) ||
								(IS_NPC(ch) && IS_SET(ch->act, ACT_MOBINVIS)) )
							promptBuf << "(Invis " << ((IS_NPC(ch) ? ch->mobinvis : ch->pcdata->wizinvis)) << ")";
						else if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
							promptBuf << "(Invis)";
						break;
					case 'I':
						promptBuf << (IS_NPC(ch) ? (IS_SET(ch->act, ACT_MOBINVIS) ? ch->mobinvis : 0)
								: (IS_SET(ch->act, PLR_WIZINVIS) ? ch->pcdata->wizinvis : 0));
						break;
					case 'k':
						/* Golias - Provides name of mob if fighting*/
						if ( (victim = ch->GetVictim()) )
							promptBuf << victim->getShort().c_str();
						else
							promptBuf << "--";
						break;
					case 'K':
						{
							if ( ch->GetVictim() )
							{
								victim =  ch->GetVictim();
								if ( victim->max_hit > 0 )
									percent = ( 100 * victim->hit ) / victim->max_hit;
								else
									percent = -1;

								// Added inanimate mob text    -Ksilyan
								if ( IS_NPC(victim) && IS_SET(victim->act, ACT_INANIMATE) )
								{
									if ( percent >= 100 ) promptBuf << "Perfect condition";
									else if ( percent >=  90 ) promptBuf << "Slightly scratched";
									else if ( percent >=  80 ) promptBuf << "A few dents";
									else if ( percent >=  70 ) promptBuf << "Quite dented";
									else if ( percent >=  60 ) promptBuf << "Damaged";
									else if ( percent >=  50 ) promptBuf << "Very damaged";
									else if ( percent >=  40 ) promptBuf << "Holed in places";
									else if ( percent >=  30 ) promptBuf << "Beginning to break";
									else if ( percent >=  20 ) promptBuf << "Giving way";
									else if ( percent >=  10 ) promptBuf << "Almost in pieces";
									else                       promptBuf << "BREAKING";
								}
								else
								{
									if ( percent >= 100 ) promptBuf << "Perfect health";
									else if ( percent >=  90 ) promptBuf << "Slightly scratched";
									else if ( percent >=  80 ) promptBuf << "A few bruises";
									else if ( percent >=  70 ) promptBuf << "Some cuts";
									else if ( percent >=  60 ) promptBuf << "Several wounds";
									else if ( percent >=  50 ) promptBuf << "Nasty wounds";
									else if ( percent >=  40 ) promptBuf << "Bleeding freely";
									else if ( percent >=  30 ) promptBuf << "Covered in blood";
									else if ( percent >=  20 ) promptBuf << "Leaking guts";
									else if ( percent >=  10 ) promptBuf << "Almost dead";
									else					   promptBuf << "DYING";
								}
							}
							else
								promptBuf << "--";
						}
						break;

					case 'm':
						if ( IS_VAMPIRE(ch) )
							promptBuf << 0;
						else
							promptBuf << ch->mana;
						break;

					case 'M':
						if ( IS_VAMPIRE(ch) )
							promptBuf << 0;
						else
							promptBuf << ch->max_mana;
						break;

                                        case 'N':
                                                promptBuf << ch->GetInRoom()->area->name;
                                                break;
					case 'o':
                        if ( time_info.hour == 0 )
	                    	promptBuf << "12AM";
                        else if ( time_info.hour == 12 )
	                    	promptBuf << "12PM";
	                    else if ( time_info.hour < 12 )
	                    {
	                        promptBuf << time_info.hour;
					        promptBuf << "AM";
					    }
	                    if ( time_info.hour > 12 )
	                    {
	                        promptBuf << time_info.hour % 12;
					        promptBuf << "PM";
					    }
						break;

					case 'O':
	                    promptBuf << time_info.hour;
						break;

					case 'r':
						{
							if ( IS_IMMORTAL(och) )
							{
								promptBuf << vnum_to_dotted(ch->GetInRoom()->vnum);
								break;
							}
						}

					case 'R':
						{
							if ( IS_IMMORTAL(och) && IS_SET(och->act, PLR_ROOMVNUM) )
							{
								promptBuf << "<#" << vnum_to_dotted(ch->GetInRoom()->vnum) << ">";
								break;
							}
						}

					case 's':
						promptBuf << sector_number_to_name(ch->GetInRoom()->sector_type);
						break;

					case 't':
						if ( ch->GetWaitSeconds() > 0 )
							promptBuf << ch->GetWaitSeconds();
						else
							promptBuf << 0;
						break;
                                        case 'T'://Thirst
                                                {
                                                   int thirst = ch->pcdata->condition[COND_THIRST];
                                                   
                                                   if ( IS_VAMPIRE(ch) )
                                                      break;
                                                   if( thirst < 10 )
                                                      promptBuf << "DYING of thirst";
                                                   else if( thirst < 25 )
                                                      promptBuf << "dehydrated";
                                                   else if( thirst < 40 )
                                                      promptBuf << "parched";
                                                   else if( thirst < 60 )
                                                      promptBuf << "dry-mouthed";
                                                   else if( thirst < 80 )
                                                      promptBuf << "thirsty";
                                                   else
                                                      promptBuf << "quenched";
                                                   break;
                                                }
					case 'u':
						{
							CHAR_DATA *vch;
							int users = 0;

							for ( vch = first_char; vch; vch = vch->next )
							{
								if ( vch->GetConnection() && can_see(ch, vch) )
									users++;
							}

							promptBuf << users;

							break;
						}
					case 'U':
						promptBuf << sysdata.maxplayers;
						break;
					case 'v':
						promptBuf << ch->move;
						break;
					case 'V':
						promptBuf << ch->max_move;
						break;
					case 'w': // weight that ch can carry
						promptBuf << ch->carry_weight;
						break;
					case 'W': // max weight
						promptBuf << can_carry_w(ch);
						break;
					case 'x':
						promptBuf << ch->exp;
						break;
					case 'X':
						promptBuf << exp_level(ch, ch->level+1) - ch->exp;
						break;
					case 'Z':
						promptBuf
								<< (IS_AFFECTED(ch, AFF_INVISIBLE)	 ? 'I' : '-')
								<< (IS_AFFECTED(ch, AFF_SANCTUARY)	 ? 'S' : '-')
								<< (IS_AFFECTED(ch, AFF_SNEAK)		 ? 's' : '-')
								<< (IS_AFFECTED(ch, AFF_HIDE)		 ? 'H' : '-')
								<< (IS_AFFECTED(ch, AFF_FLYING) 	 ? 'Y' : '-')
								<< (IS_AFFECTED(ch, AFF_TRUESIGHT)	 ? 'T' : '-')
								<< (IS_AFFECTED(ch, AFF_FIRESHIELD)      ? 'F' : '-')
								<< (IS_AFFECTED(ch, AFF_SHOCKSHIELD)     ? 'X' : '-')
								<< (IS_AFFECTED(ch, AFF_ICESHIELD)	 ? 'C' : '-');
						break;
					case 'z':
						promptBuf
								<< (IS_AFFECTED(ch, AFF_INVISIBLE)	 ? "I" : "")
								<< (IS_AFFECTED(ch, AFF_SANCTUARY)	 ? "S" : "")
								<< (IS_AFFECTED(ch, AFF_SNEAK)		 ? "s" : "")
								<< (IS_AFFECTED(ch, AFF_HIDE)		 ? "H" : "")
								<< (IS_AFFECTED(ch, AFF_FLYING) 	 ? "Y" : "")
								<< (IS_AFFECTED(ch, AFF_TRUESIGHT)	 ? "T" : "")
								<< (IS_AFFECTED(ch, AFF_FIRESHIELD)      ? "F" : "")
								<< (IS_AFFECTED(ch, AFF_SHOCKSHIELD)     ? "X" : "")
								<< (IS_AFFECTED(ch, AFF_ICESHIELD)	 ? "C" : "");
						break;
					case '!': //Percent hits
                                                /*
							if ( ch->level < LEVEL_HERO_MIN )
								promptBuf << "go jump in a lake";
							else
								promptBuf << vnum_to_dotted(ch->GetInRoom()->vnum);
                                                 */
                                                promptBuf << ( ch->hit * 100 / ch->max_hit );
                                                  break;
                                        case '@': //Percent mana
                                                promptBuf << ( ch->mana * 100 / ch->max_mana );
                                                break;
                                        case '#': //Percent move
                                                promptBuf << ( ch->move * 100 / ch->max_move );
                                                break;
                                        case '*': //Percent blood
                                                promptBuf << ( ch->pcdata->condition[COND_BLOODTHIRST] * 100 / (ch->level+10));
                                                break;

				}

				break;
		}
	}
	d->SendText( promptBuf.str() );
	//write_to_buffer(d, buf, (pbuf-buf));
	return;
}

int make_color_sequence(const char * col, ostringstream & os, PlayerConnection * d)
{
	char buf[50];

	int len = make_color_sequence(col, buf, d);

	if ( len > 0 )
		os << buf;

	return len;
}

//This function just gave me cancer
int make_color_sequence(const char *col, char *buf, PlayerConnection *d)
{
	int ln;
	const char *ctype = col;
	unsigned char cl;
	CHAR_DATA *och;
	bool ansi;

	och = d->GetOriginalCharacter();
	ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));
	col++;
	if ( !*col )
		ln = -1;
	else if ( *ctype != '&' && *ctype != '^' )
	{
		bug("Make_color_sequence: command '%c' not '&' or '^'.", *ctype);
		ln = -1;
	}
	else if ( *col == *ctype )
	{
		buf[0] = *col;
		buf[1] = '\0';
		ln = 1;
	}
	else if ( !ansi )
		ln = 0;
	else
	{
		cl = d->prevcolor;
		switch(*ctype)
		{
			default:
				bug( "Make_color_sequence: bad command char '%c'.", *ctype );
				ln = -1;
				break;

			// &- --> ~
			case '&':
				if ( *col == '-' )
				{
					buf[0] = '~';
					buf[1] = '\0';
					ln = 1;
					break;
				}

                                // &_ --> reset colors
                                if ( *col == '_' )
                                {
                                    sprintf(buf, "\033[0m");
                                    ln = 4;
                                    break;
                                }
			case '^':
				{
					int newcol;

					if ( (newcol = getcolor(*col)) < 0 )
					{
						ln = 0;
						break;
					}
					else if ( *ctype == '&' )
						cl = (cl & 0xF0) | newcol;
					else
						cl = (cl & 0x0F) | (newcol << 4);
				}
				if ( cl == d->prevcolor )
				{
					ln = 0;
					break;
				}
				strcpy(buf, "\033[");
				if ( (cl & 0x88) != (d->prevcolor & 0x88) )
				{
					strcat(buf, "m\033[");
					if ( (cl & 0x08) )
						strcat(buf, "1;");
					if ( (cl & 0x80) )
						strcat(buf, "5;");
					d->prevcolor = 0x07 | (cl & 0x88);
					ln = strlen(buf);
				}
				else
					ln = 2;
				if ( (cl & 0x07) != (d->prevcolor & 0x07) )
				{
					sprintf(buf+ln, "3%d;", cl & 0x07);
					ln += 3;
				}
				if ( (cl & 0x70) != (d->prevcolor & 0x70) )
				{
					sprintf(buf+ln, "4%d;", (cl & 0x70) >> 4);
					ln += 3;
				}
				if ( buf[ln-1] == ';' )
					buf[ln-1] = 'm';
				else
				{
					buf[ln++] = 'm';
					buf[ln] = '\0';
				}
				d->prevcolor = cl;
		}
	}
	if ( ln <= 0 )
		*buf = '\0';
	return ln;
}

const char* random_password() {
    static char pword[9];
    FILE* fp;
    int i = 0;

    if ( !(fp = fopen("/dev/urandom", "rb")) ) {
        bug("random_password: can not open /dev/urandom: errno(%d)", errno);
        return "k3k43g45";
    }

    while ( i < 8 ) {
        char c = fgetc(fp);

        if ( isalnum(c) )
            pword[i++] = c;
    }

    pword[8] = '\0';

    return pword;
}

bool mail_account_password(PlayerConnection* d, const char* pword, const char* file)
{
    FILE* sendmail;
    FILE* fp;
    char  buf[MAX_INPUT_LENGTH];

    if ( !(sendmail = popen("/usr/lib/sendmail -t", "w")) ) {
        sprintf(buf, "\r\n"
                "Error sending the email to that address (%s).\r\n\r\n"
                "Please send an email to Ydnat (ydnat@darkstone.mudservices.com)\r\n"
                "With the following information:\r\n\r\n"
                "    Email address: %s\r\n"
                "    Errno:         %d\r\n"
                "    Action Code:   newaccountemail\r\n",
                d->Account->email, d->Account->email, errno);
        //write_to_buffer(d, buf, 0);
		d->SendText(buf);
		gConnectionManager->RemoveSocket(d, false);
        //close_socket(d, FALSE);
        return false;
    }

    if ( !(fp = fopen(file, "r")) ) {
        sprintf(buf, "\r\n"
            "Error sending the email to that address (%s).\r\n\r\n"
            "Please send an email to Ydnat (ydnat@darkstone.mudservices.com)\r\n"
            "With the following information:\r\n\r\n"
            "    Email address: %s\r\n"
            "    Errno:         %d\r\n"
            "    Action Code:   newaccountfile\r\n",
            d->Account->email, d->Account->email, errno);
        //write_to_buffer(d, buf, 0);
		d->SendText(buf);
        pclose(sendmail);
		gConnectionManager->RemoveSocket(d, false);
        //close_socket(d, FALSE);
        return FALSE;
    }

    fprintf(sendmail, "To: %s (New Darkstone Player)\r\n", d->Account->email);
    fprintf(sendmail, "From: darkstone-imms@mudservices.com (Legends of the Darkstone)\r\n");
    fprintf(sendmail, "Subject: Welcome to Legends of the Darkstone!\r\n");
    fprintf(sendmail, "\r\n");

    while ( fgets(buf, MAX_INPUT_LENGTH-2, fp) ) {
        fprintf(sendmail, "%s", buf);
    }

    if ( pword ) {
        fprintf(sendmail, "\r\nYour password is %s.\r\n", pword);
        fprintf(sendmail, "\r\nWe look forward to seeing you on Legends of the Darkstone!\r\n");
    }

    fprintf(sendmail, "\r\n\r\n-- \r\n");
    fprintf(sendmail, "Sender information:\r\n");
    fprintf(sendmail, "    Request time: %s", ctime(&secCurrentTime));
    fprintf(sendmail, "    Request IP:   %s\r\n", d->GetHost());

    fclose(fp);
    pclose(sendmail);

    return TRUE;
}


// Simple bridge to vnum_to_dotted
//  - Ksilyan
void do_vnum_to_dotted(CHAR_DATA * ch, const char* arg)
{
	// try converting arg to number
	int vnum = atoi(arg);

	if ( vnum <= 0 )
	{
		send_to_char("Enter a postive integer.\r\n", ch);
		return;
	}

	ch_printf(ch, "Vnum %d = %s.\r\n", vnum, vnum_to_dotted(vnum));
}

// Simple bridge to dotted_to_vnum
//  - Ksilyan
void do_dotted_to_vnum(CHAR_DATA * ch, const char* arg)
{
	int vnum = dotted_to_vnum(0, arg);

	if ( vnum == -1 )
		send_to_char("That is not a valid vnum.\r\n", ch);
	else
		ch_printf(ch, "Vnum %s = %d.\r\n", arg, vnum);
}

char* vnum_to_dotted(int vnum)
{
    int minor, major;
    static char buf[15]; /* A little extra 32768.32768 is max */

    major = vnum >> 15;
    minor = vnum % 32768;

    sprintf(buf, "%d.%d", major, minor);
    return buf;
}

int dotted_to_vnum(int bvnum, const char* str) {
    int  vnum, major, minor;
    int  bmajor = 0, bminor = 0;

    vnum = -1;

    if ( bvnum ) {
        bmajor = bvnum >> 15;
        bminor = bvnum % 32768;
    } else {
        bmajor = bminor = 0;
    }

    if ( !str || str[0] == '\0' ) {
        return -1; /* -1 is impossible vnum */
    }

    if ( str[0] == '+' ) {
        bvnum += atoi(str);
        return bvnum;
    } else if ( str[0] == '-' ) {
        bvnum += atoi(str); /* Yes, +=, 'cause + a - is the same as - */
        return bvnum;       /* but - a - is the same as + :P */
    } else if ( str[0] == '.' ) {
        vnum = atoi(++str);

        if ( vnum >= 32768 ) {
            return -1; /* 0 <= minor < 32768 */
        }

        vnum += bmajor << 15;

        return vnum;
    }
#if 0 /* Testaur: eliminate the XA... and XE... playername bug */
      else if ( str[0] == 'x' ) {
        sscanf(str, "x%x", &vnum);
        return vnum;
    }
#endif
       else if ( !strstr(str, ".") ) {
        if ( atoi(str) <= 0 )
            return -1;
       return atoi(str) + (atoi(str) <= 32768 ? bmajor << 15 : 0);
    } else if ( !str_suffix(".", str) ) {
        return atoi(str) << 15;
    } else {
		int result;

        result = sscanf(str, "%d.%d", &major, &minor);

		if ( result <= 0 )
		{
			// error, or none converted
			return -1; // impossible vnum
		}

        vnum  = major << 15;
        vnum += minor;

        return vnum;
    }
}

// Ksilyan:

// private function object to build a connections list
class comm_fBuildConnections
{
private:
	ostream & m_os;

public:

	// constructor - remember which stream to use
	comm_fBuildConnections (ostream & os) : m_os (os) {};

	void operator() (idSocket & id)
	{
		SocketGeneral * socket = SocketMap[id];

		if ( !socket )
		{
			m_os << "A socket was found that was no longer allocated!" << endl;
			return;
		}

		m_os << socket->codeGetFullInfo() << endl;
	};
};

void do_connections(Character * ch, const char* argument)
{
	// for copying results into
	ostringstream os;

	os << "-----------" << endl;
	os << "CONNECTIONS" << endl;
	os << "-----------" << endl;

	// print into memory
	for_each(
		gConnectionManager->GetSocketsBegin(),
		gConnectionManager->GetSocketsEnd(),
		comm_fBuildConnections(os)
		);

	os << endl;
	os << "--------" << endl;
	os << "ACCOUNTS" << endl;
	os << "--------" << endl;

	for (ACCOUNT_DATA * account = first_account; account; account = account->next)
	{
		os << "Account: " << account->email << " with " << account->iref << " reference(s)." << endl;
	}

	ch->sendText(os.str());
}


