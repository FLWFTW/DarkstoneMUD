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
 *			   Player movement module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fstream>

#include "mud.h"
#include "paths.const.h"
#include "object.h"
#include "World.h"

#include "commands.h"

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 5, 7, 4, 18
};

const char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest", "somewhere"
};

const char * const dir_name_ly [] =
{
	"in a northerly direction", "in an easterly direction",
	"in a westerly direction", "in a southerly direction",
	"upward", "downward", "in a northeasterly direction",
	"in a northwesterly direction", "in a southeasterly direction",
	"in a southwesterly direction",
	"in an unknown direction"
};

extern const char * const dir_name_ern [];


const char * const dir_name_ern [] =
{
	"northern", "eastern", "southern", "western", "upwards",
	"downwards", "northeastern", "northwestern", "southeastern",
	"southwestern", "unknown"
};

const	int	trap_door	[]		=
{
    TRAP_N, TRAP_E, TRAP_S, TRAP_W, TRAP_U, TRAP_D,
    TRAP_NE, TRAP_NW, TRAP_SE, TRAP_SW
};


const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};


ROOM_INDEX_DATA * vroom_hash [64];


/*
 * Local functions.
 */
bool	has_key		 ( CHAR_DATA *ch, int key ) ;


const char *	const		sect_names[SECT_MAX][2] =
{
    { "In a room","inside"	},	{ "In a city",	"cities"	},
    { "In a field","fields"	},	{ "In a forest","forests"	},
    { "hill",	"hills"		},	{ "On a mountain","mountains"	},
    { "In the water","waters"	},	{ "In rough water","waters"	},
    { "Underwater", "underwaters" },	{ "In the air",	"air"		},
    { "In a desert","deserts"	},	{ "Somewhere",	"unknown"	},
    { "ocean floor", "ocean floor" },	{ "underground", "underground"	},
    { "On a road", "roads" }
};


const	int		sent_total[SECT_MAX]		=
{
    3, 5, 4, 4, 1, 1, 1, 1, 1, 2, 2, 25, 1, 1
};

const char *	const		room_sents[SECT_MAX][25]	=
{
    {
	"rough hewn walls of granite with the occasional spider crawling around",
	"signs of a recent battle from the bloodstains on the floor",
	"a damp musty odour not unlike rotting vegetation"
    },

    {
	"the occasional stray digging through some garbage",
	"merchants trying to lure customers to their tents",
	"some street people putting on an interesting display of talent",
	"an argument between a customer and a merchant about the price of an item",
	"several shady figures talking down a dark alleyway"
    },

    {
	"sparce patches of brush and shrubs",
	"a small cluster of trees far off in the distance",
	"grassy fields as far as the eye can see",
	"a wide variety of weeds and wildflowers"
    },

    {
	"tall, dark evergreens prevent you from seeing very far",
	"many huge oak trees that look several hundred years old",
	"a solitary lonely weeping willow",
	"a patch of bright white birch trees slender and tall"
    },

    {
	"rolling hills lightly speckled with violet wildflowers"
    },

    {
	"the rocky mountain pass offers many hiding places"
    },

    {
	"the water is smooth as glass"
    },

    {
	"rough waves splash about angrily"
    },

    {
	"a small school of fish"
    },

    {
	"the land far below",
	"a misty haze of clouds"
    },

    {
	"sand as far as the eye can see",
	"an oasis far in the distance"
    },

    {
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",	"nothing unusual",	"nothing unusual",
	"nothing unusual",
    },

    {
        "rocks and coral which litter the ocean floor."
    },

    {
	"a lengthy tunnel of rock."
    }

};

char *grab_word( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first++ = *argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

char *wordwrap( char *txt, sh_int wrap )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufp;

    buf[0] = '\0';
    bufp = buf;
    if ( txt != NULL )
    {
        char line[MAX_STRING_LENGTH];
        char temp[MAX_STRING_LENGTH];
        char *ptr, *p;
        int ln, x;

	++bufp;
        line[0] = '\0';
        ptr = txt;
        while ( *ptr )
        {
	  ptr = grab_word( ptr, temp );
          ln = strlen( line );  x = strlen( temp );
          if ( (ln + x + 1) < wrap )
          {
	    if ( line[ln-1] == '.' )
              strcat( line, "  " );
	    else
              strcat( line, " " );
            strcat( line, temp );
            p = strchr( line, '\n' );
            if ( !p )
              p = strchr( line, '\r' );
            if ( p )
            {
                strcat( buf, line );
                line[0] = '\0';
            }
          }
          else
          {
            strcat( line, "\r\n" );
            strcat( buf, line );
            strcpy( line, temp );
          }
        }
        if ( line[0] != '\0' )
            strcat( buf, line );
    }
    return bufp;
}

void decorate_room( ROOM_INDEX_DATA *room )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int nRand;
	int iRand, len;
	int previous[8];
	int sector = room->sector_type;
	const char *pre, *post;

	/*
	   room->name	  = STRALLOC( "In a virtual room" );
	   room->description = STRALLOC( "You're on a pathway.\n\r" );
	 */

	room->name_ = sect_names[sector][0];
	buf[0] = '\0';
	nRand = number_range( 1, UMIN(8,sent_total[sector]) );

	for ( iRand = 0; iRand < nRand; iRand++ )
		previous[iRand] = -1;

	for ( iRand = 0; iRand < nRand; iRand++ )
	{
		while ( previous[iRand] == -1 )
		{
			int x, z;

			x = number_range( 0, sent_total[sector]-1 );

			for ( z = 0; z < iRand; z++ )
				if ( previous[z] == x )
					break;

			if ( z < iRand )
				continue;

			previous[iRand] = x;

			len = strlen(buf);
			if ( len == 0 )
			{
				switch( number_range(1, 2 * (iRand == nRand -1) ? 1 : 2) )
				{
					case 1:	pre = "You notice ";	post = ".";	 break;
					case 2: pre = "You see ";	post = ".";	 break;
					case 3: pre = "You see ";	post = ", and "; break;
					case 4: pre = "You notice ";	post = ", and "; break;
				}
				sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
			}
			else
				if ( iRand != nRand -1 )
				{
					if ( buf[len-1] == '.' )
						switch( number_range(0, 3) )
						{
							case 0:	pre = "you notice ";	post = ".";	 break;
							case 1: pre = "you see ";	post = ", and "; break;
							case 2: pre = "you see ";	post = ".";	 break;
							case 3: pre = "over yonder ";	post = ", and "; break;
						}
					else
						switch( number_range(0, 3) )
						{
							case 0:	pre = ""; post = ".";			break;
							case 1:	pre = ""; post = " not too far away.";	break;
							case 2: pre = ""; post = ", and ";		break;
							case 3: pre = ""; post = " nearby.";		break;
						}
					sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
				}
				else
					sprintf( buf2, "%s.", room_sents[sector][x] );
			if ( len > 5 && buf[len-1] == '.' )
			{
				strcat( buf, "  " );
				buf2[0] = UPPER(buf2[0] );
			}
			else
				if ( len == 0 )
					buf2[0] = UPPER(buf2[0] );
			strcat( buf, buf2 );
		}
	}
	sprintf( buf2, "%s\n\r", wordwrap(buf, 78) );
	room->description_ = buf;
}

/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms( )
{
    int hash;
    ROOM_INDEX_DATA *room, *room_next, *prev;

    for ( hash = 0; hash < 64; hash++ )
    {
	while ( vroom_hash[hash]
	&&     !vroom_hash[hash]->first_person
	&&     !vroom_hash[hash]->first_content )
	{
	    room = vroom_hash[hash];
	    vroom_hash[hash] = room->next;
	    clean_room( room );
	    delete room;
	    --top_vroom;
	}
	prev = NULL;
	for ( room = vroom_hash[hash]; room; room = room_next )
	{
	    room_next = room->next;
	    if ( !room->first_person && !room->first_content )
	    {
		if ( prev )
		  prev->next = room_next;
		clean_room( room );
		delete room;
		--top_vroom;
	    }
	    if ( room )
	      prev = room;
	}
    }
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
ExitData *get_exit( ROOM_INDEX_DATA *room, sh_int dir )
{
	ExitData *xit;

	if ( !room )
	{
		bug( "Get_exit: NULL room", 0 );
		return NULL;
	}

	for (xit = room->first_exit; xit; xit = xit->next )
		if ( xit->vdir == dir )
			return xit;
	return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
ExitData *get_exit_to( ROOM_INDEX_DATA *room, sh_int dir, int vnum )
{
    ExitData *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir && xit->vnum == vnum )
         return xit;
    return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
ExitData *get_exit_num( ROOM_INDEX_DATA *room, sh_int count )
{
    ExitData *xit;
    int cnt;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (cnt = 0, xit = room->first_exit; xit; xit = xit->next )
       if ( ++cnt == count )
         return xit;
    return NULL;
}

void mob_open( CHAR_DATA *ch, ExitData *pexit )
{
	int door;

	/* 'open door' */
	ExitData *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
		send_to_char( "You can't do that.\n\r", 	 ch );
		return;
	}
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	{
		send_to_char( "It's already open.\n\r", 	 ch );
		return;
	}
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	{
		if ( (!has_key( ch, pexit->key)) || (!IS_SET(pexit->exit_info, EX_SECRET)))
		{
			send_to_char( "It's locked.\n\r",			 ch );
			return;
		}

		send_to_char( "*Click*\n\r", ch );
		act( AT_ACTION, "$n unlocks the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
		remove_bexit_flag( pexit, EX_LOCKED );
	}

	if ( !IS_SET(pexit->exit_info, EX_SECRET))
	{
		act( AT_ACTION, "$n opens the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
		act( AT_ACTION, "You open the $d.", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );

		if ( (pexit_rev = pexit->rexit) != NULL &&	 pexit_rev->to_room == ch->GetInRoom() )
		{
			CHAR_DATA *rch;

			for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
				act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword_.c_str(), TO_CHAR );
		}

		remove_bexit_flag( pexit, EX_CLOSED );

		if (((door=pexit->vdir) >= 0 )&& (door < 10) )
			check_room_for_traps( ch, trap_door[door]);

		return;
	}

	return;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance( CHAR_DATA *ch, sh_int move )
{
    int cur, max;

	max = can_carry_w(ch);
	cur = ch->carry_weight;
	if ( cur >= max )
		return (short) move * 4;
	else if ( cur >= max * 0.95 )
		return (short) (move * 3.5);
	else if ( cur >= max * 0.90 )
		return (short) move * 3;
	else if ( cur >= max * 0.85 )
		return (short) (move * 2.5);
	else if ( cur >= max * 0.80 )
		return (short) move * 2;
	else if ( cur >= max * 0.75 )
		return (short) (move * 1.5);
	else
		return (short) move;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall( CHAR_DATA *ch, int fall )
{
	if ( !ch->GetInRoom())
	{
		bug( "ch %s has no room! (will_fall)", ch->getName().c_str() );
		return FALSE;
	}
	if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_NOFLOOR )
		&&	 CAN_GO(ch, DIR_DOWN)
		&& (!IS_AFFECTED( ch, AFF_FLYING )
		|| ( ch->GetMount() && !IS_AFFECTED( ch->GetMount(), AFF_FLYING ) ) ) )
	{
		if ( fall > 80 )
		{
			bug( "Falling (in a loop?) more than 80 rooms: room vnum %s", vnum_to_dotted(ch->GetInRoom()->vnum) );
			char_from_room( ch );
			char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
			fall = 0;
			return TRUE;
		}
		set_char_color( AT_FALLING, ch );
		send_to_char( "You're falling down...\n\r", ch );
		move_char( ch, get_exit(ch->GetInRoom(), DIR_DOWN), ++fall );
		return TRUE;
	}
	return FALSE;
}


/*
 * create a 'virtual' room					-Thoric
 */
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, ExitData **pexit )
{
	ExitData *xit, *bxit;
	ExitData *orig_exit = *pexit;
	ROOM_INDEX_DATA *room, *backroom;
	int brvnum;
	int serial;
	int roomnum;
	int distance = -1;
	int vdir = orig_exit->vdir;
	sh_int hash;
	bool found = FALSE;

	if ( in_room->vnum > 1048576000 )	/* room is virtual */
	{
		serial = in_room->vnum;
		roomnum = in_room->tele_vnum;
		if ( (serial & 2097152000) == orig_exit->vnum )
		{
			brvnum = serial >> 16;
			--roomnum;
			distance = roomnum;
		}
		else
		{
			brvnum = serial & 2097152000;
			++roomnum;
			distance = orig_exit->distance - 1;
		}
		backroom = get_room_index( brvnum );
	}
	else
	{
		int r1 = in_room->vnum;
		int r2 = orig_exit->vnum;

		brvnum = r1;
		backroom = in_room;
		serial = (UMAX( r1, r2 ) << 16) | UMIN( r1, r2 );
		distance = orig_exit->distance - 1;
		roomnum = r1 < r2 ? 1 : distance;
	}
	hash = serial % 64;

	for ( room = vroom_hash[hash]; room; room = room->next )
		if ( room->vnum == serial && room->tele_vnum == roomnum )
		{
			found = TRUE;
			break;
		}
	if ( !found )
	{
		room = new Room();
		room->area	  = in_room->area;
		room->vnum	  = serial;
		room->tele_vnum	  = roomnum;
		room->sector_type = in_room->sector_type;
		room->room_flags  = in_room->room_flags;
		decorate_room( room );
		room->next	  = vroom_hash[hash];
		vroom_hash[hash]  = room;
		++top_vroom;
	}
	if ( !found || (xit=get_exit(room, vdir))==NULL )
	{
		xit = make_exit(room, orig_exit->to_room, vdir);
		xit->keyword_       = "";
		xit->description_   = "";
		xit->key		= -1;
		xit->distance = distance;
	}
	if ( !found )
	{
		bxit = make_exit(room, backroom, rev_dir[vdir]);
		bxit->keyword_      = "";
		bxit->description_  =  "";
		bxit->key		= -1;
		if ( (serial & 2097152000) != orig_exit->vnum )
			bxit->distance = roomnum;
		else
		{
			ExitData *tmp = get_exit( backroom, vdir );
			int fulldist = tmp->distance;

			bxit->distance = fulldist - distance;
		}
	}

	//(ExitData *)
	*pexit = xit;
	return room;
}



// <--
// Movement Types Snippet
// Ksilyan 2005-Aug-2
// Thanks to Pracllik for the original snippet
// Snippet modified by Ksilyan


// Global table for mapping movement message name to (enter, exit) message pair
typedef map<string, pair<string, string> > MovementMessageMap;
MovementMessageMap MovementMessages;


/** \brief Get a movement entry/exit message pair.
 *
 * \param message The movement message to look up.
 * \return A std::pair whose first element is the entry message and whose
 *         second element is the exit message.
 */
const pair<string, string> & GetMovementMessage(const std::string & message)
{
	MovementMessageMap::iterator it = MovementMessages.find(message);

	if ( it == MovementMessages.end() )
	{
		gTheWorld->LogBugString("Unknown movement message: " + message);

		if ( message == "default" )
		{
			gTheWorld->LogBugString("No default movement message?!?");
			return MovementMessages["????"];
		}

		return GetMovementMessage("default");
	}

	return it->second;
}

/** \brief Convenience wrapper around GetMovementMessage.
 *
 * Directly accesses the entry message.
 *
 * \param movementMessage The movement message to look up.
 */
const string & GetEntryMessage(const string & movementMessage)
{
	return GetMovementMessage(movementMessage).first;
}

/** \brief Convenience wrapper around GetMovementMessage.
 *
 * Directly accesses the exit message.
 *
 * \param movementMessage The movement message to look up.
 */
const string & GetExitMessage(const string & movementMessage)
{
	return GetMovementMessage(movementMessage).second;
}

/**
 * Load the movement messages from a file. The file is formatted
 * with one entry/exit pair per line, like so:
 * \code
 * name~entry message~exit message
 * name2~entry message2~exit message2
 * etc.
 * \endcode
 *
 * \param filename The file to load the messages from.
 */
void LoadMovementMessages(const string & filename)
{
	ifstream in( filename.c_str() );

	if ( in.fail() )
	{
		gTheWorld->LogBugString("Couldn't open movement messages file!");
		return;
	}

	while ( true )
	{
		string line;
		getline(in, line);

		if ( in.fail() == true )
			break;

		// Everything until the first twiddle is the movement message name;
		// everything afterwards is the movement message
		// The movement message has another twiddle, which separates entry/exit messages

		string::size_type pos = line.find("~");
		string::size_type pos2 = line.find("~", pos+1);

		if ( pos == string::npos || pos2 == string::npos )
		{
			gTheWorld->LogBugString("Error parsing movement message line: " + line);
			continue;
		}

		string msgName = line.substr(0, pos); // everything up until twiddle

		string enter = line.substr(pos+1, pos2-(pos+1) );
		string exit = line.substr(pos2+1);

		MovementMessages.insert( make_pair(msgName, make_pair(enter, exit) ) );
	}

	MovementMessageMap::iterator it;

	string result;

	for ( it = MovementMessages.begin(); it != MovementMessages.end(); it++ )
	{
		result += it->first + " ";
	}

	gTheWorld->LogString("Loaded character movement types: " + result);
}

/** \brief Save the movement messages to file.
 *
 * \param filename The file to save to.
 */
void SaveMovementMessages(const string & filename)
{
	ofstream out( filename.c_str() );

	MovementMessageMap::iterator it;

	for ( it = MovementMessages.begin(); it != MovementMessages.end(); it++ )
		out << it->first << "~" << it->second.first << "~" << it->second.second << endl;

	gTheWorld->LogString("Movement messages saved to " + filename);
}

void do_editmovementmessage(Character * ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	string cmd = strlower(arg1);
	string msgName = strlower(arg2);

	// Make sure basic syntax is correct, or print usage message.

	if ( cmd == "" || (cmd != "list" && msgName == "") )
	{
		ch->sendText("EditMovementMessage syntax:\n\r");
		ch->sendText("\teditmovementmessage command movement-message-name\n\r" );
		ch->sendText("\t'command' is one of: set, delete, list");

		return;
	}

	if ( cmd == "set" )
	{
		string msg = argument;

		if ( msg == "" )
		{
			ch->sendText("Can't set a message to nothing; use delete instead.\n\r");
			return;
		}

		string::size_type pos = msg.find("~");

		if ( pos == string::npos )
		{
			ch->sendText("Invalid message: must be of form: enter message~exit message");
			return;
		}

		string enterMsg = msg.substr(0, pos);
		string exitMsg = msg.substr(pos+1);

		MovementMessages[msgName] = make_pair(enterMsg, exitMsg);
		ch->sendText("Set movement message \"" + msgName + "\" to \"" + msg + "\"\n\r");
	}
	else if ( cmd == "delete" )
	{
		if ( msgName == "default" )
		{
			ch->sendText("Can't delete default movement message!\n\r");
		}

		MovementMessages.erase(msgName);
		ch->sendText("Deleted movement message \"" + msgName + "\"\n\r" );
	}
	else if ( cmd == "list" )
	{
		MovementMessageMap::iterator it;

		ch->sendText("Movement messages: \n\r");

		for ( it = MovementMessages.begin(); it != MovementMessages.end(); it++ )
			ch->sendText(it->first + ": " + it->second.first + "; " + it->second.second + "\n\r");

		return;
	}
	else
	{
		ch->sendText("Unrecognized edit-movement-message command: '" + cmd + "'.\n\r");

		// display usage message
		do_editmovementmessage(ch, "");
		return;
	}

	SaveMovementMessages(MOVEMENT_MESSAGE_FILE);
}

void do_movementmessage(Character * ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	one_argument( argument, arg1 );

	if( arg1[0] == '\0' )
	{
		send_to_char( "MovementMessage Command\n\r", ch );
		send_to_char( "Syntax: movementmessage <movement message type>\n\r", ch );
		send_to_char( "Where type is one of the following:\n\r", ch );

		MovementMessageMap::iterator it;

		for ( it = MovementMessages.begin(); it != MovementMessages.end(); it++ )
			ch->sendText( "\t" + it->first + " - " + it->second.first + "; " + it->second.second + "\n\r" );

		return;
	}

	MovementMessageMap::iterator msgIt = MovementMessages.find(arg1);

	if ( msgIt == MovementMessages.end() )
	{
		ch->sendText( "That is not a valid movement message.\n\r" );
		ch->sendText( "Valid messages:\n\r" );

		MovementMessageMap::iterator it;

		for ( it = MovementMessages.begin(); it != MovementMessages.end(); it++ )
			ch->sendText( "\t" + it->first + " - " + it->second.first + "; " + it->second.second + "\n\r" );

		return;
	}

	ch->setMovementMessage(msgIt->first);

	ch->sendText("Ok.\n\r");

}

// end movement message additions
// -->

ch_ret move_char( CHAR_DATA *ch, ExitData *pexit, int fall )
{
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	ROOM_INDEX_DATA *from_room;
	char buf[MAX_STRING_LENGTH];
	const char *txt;
	const char *dtxt;
	ch_ret retcode;
	sh_int door, distance;
	bool drunk = FALSE;
	bool brief = FALSE;

	if ( !IS_NPC( ch ) )
	{
		if ( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE )
			&& ( ch->position != POS_DRAG ) )
			drunk = TRUE;
	}

	if ( drunk && !fall )
	{
		door = number_door();
		pexit = get_exit( ch->GetInRoom(), door );
	}

#ifdef DEBUG
	if ( pexit )
	{
		sprintf( buf, "move_char: %s to door %d", ch->name, pexit->vdir );
		log_string( buf );
	}
#endif

	retcode = rNONE;
	txt = NULL;

	if ( IS_NPC(ch) && IS_SET( ch->act, ACT_MOUNTED ) )
		return retcode;

	in_room = ch->GetInRoom();
	from_room = in_room;
	if ( !pexit || (to_room = pexit->to_room) == NULL )
	{
		if ( drunk )
			send_to_char( "You hit a wall in your drunken state.\n\r", ch );
		else
			send_to_char( "Alas, you cannot go that way.\n\r", ch );
		return rNONE;
	}

	if ( current_arena.pArea ) {
		bool is_arena = !(str_cmp(pexit->to_room->area->filename, current_arena.pArea->filename));

		if ( !is_arena && in_arena(ch) ) {
			send_to_char("You can't leave the arena -- except by death!\n", ch);
			return rNONE;
		}

		if ( is_arena && !in_arena(ch) ) {
			char buf[MAX_STRING_LENGTH];

			arena_clear_char(ch);

			sprintf(buf, "%s decides to show %s true worth by entering the arena!", NAME(ch), SEX_HIS(ch) );
			talk_channel(ch, buf, CHANNEL_ARENA, "");
		}
	}

	door = pexit->vdir;
	distance = pexit->distance;

	/*
	* Exit is only a "window", there is no way to travel in that direction
	* unless it's a door with a window in it		-Thoric
	*/
	if ( IS_SET( pexit->exit_info, EX_WINDOW )
		&&	!IS_SET( pexit->exit_info, EX_ISDOOR ) )
	{
		send_to_char( "Alas, you cannot go that way.\n\r", ch );
		return rNONE;
	}

	if ( IS_SET(pexit->exit_info, EX_NOMOB)
		&& (IS_NPC(ch) && !IS_SET(ch->act, ACT_SUV)))
	{
		act( AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR );
		return rNONE;
	}


	if( IS_SET( pexit->exit_info, EX_CLOSED ) && IS_IMMORTAL( ch ) )
	{
		act( AT_PLAIN, "You pass through the $d.", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );
		if( !IS_SET( ch->act, PLR_WIZINVIS ) )	
		{
			act( AT_PLAIN, "$n passes through the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
		}
	}
	else if ( IS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
	{
		if ( !IS_SET( pexit->exit_info, EX_SECRET )
			&&	 !IS_SET( pexit->exit_info, EX_DIG ) )
		{
			if ( drunk )
			{
				act( AT_PLAIN, "$n runs into the $d in $s drunken state.", ch,
					NULL, pexit->keyword_.c_str(), TO_ROOM );
				act( AT_PLAIN, "You run into the $d in your drunken state.", ch,
					NULL, pexit->keyword_.c_str(), TO_CHAR );
			}
			else
				act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );
		}
		else
		{
			if ( drunk )
				send_to_char( "You hit a wall in your drunken state.\n\r", ch );
			else
				send_to_char( "Alas, you cannot go that way.\n\r", ch );
		}

		return rNONE;
	}

	/*
	* Crazy virtual room idea, created upon demand. 	-Thoric
	*/
	if ( distance > 1 )
	{
		if ( (to_room=generate_exit(in_room, &pexit)) == NULL )
			send_to_char( "Alas, you cannot go that way.\n\r", ch );
	}

	if ( !fall
		&&	 IS_AFFECTED(ch, AFF_CHARM)
		&&	 ch->MasterId != 0
		&&	 in_room == CharacterMap[ch->MasterId]->GetInRoom()
		&&	 !IS_SET(ch->act, ACT_SUV))
	{
		send_to_char( "What?  And leave your beloved master?\n\r", ch );
		return rNONE;
	}

	if ( room_is_private( to_room ) && !(IS_NPC(ch) && IS_SET(ch->act, ACT_SUV)))
	{
		send_to_char( "That room is private right now.\n\r", ch );
		return rNONE;
	}

	if ( !IS_IMMORTAL(ch)
		&&	!IS_NPC(ch)
		&&	ch->GetInRoom()->area != to_room->area )
	{
		if ( ch->level < to_room->area->low_hard_range )
		{
			set_char_color( AT_TELL, ch );
			switch( to_room->area->low_hard_range - ch->level )
			{
			case 1:
				send_to_char( "A voice in your mind says, 'You are almost ready to go that way...'", ch );
				break;
			case 2:
				send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'", ch );
				break;
			case 3:
				send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch);
				break;
			default:
				send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch);
			}
			return rNONE;
		}
		else if ( ch->level > to_room->area->hi_hard_range )
		{
			set_char_color( AT_TELL, ch );
			send_to_char( "A voice in your mind says, 'There is nothing more for you down that path.'", ch );
			return rNONE;
		}
	}

	if ( !fall && !IS_NPC(ch) )
	{
		int move;

		if ( in_room->sector_type == SECT_AIR
			||	 to_room->sector_type == SECT_AIR
			||	 IS_SET( pexit->exit_info, EX_FLY ) )
		{
			if ( ch->GetMount() && !IS_AFFECTED( ch->GetMount(), AFF_FLYING ) )
			{
				send_to_char( "Your mount can't fly.\n\r", ch );
				return rNONE;
			}
			if ( !ch->GetMount() && !IS_AFFECTED(ch, AFF_FLYING) )
			{
				send_to_char( "You'd need to fly to go there.\n\r", ch );
				return rNONE;
			}
		}

		if ( in_room->sector_type == SECT_WATER_NOSWIM
			||	 to_room->sector_type == SECT_WATER_NOSWIM )
		{
			OBJ_DATA *obj;
			bool found;

			found = FALSE;
			if ( ch->GetMount() )
			{
				if ( IS_AFFECTED( ch->GetMount(), AFF_FLYING )
					||	 IS_AFFECTED( ch->GetMount(), AFF_FLOATING ) )
					found = TRUE;
			}
			else if ( IS_AFFECTED(ch, AFF_FLYING)
				||	 IS_AFFECTED(ch, AFF_FLOATING) )
				found = TRUE;

				/*
				* Look for a boat.
			*/
			if ( !found )
			{
				for ( obj = ch->first_carrying; obj; obj = obj->next_content )
				{
					if ( obj->item_type == ITEM_BOAT )
					{
						found = TRUE;
						if ( drunk )
							txt = "paddles unevenly";
						else
							txt = "paddles";
						break;
					}
				}
			}

			if ( !found )
			{
				send_to_char( "You'd need a boat to go there.\n\r", ch );
				return rNONE;
			}
		}

		if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
		{
			bool found;

			found = FALSE;
			if ( ch->GetMount() && IS_AFFECTED( ch->GetMount(), AFF_FLYING ) )
				found = TRUE;
			else if ( IS_AFFECTED(ch, AFF_FLYING) )
				found = TRUE;

			if ( !found && !ch->GetMount() )
			{
				if ( ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_climb] )
					||		drunk || ch->mental_state < -90 )
				{
					send_to_char( "You start to climb... but lose your grip and fall!\n\r", ch);
					learn_from_failure( ch, gsn_climb );
					if ( pexit->vdir == DIR_DOWN )
					{
						retcode = move_char( ch, pexit, 1 );
						return retcode;
					}
					set_char_color( AT_HURT, ch );
					send_to_char( "OUCH! You hit the ground!\n\r", ch );
					ch->AddWait(20);
					retcode = damage( ch, ch, (pexit->vdir == DIR_UP ? 10 : 5),
						TYPE_UNDEFINED , 0);
					return retcode;
				}
				found = TRUE;
				learn_from_success( ch, gsn_climb );
				ch->AddWait( skill_table[gsn_climb]->beats );
				txt = "climbs";
			}

			if ( !found )
			{
				send_to_char( "You can't climb.\n\r", ch );
				return rNONE;
			}
		}

		if ( ch->GetMount() )
		{
			switch (ch->GetMount()->position)
			{
			case POS_DEAD:
				send_to_char( "Your mount is dead!\n\r", ch );
				return rNONE;
				break;

			case POS_MORTAL:
			case POS_INCAP:
				send_to_char( "Your mount is hurt far too badly to move.\n\r", ch );
				return rNONE;
				break;

			case POS_STUNNED:
				send_to_char( "Your mount is too stunned to do that.\n\r", ch );
				return rNONE;
				break;

			case POS_SLEEPING:
				send_to_char( "Your mount is sleeping.\n\r", ch );
				return rNONE;
				break;

			case POS_RESTING:
				send_to_char( "Your mount is resting.\n\r", ch);
				return rNONE;
				break;

			case POS_SITTING:
				send_to_char( "Your mount is sitting down.\n\r", ch);
				return rNONE;
				break;

			default:
				break;
			}

			move = 1;

			if ( ch->GetMount()->move < move )
			{
				send_to_char( "Your mount is too exhausted.\n\r", ch );
				return rNONE;
			}
		}
		else
		{
			if ( !IS_AFFECTED(ch, AFF_FLYING)
				&&	 !IS_AFFECTED(ch, AFF_FLOATING) )
				move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)] );
			else
				move = 1;
			if ( ch->move < move )
			{
				send_to_char( "You are too exhausted.\n\r", ch );
				return rNONE;
			}
		}

		if(!IS_IMMORTAL(ch))
			ch->AddWait( move );
		if ( ch->GetMount() )
			ch->GetMount()->move -= move;
		else
			ch->move -= move;
	}

	/*
	* Check if player can fit in the room
	*/
	if ( to_room->tunnel > 0 && !(IS_NPC(ch) && IS_SET(ch->act, ACT_SUV)))
	{
		CHAR_DATA *ctmp;
		int count = ch->GetMount() ? 1 : 0;

		for ( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
		{
			if ( ++count >= to_room->tunnel )
			{
				if ( ch->GetMount() && count == to_room->tunnel )
					send_to_char( "There is no room for both you and your mount in there.\n\r", ch );
				else
					send_to_char( "There is no room for you in there.\n\r", ch );
				return rNONE;
			}
		}
	}

	// Check for a room barricade
	Character * inRoomCh;
	short barricaderCount = 0;
	short totalDex = 0; // total dex of barricading character(s)

	// Count the barricaders
	for ( inRoomCh = in_room->first_person; inRoomCh; inRoomCh = inRoomCh->next_in_room )
	{
		if ( inRoomCh != ch && inRoomCh->BarricadeDir == pexit->vdir )
		{
			// the character is barricading the exit.
			barricaderCount++;
			totalDex += inRoomCh->getDex();
		}
	}

	if ( barricaderCount > 0 )
	{
		if ( IS_SET(pexit->exit_info, EX_BARRICADE) )
		{
			// the exit is "barricadable", which means that
			// to get through it, the barricaders have to be
			// dead

			// get a barricader
			Character * barricader;
			barricader = in_room->first_person;
			while ( barricader && barricader->BarricadeDir != pexit->vdir )
				barricader = barricader->next_in_room;

			act( AT_SKILL, "You prevent $N from going $t.", barricader, dir_name[pexit->vdir], ch, TO_CHAR );
			act( AT_SKILL, "$n prevents you from going $t!", barricader, dir_name[pexit->vdir], ch, TO_VICT );
			act( AT_SKILL, "$n prevents $N from going $t.", barricader, dir_name[pexit->vdir], ch, TO_NOTVICT );

			return rNONE;
		}
		else
		{
			// else means that it's a normal room, so we need to run
			// speed checks

			short chance = UMIN(90, 100 * ch->getDex() / totalDex);

			// Roll to see if barricade works

			if ( ch->ChanceRoll(chance) )
			{
				// ch makes it through!
				act( AT_SKILL, "You slip through the barricade!", ch, NULL, NULL, TO_CHAR);
				act( AT_SKILL, "$n slips through the barricade!", ch, NULL, NULL, TO_ROOM);
			}
			else
			{
				// ch is stopped

				// get a barricader
				Character * barricader;
				barricader = in_room->first_person;
				while ( barricader && barricader->BarricadeDir != pexit->vdir )
					barricader = barricader->next_in_room;

				act( AT_SKILL, "You prevent $N from going $t.", barricader, dir_name[pexit->vdir], ch, TO_CHAR );
				act( AT_SKILL, "You are no longer barricading the $t exit!", barricader, dir_name_ern[pexit->vdir], ch, TO_CHAR );

				act( AT_SKILL, "$n prevents you from going $t!", barricader, dir_name[pexit->vdir], ch, TO_VICT );
				act( AT_SKILL, "$n prevents $N from going $t.", barricader, dir_name[pexit->vdir], ch, TO_NOTVICT );

				// Remove the barricade
				barricader->BarricadeDir = -1;
				// Add waits
				barricader->AddWaitSeconds(1);
				ch->AddWaitSeconds(3);
				return rNONE;
			}
		}
	}

	/* check for traps on exit - later */

	if ( ch->exitDesc_.length() == 0
		/*	  && !IS_AFFECTED(ch, AFF_SNEAK)   this check moved inside act */
		&& ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
	{
		if ( fall )
			txt = "falls";
		else if ( !txt )
		{
			if ( ch->GetMount() )
			{
				if ( IS_AFFECTED( ch->GetMount(), AFF_FLOATING ) )
					txt = "floats";
				else if ( IS_AFFECTED( ch->GetMount(), AFF_FLYING ) )
					txt = "flys";
				else
					txt = "rides";
			}
			else
			{
				if ( IS_AFFECTED( ch, AFF_FLOATING ) )
				{
					if ( drunk )
						txt = "floats unsteadily";
					else
						txt = "floats";
				}
				else if ( IS_AFFECTED( ch, AFF_FLYING ) )
				{
					if ( drunk )
						txt = "flys shakily";
					else
						txt = "flys";
				}
				else if ( ch->position == POS_SHOVE )
					txt = "is shoved";
				else if ( ch->position == POS_DRAG )
					txt = "is dragged";
				else
				{
					if ( drunk )
						txt = "stumbles drunkenly";
					else
						txt = GetExitMessage( ch->getMovementMessage() ).c_str(); /* "leaves"; */
				}
			}
		}
		act_check_sneak =1;
		if ( ch->GetMount() )
		{
			sprintf( buf, "$n %s %s upon $N.", txt, dir_name[door] );
			act( AT_ACTION, buf, ch, NULL, ch->GetMount(), TO_NOTVICT );
		}
		else
		{
			sprintf( buf, "$n %s $T.", txt );
			act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM );
		}
		act_check_sneak=0;
	}
	else if ( ch->exitDesc_.length() > 0
		/* && !IS_AFFECTED(ch, AFF_SNEAK) */
		&& (IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS)))
		/* If the mob has an EXIT_DESCR, use it! */
	{
		exit_act(AT_ACTION, ch->exitDesc_.c_str(), ch, door, FALSE, TO_ROOM);
	}

	rprog_leave_trigger( ch );
	if( char_died(ch) )
		return global_retcode;

	// Ksilyan: so that if wasinroom will work.
	ch->WasInRoomId = ch->InRoomId;

	char_from_room( ch );
	if ( ch->GetMount() )
	{
		rprog_leave_trigger( ch->GetMount() );
		if( char_died(ch) )
			return global_retcode;
		if( ch->GetMount() )
		{
			char_from_room( ch->GetMount() );
			char_to_room( ch->GetMount(), to_room );
		}
	}


	char_to_room( ch, to_room );
	if ( ch->enterDesc_.length() == 0
		/* && !IS_AFFECTED(ch, AFF_SNEAK) */
		&& ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
	{
		if ( fall )
			txt = "falls";
		else if ( ch->GetMount() )
		{
			if ( IS_AFFECTED( ch->GetMount(), AFF_FLOATING ) )
				txt = "floats in";
			else if ( IS_AFFECTED( ch->GetMount(), AFF_FLYING ) )
				txt = "flys in";
			else
				txt = "rides in";
		}
		else
		{
			if ( IS_AFFECTED( ch, AFF_FLOATING ) )
			{
				if ( drunk )
					txt = "floats in unsteadily";
				else
					txt = "floats in";
			}
			else if ( IS_AFFECTED( ch, AFF_FLYING ) )
			{
				if ( drunk )
					txt = "flys in shakily";
				else
					txt = "flys in";
			}
			else if ( ch->position == POS_SHOVE )
				txt = "is shoved in";
			else if ( ch->position == POS_DRAG )
				txt = "is dragged in";
			else
			{
				if ( drunk )
					txt = "stumbles drunkenly in";
				else
					txt = GetEntryMessage( ch->getMovementMessage() ).c_str(); /*"arrives";*/
			}
		}
		switch( door )
		{
		default: dtxt = "somewhere";	break;
		case 0:  dtxt = "the south";	break;
		case 1:  dtxt = "the west"; break;
		case 2:  dtxt = "the north";	break;
		case 3:  dtxt = "the east"; break;
		case 4:  dtxt = "below";		break;
		case 5:  dtxt = "above";		break;
		case 6:  dtxt = "the south-west";	break;
		case 7:  dtxt = "the south-east";	break;
		case 8:  dtxt = "the north-west";	break;
		case 9:  dtxt = "the north-east";	break;
		}
		act_check_sneak=1;
		if ( ch->GetMount() )
		{
			sprintf( buf, "$n %s from %s upon $N.", txt, dtxt );
			act( AT_ACTION, buf, ch, NULL, ch->GetMount(), TO_ROOM );
		}
		else
		{
			sprintf( buf, "$n %s from %s.", txt, dtxt );
			act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
		}
		act_check_sneak=0;
	}
	else	 if ( ch->enterDesc_.length() > 0
		/*	&& !IS_AFFECTED(ch, AFF_SNEAK) */
		&& ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) ) )
	{
		exit_act(AT_ACTION, ch->enterDesc_.c_str(), ch, door, TRUE, TO_ROOM);
	}


	if ( !IS_IMMORTAL(ch)
		&&	!IS_NPC(ch)
		&&	ch->GetInRoom()->area != to_room->area )
	{
		if ( ch->level < to_room->area->low_soft_range )
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char("You feel uncomfortable being in this strange land...\n\r", ch);
		}
		else
			if ( ch->level > to_room->area->hi_soft_range )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char("You feel there is not much to gain visiting this place...\n\r", ch);
			}
	}

	/* Make sure everyone sees the room description of death traps. */
	if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
	{
		if( IS_SET( ch->act, PLR_BRIEF ) )
			brief = TRUE;
		REMOVE_BIT( ch->act, PLR_BRIEF );
	}

	look_from_dir( ch, door );
	if ( brief )
		SET_BIT( ch->act, PLR_BRIEF );

		/*
		* Put good-old EQ-munching death traps back in! 	-Thoric
	*/
	if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch )
		&& !(IS_NPC(ch) && IS_SET(ch->act, ACT_SUV)))
	{
		act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
		set_char_color( AT_DEAD, ch );
		send_to_char( "Oopsie... you're dead!\n\r", ch );
		sprintf(buf, "%s hit a DEATH TRAP in room %s!",
			ch->getName().c_str(), vnum_to_dotted(ch->GetInRoom()->vnum) );
		log_string( buf );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", sysdata.level_monitor );
		set_cur_char(supermob);
		raw_kill(supermob, ch);
		return rCHAR_DIED;
	}

	/* BIG ugly looping problem here when the character is mptransed back
	to the starting room.  To avoid this, check how many chars are in
	the room at the start and stop processing followers after doing
	the right number of them.  -- Narn
	*/
	if ( !fall )
	{
		CHAR_DATA *fch;
		CHAR_DATA *nextinroom;
		int chars = 0, count = 0;

		for ( fch = from_room->first_person; fch; fch = fch->next_in_room )
			chars++;

		for ( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
		{
			nextinroom = fch->next_in_room;
			count++;
			if ( fch != ch		/* loop room bug fix here by Thoric */
				&& fch->MasterId == ch->GetId()
				&& (fch->position == POS_STANDING || fch->position == POS_MOUNTED))
			{
				act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
				move_char( fch, pexit, 0 );
			}
		}
	}

	if ( ch->GetInRoom()->first_content )
		retcode = check_room_for_traps( ch, TRAP_ENTER_ROOM );
	if ( retcode != rNONE )
		return retcode;

	if ( char_died(ch) )
		return retcode;

	mprog_entry_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	mprog_greet_dir_trigger(ch, rev_dir[pexit->vdir]);
	if ( char_died(ch) )
		return retcode;

	rprog_greet_dir_trigger(ch, rev_dir[pexit->vdir]);
	if ( char_died(ch) )
		return retcode;

	rprog_enter_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	mprog_greet_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	oprog_greet_trigger( ch );
	if ( char_died(ch) )
		return retcode;

	oprog_greet_dir_trigger(ch, rev_dir[pexit->vdir]);
	if ( char_died(ch) )
		return retcode;

	if (!will_fall( ch, fall )
		&&	 fall > 0 )
	{
		if (!IS_AFFECTED( ch, AFF_FLOATING )
			|| ( ch->GetMount() && !IS_AFFECTED( ch->GetMount(), AFF_FLOATING ) ) )
		{
			set_char_color( AT_HURT, ch );
			send_to_char( "OUCH! You hit the ground!\n\r", ch );
			ch->AddWait( 20 );
			retcode = damage( ch, ch, 20 * fall, TYPE_UNDEFINED, 0 );
		}
		else
		{
			set_char_color( AT_MAGIC, ch );
			send_to_char( "You lightly float down to the ground.\n\r", ch );
		}
	}
	return retcode;
}


void do_north(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_NORTH), 0 );
    return;
}


void do_east(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_EAST), 0 );
    return;
}


void do_south(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_SOUTH), 0 );
    return;
}


void do_west(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_WEST), 0 );
    return;
}


void do_up(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_UP), 0 );
    return;
}


void do_down(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_DOWN), 0 );
    return;
}

void do_northeast(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_NORTHEAST), 0 );
    return;
}

void do_northwest(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_NORTHWEST), 0 );
    return;
}

void do_southeast(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_SOUTHEAST), 0 );
    return;
}

void do_southwest(CHAR_DATA *ch, const char* argument)
{
    move_char( ch, get_exit(ch->GetInRoom(), DIR_SOUTHWEST), 0 );
    return;
}


/*
 * Barricade command - added by Ksilyan. Prevent people from taking
 * an exit as long as the blocker is alive.
 * The barricader stores the direction s/he is blocking. The move
 * routine looks at characters and checks if anyone is blocking,
 * and if so, stops them. If the exit has the BARRICADE flag, then
 * the blocker must be killed, or must choose to unbarricade, for
 * the exit to be "walkable". If the exit does not have the flag,
 * then the blocker's barricade is removed if s/he successfully
 * blocks somebody. This check is based on dexterity (speed) and
 * is affected by the amount of people blocking.
 */
void do_barricade(Character * ch, const char* argument)
{
	ExitData * pexit;
	char exitName[MAX_INPUT_LENGTH];

	if ( !argument || strlen(argument) == 0 )
	{
		// No argument means stop barricading.
		if ( ch->BarricadeDir == -1 )
		{
			ch->sendText("You're not barricading anything.\r\n");
			return;
		}
		else
		{
			string buf;
			buf = "You stop barricading the " + string(dir_name_ern[ch->BarricadeDir]) + " exit.";
			act( AT_ACTION, buf.c_str(), ch, NULL, NULL, TO_CHAR );
			buf = "$n stops barricading the " + string(dir_name_ern[ch->BarricadeDir]) + " exit.";
			act( AT_ACTION, buf.c_str(), ch, NULL, NULL, TO_ROOM );
			ch->BarricadeDir = -1;
			return;
		}
	}

	argument = one_argument(argument, exitName);

	if ( ( pexit = find_door( ch, exitName, true ) ) == NULL )
	{
		ch->sendText("You can't find that exit!\r\n");
		return;
	}

	// Is the player already barricading something?
	if ( ch->BarricadeDir != -1 )
	{
		// Yes...
		ch->sendText("You are already barricading an exit!\r\n");
		return;
	}

	// ok, now set the player's barricade to the exit number.
	ch->BarricadeDir = pexit->vdir;
	string buf;
	buf = "You barricade the " + string(dir_name_ern[ch->BarricadeDir]) + " exit.";
	act( AT_SKILL, buf.c_str(), ch, NULL, NULL, TO_CHAR );
	buf = "$n barricades the " + string(dir_name_ern[ch->BarricadeDir]) + " exit!";
	act( AT_SKILL, buf.c_str(), ch, NULL, NULL, TO_ROOM );

	return;
}



ExitData *find_door( CHAR_DATA *ch, const char *arg, bool quiet )
{
	ExitData *pexit;
	int door;

	if (arg == NULL || !str_cmp(arg,""))
		return NULL;

	pexit = NULL;
	if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
	else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
	else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
	else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
	else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
	else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
	else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
	else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
	else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
	else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
	else
	{
		for ( pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next )
		{
			if ( (quiet || IS_SET(pexit->exit_info, EX_ISDOOR))
				&&	  pexit->keyword_.c_str()
				&&	  nifty_is_name( arg, pexit->keyword_.c_str() ) )
				return pexit;
		}
		if ( !quiet )
			act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
		return NULL;
	}

	if ( (pexit = get_exit( ch->GetInRoom(), door )) == NULL )
	{
		if ( !quiet)
			act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
		return NULL;
	}

	if ( quiet )
		return pexit;

	if ( IS_SET(pexit->exit_info, EX_SECRET) )
	{
		act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
		return NULL;
	}

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
		send_to_char( "You can't do that.\n\r", ch );
		return NULL;
	}

	return pexit;
}


void toggle_bexit_flag( ExitData *pexit, int flag )
{
    ExitData *pexit_rev;

    TOGGLE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	TOGGLE_BIT( pexit_rev->exit_info, flag );
}

void set_bexit_flag( ExitData *pexit, int flag )
{
    ExitData *pexit_rev;

    SET_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	SET_BIT( pexit_rev->exit_info, flag );
}

void remove_bexit_flag( ExitData *pexit, int flag )
{
    ExitData *pexit_rev;

    REMOVE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	REMOVE_BIT( pexit_rev->exit_info, flag );
}

void do_open(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	ExitData *pexit;
	int door;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Open what?\n\r", ch );
		return;
	}

	if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
	{
		/* 'open door' */
		ExitData *pexit_rev;

		if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
		{ send_to_char( "You can't do that.\n\r",      ch ); return; }
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
		{ send_to_char( "It's already open.\n\r",      ch ); return; }
		if (  IS_SET(pexit->exit_info, EX_LOCKED) )
		{ send_to_char( "It's locked.\n\r",            ch ); return; }

		if ( !IS_SET(pexit->exit_info, EX_SECRET)
				||   (pexit->keyword_.length() > 0 && nifty_is_name( arg, pexit->keyword_.c_str() )) )
		{
			act( AT_ACTION, "$n opens the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
			act( AT_ACTION, "You open the $d.", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );
			if ( (pexit_rev = pexit->rexit) != NULL
					&&   pexit_rev->to_room == ch->GetInRoom() )
			{
				CHAR_DATA *rch;

				for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
					act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword_.c_str(), TO_CHAR );
			}
			remove_bexit_flag( pexit, EX_CLOSED );
			if ( (door=pexit->vdir) >= 0 && door < 10 )
				check_room_for_traps( ch, trap_door[door]);
			return;
		}
	}

	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		/* 'open object' */
		if ( obj->item_type != ITEM_CONTAINER )
		{
			ch_printf( ch, "%s is not a container.\n\r", capitalize( obj->shortDesc_.c_str() ) );
			return;
		}
		if ( !IS_SET(obj->value[1], CONT_CLOSED) )
		{
			ch_printf( ch, "%s is already open.\n\r", capitalize( obj->shortDesc_.c_str() ) );
			return;
		}
		if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
		{
			ch_printf( ch, "%s cannot be opened or closed.\n\r", capitalize( obj->shortDesc_.c_str() ) );
			return;
		}
		if ( IS_SET(obj->value[1], CONT_LOCKED) )
		{
			ch_printf( ch, "%s is locked.\n\r", capitalize( obj->shortDesc_.c_str() ) );
			return;
		}

		REMOVE_BIT(obj->value[1], CONT_CLOSED);
		act( AT_ACTION, "You open $p.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n opens $p.", ch, obj, NULL, TO_ROOM );
		check_for_trap( ch, obj, TRAP_OPEN );
		/* Regular storeroom check */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
			save_storeitems_room(ch);
		}
		return;
	}

	ch_printf( ch, "You see no %s here.\n\r", arg );
	return;
}



void do_close(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ExitData *pexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'close door' */
	ExitData *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	act( AT_ACTION, "$n closes the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
	act( AT_ACTION, "You close the $d.", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );

	/* close the other side */
	if ( ( pexit_rev = pexit->rexit ) != NULL
	&&   pexit_rev->to_room == ch->GetInRoom() )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword_.c_str(), TO_CHAR );
	}
	set_bexit_flag( pexit, EX_CLOSED );
	if ( (door=pexit->vdir) >= 0 && door < 10 )
	  check_room_for_traps( ch, trap_door[door]);
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{
          ch_printf( ch, "%s is not a container.\n\r", capitalize( obj->shortDesc_.c_str() ) );
          return;
        }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{
          ch_printf( ch, "%s is already closed.\n\r", capitalize( obj->shortDesc_.c_str() ) );
          return;
        }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        {
          ch_printf( ch, "%s cannot be opened or closed.\n\r", capitalize( obj->shortDesc_.c_str() ) );
          return;
        }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "You close $p.", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "$n closes $p.", ch, obj, NULL, TO_ROOM );
	check_for_trap( ch, obj, TRAP_CLOSE );
        /* Regular storeroom check */
        if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
            save_storeitems_room(ch);
        }
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}


bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	if ( obj->pIndexData->vnum == key || obj->value[0] == key )
	    return TRUE;

    return FALSE;
}


void do_lock(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ExitData *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'lock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword_.length() > 0 && nifty_is_name( arg, pexit->keyword_.c_str() )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    act( AT_ACTION, "$n locks the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
	    set_bexit_flag( pexit, EX_LOCKED );
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n locks $p.", ch, obj, NULL, TO_ROOM );

    /* Regular storeroom check */
    if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
        save_storeitems_room(ch);
    }
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_unlock(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ExitData *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'unlock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword_.length() > 0 && nifty_is_name( arg, pexit->keyword_.c_str() )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    act( AT_ACTION, "$n unlocks the $d.", ch, NULL, pexit->keyword_.c_str(), TO_ROOM );
	    remove_bexit_flag( pexit, EX_LOCKED );
            return;
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
         /* Regular storeroom check */
        if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
            save_storeitems_room(ch);
        }
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}

void do_bashdoor(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *gch;
	ExitData *pexit;
	char       arg [ MAX_INPUT_LENGTH ];

	if ( !IS_NPC( ch )
			&& ch->level < skill_table[gsn_bashdoor]->skill_level[ch->Class] )
	{
		send_to_char( "You're not enough of a warrior to bash doors!\n\r", ch );
		return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Bash what?\n\r", ch );
		return;
	}

	if ( ch->IsFighting() )
	{
		send_to_char( "You can't break off your fight.\n\r", ch );
		return;
	}

	if ( ( pexit = find_door( ch, arg, FALSE ) ) != NULL )
	{
		ROOM_INDEX_DATA *to_room;
		ExitData       *pexit_rev;
		int              chance;
		const char	    *keyword;

		if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
			send_to_char( "Calm down.  It is already open.\n\r", ch );
			return;
		}

		ch->AddWait( skill_table[gsn_bashdoor]->beats );

		if ( IS_SET( pexit->exit_info, EX_SECRET ) )
			keyword = "wall";
		else
			keyword = pexit->keyword_.c_str();
		if ( !IS_NPC(ch) )
			chance = ch->pcdata->learned[gsn_bashdoor] / 2;
		else
			chance = 90;
		if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
			chance /= 3;

		if ( !IS_SET( pexit->exit_info, EX_BASHPROOF )
				&&   ch->move >= 15
				&&   number_percent( ) < ( chance + 4 * ( ch->getStr() - 19 ) ) )
		{
			REMOVE_BIT( pexit->exit_info, EX_CLOSED );
			if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
				REMOVE_BIT( pexit->exit_info, EX_LOCKED );
			SET_BIT( pexit->exit_info, EX_BASHED );

			act(AT_SKILL, "Crash!  You bashed open the $d!", ch, NULL, keyword, TO_CHAR );
			act(AT_SKILL, "$n bashes open the $d!",          ch, NULL, keyword, TO_ROOM );
			learn_from_success(ch, gsn_bashdoor);

			if ( (to_room = pexit->to_room) != NULL
					&&   (pexit_rev = pexit->rexit) != NULL
					&&    pexit_rev->to_room	== ch->GetInRoom() )
			{
				CHAR_DATA *rch;

				REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
				if ( IS_SET( pexit_rev->exit_info, EX_LOCKED ) )
					REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
				SET_BIT( pexit_rev->exit_info, EX_BASHED );

				for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
				{
					act(AT_SKILL, "The $d crashes open!",
							rch, NULL, pexit_rev->keyword_.c_str(), TO_CHAR );
				}
			}
			damage( ch, ch, ( ch->max_hit / 20 ), gsn_bashdoor, 0 );

		}
		else
		{
			act(AT_SKILL, "WHAAAAM!!!  You bash against the $d, but it doesn't budge.",
					ch, NULL, keyword, TO_CHAR );
			act(AT_SKILL, "WHAAAAM!!!  $n bashes against the $d, but it holds strong.",
					ch, NULL, keyword, TO_ROOM );
			damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor, 0 );
			learn_from_failure(ch, gsn_bashdoor);
		}
	}
	else
	{
		act(AT_SKILL, "WHAAAAM!!!  You bash against the wall, but it doesn't budge.",
				ch, NULL, NULL, TO_CHAR );
		act(AT_SKILL, "WHAAAAM!!!  $n bashes against the wall, but it holds strong.",
				ch, NULL, NULL, TO_ROOM );
		damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor, 0 );
		learn_from_failure(ch, gsn_bashdoor);
	}


	// what are we doing here? non-charmed NPCs will attack a player trying
	// to bash a door?

	if ( !char_died( ch ) )
		for ( gch = ch->GetInRoom()->first_person; gch; gch = gch->next_in_room )
		{
			if ( IS_AWAKE( gch )
					&& !gch->IsFighting()
					&& ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
					&& ( ch->level - gch->level <= 4 )
					&& number_bits( 2 ) == 0 )
				multi_hit( gch, ch, TYPE_UNDEFINED );
		}

	return;
}


void do_stand(CHAR_DATA *ch, const char* argument)
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't seem to wake up!\n\r", ch ); return; }

	send_to_char( "You wake and climb quickly to your feet.\n\r", ch );
	act( AT_ACTION, "$n arises from $s slumber.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING:
        send_to_char( "You gather yourself and stand up.\n\r", ch );
	act( AT_ACTION, "$n rises from $s rest.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_SITTING:
	send_to_char( "You move quickly to your feet.\n\r", ch );
	act( AT_ACTION, "$n rises up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    ch->SittingOnId = 0;

    return;
}


void do_sit(CHAR_DATA *ch, const char* argument)
{
    OBJ_DATA *pObj = NULL;
    CHAR_DATA *vch;
    int count = 0;

    if ( argument && argument[0] ) {
        if ( !(pObj = get_obj_list_rev(ch, argument, ch->GetInRoom()->last_content)) ) {
            ch_printf(ch, "You don't see a %s here.\r\n", argument);
            return;
        }
    }

    if ( pObj ) {
        if ( pObj->value[0] == 0 ) {
            send_to_char("You can't sit on that.\r\n", ch);
            return;
        }

        for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next_in_room ) {
            if ( vch->SittingOnId == pObj->GetId() ) {
                count++;
            }
        }

        if ( count >= pObj->value[0] && pObj->GetId() != ch->SittingOnId ) {
            ch_printf(ch, "%s is pretty crowded...\r\n", capitalize(pObj->shortDesc_.c_str()));
            return;
        }
    }

    switch ( ch->position ) {
        case POS_SLEEPING:
            send_to_char("You are asleep...\r\n", ch);
            return;
        case POS_SITTING:
            send_to_char("You are already sitting...\r\n", ch);
            return;
        case POS_RESTING:
            if ( ch->SittingOnId != 0 ) {
                act(AT_ACTION, "You sit up straight in $p.", ch, ObjectMap[ch->SittingOnId], NULL, TO_CHAR);
                act(AT_ACTION, "$n sits up straight in $p.", ch, ObjectMap[ch->SittingOnId], NULL, TO_ROOM);
            } else {
                send_to_char("You sit on the ground.\r\n", ch);
            }
            break;
        case POS_STANDING:
        default:
            if ( pObj ) {
                act(AT_ACTION, "You sit down on $p.", ch, pObj, NULL, TO_CHAR);
                act(AT_ACTION, "$n sits down on $p.", ch, pObj, NULL, TO_ROOM);
                ch->SittingOnId = pObj->GetId();
            } else {
                act(AT_ACTION, "You sit on the ground.", ch, pObj, NULL, TO_CHAR);
                act(AT_ACTION, "$n sits on the ground.", ch, pObj, NULL, TO_ROOM);
            }
            break;
        case POS_FIGHTING:
            send_to_char("You can't do that while fighting!\r\n", ch);
            return;
        case POS_MOUNTED:
            send_to_char("You just might want to dismount first...\r\n", ch);
            return;
    }

    ch->position = POS_SITTING;
}


void do_rest(CHAR_DATA *ch, const char* argument)
{
    OBJ_DATA *pObj = NULL;
    CHAR_DATA *vch;
    int count = 0;

    if ( argument && argument[0] ) {
        if ( !(pObj = get_obj_list_rev(ch, argument, ch->GetInRoom()->last_content)) ) {
            ch_printf(ch, "You don't see a %s here.\r\n", argument);
            return;
        }
    }

    if ( pObj ) {
        if ( pObj->value[0] == 0 ) {
            send_to_char("You can't sit on that.\r\n", ch);
            return;
        }

        for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next_in_room ) {
            if ( vch->SittingOnId == pObj->GetId() ) {
                count++;
            }
        }

        if ( count >= pObj->value[0] && pObj->GetId() != ch->SittingOnId ) {
            ch_printf(ch, "%s is pretty crowded...\r\n", capitalize(pObj->shortDesc_.c_str()));
            return;
        }
    }

    switch ( ch->position ) {
        case POS_SLEEPING:
            send_to_char("You are asleep...\r\n", ch);
            return;
        case POS_RESTING:
            send_to_char("You are already resting...\r\n", ch);
            return;
        case POS_SITTING:
            if ( ch->SittingOnId != 0 ) {
                act(AT_ACTION, "You slunch down on $p.", ch, ch->GetSittingOn(), NULL, TO_CHAR);
                act(AT_ACTION, "$n slunches down on $p.", ch, ch->GetSittingOn(), NULL, TO_ROOM);
            } else {
                send_to_char("You sprawl out on the ground.\r\n", ch);
            }
            break;
        case POS_STANDING:
        default:
            if ( pObj ) {
                act(AT_ACTION, "You throw yourself down on $p.", ch, pObj, NULL, TO_CHAR);
                act(AT_ACTION, "$n throws $mself down on $p.", ch, pObj, NULL, TO_ROOM);
                ch->SittingOnId = pObj->GetId();
            } else {
                act(AT_ACTION, "You sprawl out on the ground.", ch, pObj, NULL, TO_CHAR);
                act(AT_ACTION, "$n sprawls out on the ground.", ch, pObj, NULL, TO_ROOM);
            }
            break;
        case POS_FIGHTING:
            send_to_char("You can't do that while fighting!\r\n", ch);
            return;
        case POS_MOUNTED:
            send_to_char("You just might want to dismount first...\r\n", ch);
            return;
    }

    ch->position = POS_RESTING;

    rprog_rest_trigger( ch );
    return;
}


void do_sleep(CHAR_DATA *ch, const char* argument)
{

	/* Why can't somebody sleep on a bed?
	   if ( ch->SittingOnId != 0 )
	   {
	   ch_printf(ch, "You must get off of the %s, first.\r\n", ch->GetSittingOn()->shortDesc_.c_str());
	   return;
	   }
	 */

	switch ( ch->position )
	{
		case POS_SLEEPING:
			send_to_char( "You are already sleeping.\n\r", ch );
			return;

		case POS_RESTING:
			if ( ch->mental_state > 30 && (number_percent()+10) < ch->mental_state )
			{
				send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
				act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM);
				return;
			}
			send_to_char( "You close your eyes and drift into slumber.\n\r", ch );
			act( AT_ACTION, "$n closes $s eyes and drifts into a deep slumber.", ch, NULL, NULL, TO_ROOM );
			ch->position = POS_SLEEPING;
			break;

		case POS_SITTING:
			if ( ch->mental_state > 30 && (number_percent()+5) < ch->mental_state )
			{
				send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
				act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
				return;
			}
			send_to_char( "You slump over and fall dead asleep.\n\r", ch );
			act( AT_ACTION, "$n nods off and slowly slumps over, dead asleep.", ch, NULL, NULL, TO_ROOM );
			ch->position = POS_SLEEPING;
			break;

		case POS_STANDING:
			if ( ch->mental_state > 30 && number_percent() < ch->mental_state )
			{
				send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
				act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
				return;
			}
			send_to_char( "You collapse into a deep sleep.\n\r", ch );
			act( AT_ACTION, "$n collapses into a deep sleep.", ch, NULL, NULL, TO_ROOM );
			ch->position = POS_SLEEPING;
			break;

		case POS_FIGHTING:
			send_to_char( "You are busy fighting!\n\r", ch );
			return;
		case POS_MOUNTED:
			send_to_char( "You really should dismount first.\n\r", ch );
			return;
	}

	rprog_sleep_trigger( ch );
	return;
}


void do_wake(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
    { act( AT_PLAIN, "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) || victim->position < POS_SLEEPING )
    { act( AT_PLAIN, "You can't seem to wake $M!",  ch, NULL, victim, TO_CHAR );  return; }

    act( AT_ACTION, "You wake $M.", ch, NULL, victim, TO_CHAR );
    victim->position = POS_STANDING;
    act( AT_ACTION, "$n wakes you.", ch, NULL, victim, TO_VICT );
    return;
}


/*
 * teleport a character to another room
 */
void teleportch( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool show )
{
    char buf[MAX_STRING_LENGTH];

    if ( room_is_private( room ) )
      return;
    act( AT_ACTION, "$n disappears suddenly!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    act( AT_ACTION, "$n arrives suddenly!", ch, NULL, NULL, TO_ROOM );
    if ( show )
      do_look( ch, "auto" );
    if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
       act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
       set_char_color( AT_DEAD, ch );
       send_to_char( "Oopsie... you're dead!\n\r", ch );
       sprintf(buf, "%s hit a DEATH TRAP in room %s!",
		     ch->getName().c_str(), vnum_to_dotted(ch->GetInRoom()->vnum) );
       log_string( buf );
       to_channel( buf, CHANNEL_MONITOR, "Monitor", sysdata.level_monitor );
       extract_char( ch, FALSE );
    }
}

void teleport( CHAR_DATA *ch, int room, int flags )
{
    CHAR_DATA *nch, *nch_next;
    ROOM_INDEX_DATA *pRoomIndex;
    bool show;

    pRoomIndex = get_room_index( room );
    if ( !pRoomIndex )
    {
	bug( "teleport: bad room vnum %s", vnum_to_dotted(room) );
	return;
    }

    if ( IS_SET( flags, TELE_SHOWDESC ) )
      show = TRUE;
    else
      show = FALSE;
    if ( !IS_SET( flags, TELE_TRANSALL ) )
    {
	teleportch( ch, pRoomIndex, show );
	return;
    }
    for ( nch = ch->GetInRoom()->first_person; nch; nch = nch_next )
    {
	nch_next = nch->next_in_room;
	teleportch( nch, pRoomIndex, show );
    }
}

/*
 * "Climb" in a certain direction.				-Thoric
 */
void do_climb(CHAR_DATA *ch, const char* argument)
{
    ExitData *pexit;
    //bool found;

    //found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xCLIMB ) )
	    {
		move_char( ch, pexit, 0 );
		return;
	    }
	send_to_char( "You cannot climb here.\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xCLIMB ))
    {
	move_char( ch, pexit, 0 );
	return;
    }
    send_to_char( "You cannot climb there.\n\r", ch );
    return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
void do_enter(CHAR_DATA *ch, const char* argument)
{
	ExitData *pexit;
	//bool found;

	//found = FALSE;
	if ( argument[0] == '\0' )
	{
		for ( pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next )
			if ( IS_SET( pexit->exit_info, EX_xENTER ) )
			{
				move_char( ch, pexit, 0 );
				return;
			}
		send_to_char( "You cannot find an entrance here.\n\r", ch );
		return;
	}

	if ( (pexit = find_door( ch, argument, TRUE )) != NULL
			&&   IS_SET( pexit->exit_info, EX_xENTER ))
	{
		move_char( ch, pexit, 0 );
		return;
	}
	send_to_char( "You cannot enter that.\n\r", ch );
	return;
}

/*
 * Leave through an exit.					-Thoric
 */
void do_leave(CHAR_DATA *ch, const char* argument)
{
	ExitData *pexit;
	//bool found;

	//found = FALSE;
	if ( argument[0] == '\0' )
	{
		for ( pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next )
			if ( IS_SET( pexit->exit_info, EX_xLEAVE ) )
			{
				move_char( ch, pexit, 0 );
				return;
			}
		send_to_char( "You cannot find an exit here.\n\r", ch );
		return;
	}

	if ( (pexit = find_door( ch, argument, TRUE )) != NULL
			&&   IS_SET( pexit->exit_info, EX_xLEAVE ))
	{
		move_char( ch, pexit, 0 );
		return;
	}
	send_to_char( "You cannot leave that way.\n\r", ch );
	return;
}

/* Send the exit_message to the room. */
/* Copy of act( (almost) */
void exit_act( sh_int AType, const char *format, CHAR_DATA *ch, int door, bool bEnter, int type)
{
	char *txt;
	CHAR_DATA *to;

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
		to = NULL;
	else if ( type == TO_CHAR )
		to = ch;
	else
		to = ch->GetInRoom()->first_person;

	/*
	 *     * ACT_SECRETIVE handling
	 *     */
	if ( IS_NPC(ch) && IS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR )
		return;

	if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
	{
		OBJ_DATA *to_obj;

		txt = exit_act_string(format, NULL, ch, door, bEnter);

		if ( IS_SET(to->GetInRoom()->progtypes, ACT_PROG) )
			rprog_act_trigger(txt, to->GetInRoom(), ch, NULL, NULL);

		for ( to_obj = to->GetInRoom()->first_content; to_obj;
				to_obj = to_obj->next_content )
		{
			if ( IS_SET(to_obj->pIndexData->progtypes, ACT_PROG) )
				oprog_act_trigger(txt, to_obj, ch, NULL, NULL);
		}
	}

	/* Anyone feel like telling me the point of looping through the whole
	 *     room when we're only sending to one char anyways..? -- Alty */
	for ( ; to; to = (type == TO_CHAR)
			? NULL : to->next_in_room )
	{
		if ((!to->GetConnection()
					&& (  IS_NPC(to) && !IS_SET(to->pIndexData->progtypes, ACT_PROG) ))
				||   !IS_AWAKE(to) )
			continue;

		if ( type == TO_CHAR && to != ch )
			continue;
		if ( type == TO_ROOM && to == ch )
			continue;

		if ( to != ch
				&& get_trust(to) < LEVEL_IMMORTAL
				&& IS_AFFECTED( ch, AFF_SNEAK)
		   )
			continue;

		txt = exit_act_string(format, to, ch, door, bEnter);

		if (to->GetConnection())
		{
			set_char_color(AType, to);
			send_to_char_color(txt, to);
			/*        write_to_buffer( to->desc, txt, strlen(txt) ); */
		}
		if (MOBtrigger)
		{
			/* Note: use original string, not string with ANSI. -- Alty */
			mprog_act_trigger( txt, to, ch, NULL, NULL );
		}
	}
	MOBtrigger = TRUE;
	return;
}


char *exit_act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
		                       int door, bool bEnter)
{
	static const int dir_reverse [] = { 2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 0
	};

	static const char * const he_she  [] = { "it",  "he",  "she"
	};
	static const char * const him_her [] = { "it",  "him", "her"
	};
	static const char * const his_her [] = { "its", "his", "her"
	};
	static char buf[MAX_STRING_LENGTH];
	char *point = buf;
	const char *str = format;
	const char *i;

	if ( bEnter )
		door = dir_reverse[door];

	while ( *str != '\0' )
	{
		if ( *str != '$' )
		{
			*point++ = *str++;
			continue;
		}
		++str;

		switch ( *str )
		{
			default:
				bug( "Exit_Act: bad code %c.", *str );
				i = " <@@@> ";
				break;
			case 'x':
				i = dir_name[door];
				break;
			case 'l':
				i = dir_name_ly[door];
				break;

			case 'n':
				i = (to ? PERS( ch, to) : NAME( ch));
				break;

			case 'e':
				if (ch->sex > 2 || ch->sex < 0)
				{
					bug("act_string: player %s has sex set at %d!", ch->getName().c_str(),
							ch->sex);
					i = "it";
				}
				else
					i = he_she [URANGE(0,  ch->sex, 2)];
				break;

			case 'm':
				if (ch->sex > 2 || ch->sex < 0)
				{
					bug("act_string: player %s has sex set at %d!", ch->getName().c_str(),
							ch->sex);
					i = "it";
				}
				else
					i = him_her[URANGE(0,  ch->sex, 2)];
				break;

			case 's':
				if (ch->sex > 2 || ch->sex < 0)
				{
					bug("act_string: player %s has sex set at %d!", ch->getName().c_str(),
							ch->sex);
					i = "its";
				}
				else
					i = his_her[URANGE(0,  ch->sex, 2)];
				break;
		}
		++str;

		while ( (*point = *i) != '\0' )
			++point, ++i;
	}


	strcpy(point, "\n\r");
	buf[0] = UPPER(buf[0]);
	return buf;
}
#undef NAME

/* show a message when a char moves in from a certain direction
   show default description otherwise
 */
void look_from_dir(CHAR_DATA* ch, int dir)
{
	const char *desc_name;
	ROOM_INDEX_DATA *pRoom = ch->GetInRoom();
	const char *pdesc;

	switch ( dir ) {
		case 0:     desc_name = "desc_south";       break;
		case 1:     desc_name = "desc_west";        break;
		case 2:     desc_name = "desc_north";       break;
		case 3:     desc_name = "desc_east";        break;
		case 4:     desc_name = "desc_below";       break;
		case 5:     desc_name = "desc_above";       break;
		case 6:     desc_name = "desc_southwest";   break;
		case 7:     desc_name = "desc_southeast";   break;
		case 8:     desc_name = "desc_northwest";   break;
		case 9:     desc_name = "desc_northeast";   break;
		default:    desc_name = "desc_somewhere";   break;
	}

	if ( (pdesc = get_extra_descr(desc_name, pRoom->first_extradesc) ) ) {
		set_char_color(AT_RMNAME, ch);
		send_to_char(pRoom->name_.c_str(), ch);
		send_to_char("\r\n", ch);
		set_char_color(AT_RMDESC, ch);

		if ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF) ) {
			send_to_char(pdesc, ch);
		}

		if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOMAP) ) {
			if ( pRoom->map ) {
				do_lookmap(ch, NULL);
			}
		}

		if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) ) {
			do_exits(ch, "auto" );
		}

		show_list_to_char(ch->GetInRoom()->first_content, ch, FALSE, FALSE);
		show_char_to_char(ch->GetInRoom()->first_person, ch);

		return;
	}

	do_look(ch, "auto");

}

const char *rev_exit( sh_int vdir )
{
    switch( vdir )
    {
    default: return "somewhere";
    case 0:  return "the south";
    case 1:  return "the west";
    case 2:  return "the north";
    case 3:  return "the east";
    case 4:  return "below";
    case 5:  return "above";
    case 6:  return "the southwest";
    case 7:  return "the southeast";
    case 8:  return "the northwest";
    case 9:  return "the northeast";
    }

    return "<\?\?\?>";
}

