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
 *			   Wizard/god command module			    *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef unix
	#include <unistd.h>
	#include <crypt.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"
#include "mxp.h"
#include "World.h"
#include "connection.h"
#include "connection_manager.h"
#include "object.h"
#include "utility_objs.hpp"

#include "commands.h"

#include "db_public.h"

#include "paths.const.h"

#include "ScentController.h"

#include "Scent.h"

// STL includes
#include <algorithm>
#include <iostream>
#include <sstream>

#define RESTORE_INTERVAL 21600

const char * const save_flag[] =
{ "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
"auction", "get", "receive", "idle", "backup", "r13", "r14", "r15", "r16",
"r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
"r28", "r29", "r30", "r31" };


/* from comm.c */
//bool	write_to_descriptor	 ( int desc, char *txt, int length ) ;

/*
 * Local functions.
 */
ROOM_INDEX_DATA * find_location	 ( CHAR_DATA *ch, char *arg ) ;
void              save_banlist   ( void ) ;
void              close_area     ( AREA_DATA *pArea ) ;

int               get_color (const char *argument); /* function proto */

AREA_DATA *get_area_obj( OBJ_INDEX_DATA * pObjIndex );
AREA_DATA *get_area_mob( MOB_INDEX_DATA * pMobIndex );


/*
 * Global variables.
 */

char reboot_time[50];
time_t secNewBoot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA * room_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];


/*
	KSILYAN'S TOY
	just a test toy thing.
*/

void do_ksilyantoy(CHAR_DATA *ch, const char* argument)
{
	//ch->sendText("\e[1;1H");
	//ch->sendText("Hello\n\r");

	/*Object * obj;

	obj = get_obj_list( ch, argument, ch->first_carrying );

	if ( !obj )
		obj = get_obj_list( ch, argument, ch->GetInRoom()->first_content );

	if ( !obj )
	{
		ch->sendText("No such object.\n\r");
		return;
	}

	ch->sendText( MasterDatabase->ShowObject(obj) );*/

	/*class MyEvent : public Event
	{
	public:

		MyEvent (const int iTime,
			const string sMsg,
			Character * ch,
			const bool bRepeat = false,
			const long iSequence = 0)
			: Event (iTime, bRepeat, iSequence),
			m_sMsg (sMsg), m_ch(ch)
		{ };

		virtual void OnEvent (void)
		{
			m_ch->sendText(m_sMsg);
		};

	private:

		const string m_sMsg;
		Character * m_ch;

	};	// end of class myevent_A

	MyEvent * event = new	MyEvent (5000,
			"hello there!",
			ch,
			false
			);
	gTheWorld->AddEvent(event);*/

	/*list<SocketGeneral*>::iterator itor;

	list<SocketGeneral*> localList;

	//localList.push_back(*(gConnectionManager->GetSocketsBegin()));
	//localList.push_front(*(gConnectionManager->GetSocketsBegin()++));

	localList.insert (localList.end(), gConnectionManager->GetSocketsBegin(), gConnectionManager->GetSocketsEnd());

	for (itor = gConnectionManager->GetSocketsBegin(); itor != gConnectionManager->GetSocketsEnd(); itor++)
	{
		bool result = listContains(localList, (*itor));

		ch_printf(ch, "For descriptor %d, the answer is: ", (*itor)->GetDescriptor());

		if (result)
			ch->sendText("True\n");
		else
			ch->sendText("False\n");
	}*/

	/*send_to_char("\e[1z", ch);
	send_to_char("<FRAME Name=\"Status\" Height=\"20c\" Floating>\n", ch);
	send_to_char("\e[1z", ch);
	send_to_char("<send href=\"say Hello\">Hello!</send>\n\r", ch);
	send_to_char("\e[1z", ch);
	send_to_char("<DEST Status>Hello!!!</DEST>\n\r", ch);
	send_to_char("\e[1z", ch);
	send_to_char("<IMG tessle.gif>\n", ch);
	send_to_char("\e[1z", ch);
	send_to_char("<FRAME Status REDIRECT>Hello there.<FRAME _previous REDIRECT>\n", ch);*/

	ch->sendText(MXPTAG("font face='Times New Roman'") "Hello!" MXPTAG("/font") MXPTAG("font color=red") MXPTAG("B") "YO" MXPTAG("/B") MXPTAG("/font") "\r\n");
}

// AIYAN - SCENT SYSTEM - 27/03/05

void do_scentedit(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char("Syntax: ScentEdit <List/View/Edit/Create/Save/Delete>\n\r", ch);
		return;
	}

	// Display all scents - welcome to Hacksville, population me
	if ( !str_cmp( arg, "list" ) )
	{
		list<Scent*>::iterator it;

		if ( ScentController::instance()->firstScent() == ScentController::instance()->lastScent() )
		{
			send_to_char("No scents found.\n\r", ch);
			return;
		}

		send_to_char("\n\rCurrent Scents: \n\r", ch);

		for ( it = ScentController::instance()->firstScent(); it != ScentController::instance()->lastScent(); ++it )
		{
			ch_printf(ch, "%d) %s\n\r", (*it)->getID(), (*it)->getName().c_str());
		}

		return;
	}

	if ( !str_cmp( arg, "view" ) )
	{
		Scent *TempScent = NULL;

		if ( argument[0] == '\0' )
		{
			send_to_char("Syntax: Scent view <ID>\n\r", ch);
			return;
		}

		if ( !is_number(argument) )
		{
			send_to_char("Scent view argument must be numeric.\n\r", ch);
			return;
		}

		int scentID = atoi(argument);

		if (( TempScent = ScentController::instance()->findScent(scentID)) == NULL )
		{
			send_to_char("No scent with that ID.\n\r", ch);
			return;
		}

		ch_printf(ch, "\n\rScent Data\n\r");
		ch_printf(ch, "Scent ID: %d\nScent Name: %s\nScent Description: %s\n\r", TempScent->getID(), TempScent->getName().c_str(), TempScent->getDescription().c_str());

		return;
	}

	if ( !str_cmp( arg, "edit" ) )
	{

		argument = one_argument(argument, arg);
		Scent *editScent = NULL;

		if ( arg[0] == '\0' )
		{
			send_to_char("Syntax: Scent Edit <ID> <Name/Desc> <Text>\n\r", ch);
			return;
		}

		if ( !is_number(arg) )
		{
			send_to_char("ID must be numeric.\n\r", ch);
			return;
		}

		int scentID = atoi(arg);

		if (( editScent = ScentController::instance()->findScent(scentID)) == NULL )
		{
			send_to_char("No scent with that ID.\n\r", ch);
			return;
		}

		argument = one_argument(argument, arg2);

		if ( arg2[0] == '\0' )
		{
			send_to_char("Edit name or desc?\n\r", ch);
			return;
		}

		if ( !str_cmp( arg2, "name" ) )
		{
			editScent->setName(argument);
			send_to_char("Scent name set.\n\r", ch);
			return;
		}

		if ( !str_cmp( arg2, "desc" ) )
		{
			editScent->setDescription(argument);
			send_to_char("Scent description set.\n\r", ch);
			return;
		}

		return;
	}

	if ( !str_cmp( arg, "create" ) )
	{
		argument = one_argument(argument, arg);
		argument = one_argument(argument, arg2);

		if ( arg[0] == '\0' )
		{
			send_to_char("Syntax: ScentEdit create <ID> <Name> <Description>\n\r", ch);
			return;
		}

		if ( arg2[0] == '\0' )
		{
			send_to_char("Syntax: ScentEdit Create ID <Name> <Description>\n\r", ch);
			return;
		}

		if ( argument[0] == '\0' )
		{
			send_to_char("Syntax: ScentEdit Create ID Name <Description>\n\r", ch);
			return;
		}

		if ( !is_number(arg) )
		{
			send_to_char("ID must be numberic.\n\r", ch);
			return;
		}

		int scentID = atoi(arg);

		if ( ScentController::instance()->findScent(scentID) != NULL )
		{
			send_to_char("A scent with that ID already exists.\n\r", ch);
			return;
		}

		Scent *NewScent = new Scent(scentID, arg2, argument);

		ScentController::instance()->addScent(NewScent);

		send_to_char("Scent added.\n\r", ch);

		return;
	}

	if ( !str_cmp( arg, "delete" ) )
	{
		Scent * deleteScent = NULL;

		if ( argument[0] == '\0' )
		{
			send_to_char("Syntax: ScentEdit Delete <ID>\n\r", ch);
			return;
		}

		if ( !is_number(argument ) )
		{
			send_to_char("ID must be numeric.\n\r", ch);
			return;
		}

		int scentID = atoi(argument);

		if (( deleteScent = ScentController::instance()->findScent(scentID)) == NULL )
		{
			send_to_char("No such scent with that ID.\n\r", ch);
			return;
		}

		ScentController::instance()->deleteScent(deleteScent);

		send_to_char("Scent deleted.\n\r", ch);

	}

	if ( !str_cmp( arg, "save" ) )
	{
		send_to_char("Saving scents\n\r", ch);
		ScentController::instance()->saveScents();
		return;
	}

	return;
}


int get_saveflag( const char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(save_flag) / sizeof(save_flag[0]); x++ )
      if ( !str_cmp( name, save_flag[x] ) )
        return x;
    return -1;
}

void do_wizhelp(CHAR_DATA *ch, const char* argument)
{
    CMDTYPE * cmd;
    char cmdprefix[MAX_INPUT_LENGTH] = "\0";
    int col, hash;
    int low = 0, high = 0;

    if ( !argument || argument[0] == '\0' )
    {
        low = LEVEL_HERO_MIN;
        high = get_trust(ch);
    }
    else
    {
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if ( is_number(arg1) ) {
            low = atoi(arg1);
            high = low;
        } else {
            low = LEVEL_HERO_MIN;
            high = get_trust(ch);
            strcpy(cmdprefix, arg1);
        }

        if ( cmdprefix[0] == '\0' && is_number(arg2) ) {
            high = atoi(arg2);
        }

        if ( high > get_trust(ch) ) {
            high = get_trust(ch);
        }

        if ( low < LEVEL_HERO_MIN ) {
            low = LEVEL_HERO_MIN;
        }

        if ( high < LEVEL_HERO_MIN )  {
            high = get_trust(ch);
        }
    }

    col = 0;

    set_pager_color( AT_IMMORT, ch );

    ch_printf(ch, "Immortal commands available from level %d to level %d.\r\n", low, high);

    if ( cmdprefix[0] != '\0' ) {
        ch_printf(ch, "Only listing commands that start with '%s'.\r\n",
                      cmdprefix);
    }

    set_pager_color( AT_PLAIN, ch );

    size_t maxLength = 1;

    // Do a first pass to get the highest width
    for ( hash = 0; hash < 126; hash++ ) {
        for ( cmd = command_hash[hash]; cmd; cmd = cmd->next ) {
            if ( cmd->level >= low
            &&   cmd->level <= high )
            {
                if ( cmdprefix[0] != '\0'
                &&   str_prefix(cmdprefix, cmd->name) ) {
                    continue;
                }

                maxLength = max(strlen(cmd->name), maxLength);
            }
        }
    }

    // Now, print with that width
    int numCols = 80/maxLength;
    char formatStr[20];
    sprintf(formatStr, "%%-%lus  ", maxLength);

    for ( hash = 0; hash < 126; hash++ ) {
        for ( cmd = command_hash[hash]; cmd; cmd = cmd->next ) {
            if ( cmd->level >= low
            &&   cmd->level <= high )
            {
                if ( cmdprefix[0] != '\0'
                &&   str_prefix(cmdprefix, cmd->name) ) {
                    continue;
                }

                pager_printf( ch, formatStr, cmd->name );

                if ( ++col % numCols == 0 )
                    send_to_pager( "\n\r", ch );
            }
        }
    }

    if ( col % numCols != 0 )
        send_to_pager( "\n\r", ch );

    return;
}




void do_restrict(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	sh_int level, hash;
	CMDTYPE *cmd;
	bool found;

	found = FALSE;

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Restrict which command?\n\r", ch );
		return;
	}

	argument = one_argument ( argument, arg2 );
	if ( arg2[0] == '\0' )
		level = get_trust( ch );
	else
		level = atoi( arg2 );

	level = UMAX( UMIN( get_trust( ch ), level ), 0 );

	hash = arg[0] % 126;
	for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	{
		if ( !str_prefix( arg, cmd->name )
				&&    cmd->level <= get_trust( ch ) )
		{
			found = TRUE;
			break;
		}
	}

	if ( found )
	{
		if ( !str_prefix( arg2, "show" ) )
		{
			sprintf(buf, "%s show", cmd->name);
			do_cedit(ch, buf);
			/*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
			return;
		}
		cmd->level = level;
		ch_printf( ch, "You restrict %s to level %d\n\r",
				cmd->name, level );
		sprintf( buf, "%s restricting %s to level %d",
				ch->getName().c_str(), cmd->name, level );
		log_string( buf );
	}
	else
		send_to_char( "You may not restrict that command.\n\r", ch );

	return;
}

/*
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc( CHAR_DATA *ch, char *name )
{
	CHAR_DATA		*ret_char;
	static unsigned int number_of_hits;

	number_of_hits = 0;
	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if ( d->GetCharacter() && (!str_prefix( name, d->GetCharacter()->getName().c_str() )) &&
			IS_WAITING_FOR_AUTH(d->GetCharacter()) )
		{
			if ( ++number_of_hits > 1 )
			{
				ch_printf( ch, "%s does not uniquely identify a char.\n\r", name );
				return NULL;
			}
			ret_char = d->GetCharacter();	   /* return current char on exit */
		}
	}
	if ( number_of_hits==1 )
		return ret_char;
	else
	{
		send_to_char( "No one like that waiting for authorization.\n\r", ch );
		return NULL;
	}
}

void do_authorize(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Usage:  authorize <player> <yes|name|no/deny>\n\r", ch );
		send_to_char( "Pending authorizations:\n\r", ch );
		send_to_char( " Chosen Character Name\n\r", ch );
		send_to_char( "---------------------------------------------\n\r", ch );

		itorSocketId itor;
		for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
		{
			// we know that the world's player connection list only holds player connections IDs,
			// so we can safely cast it to PlayerConnection*
			PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

			if ( (victim = d->GetCharacter()) != NULL && IS_WAITING_FOR_AUTH(victim) )
				ch_printf( ch, " %s@%s new %s %s...\n\r",
				           victim->getName().c_str(),
				           victim->GetConnection()->GetHost(),
				           race_table[victim->race].race_name,
				           class_table[victim->Class]->whoName_.c_str() );
		}
		return;
	}

	victim = get_waiting_desc( ch, arg1 );
	if ( victim == NULL )
		return;

	if ( arg2[0]=='\0' || !str_cmp( arg2,"accept" ) || !str_cmp( arg2,"yes" ))
	{
		victim->pcdata->auth_state = 3;
		victim->pcdata->authedBy_ = ch->getName();
		sprintf( buf, "%s authorized %s", ch->getName().c_str(),
			victim->getName().c_str() );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->level );
		ch_printf( ch, "You have authorized %s.\n\r", victim->getName().c_str());

		/* Below sends a message to player when name is accepted - Brittany   */

		ch_printf( victim,											  /* B */
			"The MUD Administrators have accepted the name %s.\n\r" 	  /* B */
			"You will be authorized to enter the Realms at the end of "   /* B */
			"this area.\n\r",victim->getName().c_str()); 							  /* B */
		return;
	}
	else if ( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
	{
		send_to_char( "You have been denied access.\n\r", victim);
		sprintf( buf, "%s denied authorization to %s", ch->getName().c_str(),
			victim->getName().c_str() );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->level );
		ch_printf( ch, "You have denied %s.\n\r", victim->getName().c_str());
		do_quit(victim, "");
	}

	else if ( !str_cmp( arg2, "name" ) || !str_cmp(arg2, "n" ) )
	{
		victim->pcdata->auth_state = 2;
		sprintf( buf, "%s has denied %s's name", ch->getName().c_str(),
			victim->getName().c_str() );
		to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->level );
		ch_printf (victim,
			"The MUD Administrators have found the name %s "
			"to be unacceptable.\n\r"
			"You may choose a new name when you reach " 			  /* B */
			"the end of this area.\n\r" 							  /* B */
			"The name you choose must be medieval and original.\n\r"
			"No titles, descriptive words, or names close to any existing "
			"Immortal's name.\n\r", victim->getName().c_str());
		ch_printf( ch, "You requested %s change names.\n\r", victim->getName().c_str());
		return;
	}

	else
	{
		send_to_char("Invalid argument.\n\r", ch);
		return;
	}
}

void do_bamfin(CHAR_DATA *ch, const char* argument)
{
	if ( !IS_NPC(ch) )
	{
		if ( !strstr(argument, "$n") ) {
			send_to_char("You must include the ever important $n character!\n",
					ch);
			return;
		}
		ch->pcdata->bamfIn_ = SmashTilde(argument);
		send_to_char( "Ok.\n\r", ch );
	}
	return;
}



void do_bamfout(CHAR_DATA *ch, const char* argument)
{
    if ( !IS_NPC(ch) )
    {
        if ( !strstr(argument, "$n") ) {
            send_to_char("You must include the ever important $n character!\n",
                    ch);
            return;
        }
	    ch->pcdata->bamfOut_ = SmashTilde(argument);
	    send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_rank(CHAR_DATA *ch, const char* argument)
{
  if ( IS_NPC(ch) )
    return;

  if ( !argument || argument[0] == '\0' )
  {
    send_to_char( "Usage: rank <string>.\n\r", ch );
    send_to_char( "   or: rank none.\n\r", ch );
    return;
  }

  argument = smash_tilde_static( argument );

  if ( !str_cmp( argument, "none" ) )
    ch->pcdata->rank_ = "";
  else
    ch->pcdata->rank_ = argument;

  send_to_char( "Ok.\n\r", ch );

  return;
}


void do_retire(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Retire whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( victim->level < LEVEL_WANDERER )
    {
	send_to_char( "The minimum level for retirement is savior.\n\r", ch );
	return;
    }

    if ( IS_RETIRED( victim ) )
    {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s returns from retirement.\n\r", victim->getName().c_str() );
      ch_printf( victim, "%s brings you back from retirement.\n\r", ch->getName().c_str() );
    }
    else
    {
      SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s is now a retired immortal.\n\r", victim->getName().c_str() );
      ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\n\r", ch->getName().c_str() );
    }
    return;
}

void do_deny(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    do_quit( victim, "" );

    return;
}



void do_disconnect(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Disconnect whom?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim->GetConnection() == NULL )
	{
		act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( get_trust(ch) <= get_trust( victim ) )
	{
		send_to_char( "They might not like that...\n\r", ch );
		return;
	}

	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if ( d == victim->GetConnection() )
		{
			gConnectionManager->RemoveSocket(d, false);
			send_to_char( "Ok.\n\r", ch );
			return;
		}
	}

	bug( "Do_disconnect: *** desc not found ***.", 0 );
	send_to_char( "Descriptor not found!\n\r", ch );
	return;
}

/*
 * Force a level one player to quit.             Gorog
 */
void do_fquit(CHAR_DATA *ch, const char* argument)
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' )
     {
     send_to_char( "Force whom to quit?\n\r", ch );
     return;
     }

  if ( !( victim = get_char_world( ch, arg1 ) ) )
     {
     send_to_char( "They aren't here.\n\r", ch );
     return;
     }


  send_to_char( "The MUD administrators force you to quit\n\r", victim );
  do_quit (victim, "");
  send_to_char( "Ok.\n\r", ch );
  return;
}


void do_forceclose(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	int desc;

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Usage: forceclose <descriptor#>\n\r", ch );
		return;
	}
	desc = atoi( arg );

	itorSocketId itor;
	for ( itor = gConnectionManager->GetSocketsBegin(); itor != gConnectionManager->GetSocketsEnd(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		SocketGeneral * socket = SocketMap[*itor];

		if ( socket->GetDescriptor() == desc )
		{
			gConnectionManager->RemoveSocket(socket, false);
			send_to_char( "Ok.\n\r", ch );
			return;
		}
	}

	send_to_char( "Not found!\n\r", ch );
	return;
}



void do_pardon(CHAR_DATA *ch, const char* argument)
{
   send_to_char( "This is useless now.\r\n", ch);
   return;
}


void echo_to_all( sh_int AT_COLOR, const char *argument, sh_int tar )
{
	if ( !argument || argument[0] == '\0' )
		return;

	itorSocketId itor;

	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		/* Added showing echoes to players who are editing, so they won't
		miss out on important info like upcoming reboots. --Narn */
		if ( d->ConnectedState == CON_PLAYING || d->ConnectedState == CON_EDITING )
		{
			/* This one is kinda useless except for switched.. */
			if ( tar == ECHOTAR_PC && IS_NPC(d->GetCharacter()) )
				continue;
			else if ( tar == ECHOTAR_IMM && !IS_IMMORTAL(d->GetCharacter()) )
				continue;
			set_char_color( AT_COLOR, d->GetCharacter() );
			send_to_char( argument, d->GetCharacter() );
			send_to_char( "\n\r",	d->GetCharacter() );
		}
	}
	return;
}

void do_echo(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;
    int target;
    const char *parg;

    if ( IS_SET(ch->act, PLR_NO_EMOTE) )
    {
        send_to_char( "You are noemoted and can not echo.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Echo what?\n\r", ch );
        return;
    }

    if ( (color = get_color(argument)) )
        argument = one_argument(argument, arg);
    parg = argument;
    argument = one_argument(argument, arg);
    if ( !str_cmp( arg, "PC" )
         ||   !str_cmp( arg, "player" ) )
        target = ECHOTAR_PC;
    else if ( !str_cmp( arg, "imm" ) )
        target = ECHOTAR_IMM;
    else
    {
        target = ECHOTAR_ALL;
        argument = parg;
    }
    if ( !color && (color = get_color(argument)) )
        argument = one_argument(argument, arg);
    if ( !color )
        color = AT_IMMORT;
    one_argument(argument, arg);

    echo_to_all ( color, argument, target );
}

void echo_to_room( sh_int AT_COLOR, ROOM_INDEX_DATA *room, const char *argument )
{
    CHAR_DATA *vic;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
	set_char_color( AT_COLOR, vic );
	send_to_char( argument, vic );
	send_to_char( "\n\r",   vic );
    }
}

void do_recho(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;

    if ( IS_SET(ch->act, PLR_NO_EMOTE) )
    {
        send_to_char( "You are noemoted and can not recho.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Recho what?\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( (color = get_color ( argument )) )
    {
        argument = one_argument ( argument, arg );
        echo_to_room ( color, ch->GetInRoom(), argument );
    }
    else
       echo_to_room ( AT_IMMORT, ch->GetInRoom(), argument );
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( dotted_to_vnum(ch->GetInRoom()->vnum, arg) > 0 )
    {
        return get_room_index( dotted_to_vnum(ch->GetInRoom()->vnum, arg) );
    }

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
    {
    	return victim->GetInRoom();
    }

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
    {
	    return obj->GetInRoom();
    }

    return NULL;
}


/*
    KSILYAN

    Television functions
*/

void television_talk( OBJ_DATA *obj, const char * text )
{
	string buf;

	buf = string("shows: ") + text;

	if (buf[buf.length() - 2] == '\n')
		buf[buf.length() - 2] = '\0';

	set_supermob(obj);
	MOBtrigger = FALSE;
	do_emote(supermob, (char*)buf.c_str() );
	release_supermob();
}

void do_television_set(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA * room;
    OBJ_DATA * television;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( arg1[0] == '\0' ) || ( arg2[0] == '\0' ) )
    {
        send_to_char( "Use which object as television of which room?\n\r", ch);
        send_to_char( "Usage: television_set [object] [room vnum]\n\r", ch);
        send_to_char( "Use object=none to remove the television.\n\r", ch);
        return;
    }

    if ( !strcmp(arg1, "none") )
    {
        television = NULL;
    }
    else
    {
        television = find_obj(ch, arg1, FALSE);

        if (television == NULL)
        {
                send_to_char( "No such object here!\n\r", ch);
                return;
        }
    }

    room = find_location(ch, arg2);

    if (room == NULL)
    {
        send_to_char( "That room does not exist!\n\r", ch);
        return;
    }

    if (IS_SET(room->room_flags, ROOM_PRIVATE))
    {
        send_to_char( "You can't look at a private room!", ch) ;
        return;
    }

    if (room->TelevisionObjectId != 0)
        ObjectMap[room->TelevisionObjectId]->TelevisionRoomId = 0;

    room->TelevisionObjectId = television->GetId();
    if (television)
        television->TelevisionRoomId = room->GetId();
    send_to_char( "Done!", ch);
}

void do_transfer(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	CHAR_DATA *victim;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Transfer whom (and where)?\n\r", ch );
		return;
	}

	if ( !str_cmp( arg1, "all" ) )
	{
		itorSocketId itor;
		for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
		{
			// we know that the world's player connection list only holds player connections IDs,
			// so we can safely cast it to PlayerConnection*
			PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

			if ( d->ConnectedState == CON_PLAYING
					&&	 d->GetCharacter() != ch
					&&	 d->GetCharacter()->GetInRoom()
					&&	 d->newstate != 2
					&&	 can_see( ch, d->GetCharacter() ) )
			{
				char buf[MAX_STRING_LENGTH];
				sprintf( buf, "%s %s", d->GetCharacter()->getName().c_str(), arg2 );
				do_transfer( ch, buf );
			}
		}
		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if ( arg2[0] == '\0' )
	{
		location = ch->GetInRoom();
	}
	else
	{
		if ( ( location = find_location( ch, arg2 ) ) == NULL )
		{
			send_to_char( "No such location.\n\r", ch );
			return;
		}

		if ( room_is_private( location ) )
		{
			send_to_char( "That room is private right now.\n\r", ch );
			return;
		}
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if (NOT_AUTHED(victim))
	{
		send_to_char( "They are not authorized yet!\n\r", ch);
		return;
	}

	if ( !victim->GetInRoom() )
	{
		send_to_char( "They are in limbo.\n\r", ch );
		return;
	}

	if ( current_arena.pArea && !str_cmp(location->area->filename, current_arena.pArea->filename)) {
		char buf[MAX_STRING_LENGTH];

		arena_clear_char(victim);

		sprintf(buf, "%s decided that %s should enter the arena.", NAME(ch), NAME(victim) );
		talk_channel(ch, buf, CHANNEL_ARENA, "");
	}

	if ( current_arena.pArea && str_cmp(location->area->filename, current_arena.pArea->filename)
			&& in_arena(victim)) {
		char buf[MAX_STRING_LENGTH];

		arena_clear_char(victim);

		sprintf(buf, "%s has decided that %s is not worthy of the arena.", NAME(ch), NAME(victim));
		talk_channel(ch, buf, CHANNEL_ARENA, "");
	}

	if ( victim->IsFighting() )
		victim->StopAllFights();

	act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
	victim->retran = victim->GetInRoom()->vnum;
	char_from_room( victim );
	char_to_room( victim, location );
	act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
	if ( ch != victim )
		act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
	do_look( victim, "auto" );
	send_to_char( "Ok.\n\r", ch );
	if (!IS_IMMORTAL(victim) && !IS_NPC(victim)
			&&	!in_hard_range( victim, location->area ) )
		send_to_char("Warning: the player's level is not within the area's level range.\n\r", ch);
}

void do_retran(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char("Retransfer whom?\n\r", ch );
		return;
	}
	if ( !(victim = get_char_world(ch, arg)) )
	{
		send_to_char("They aren't here.\n\r", ch );
		return;
	}
	sprintf(buf, "'%s' %d", victim->getName().c_str(), victim->retran);
	do_transfer(ch, buf);
	return;
}

void do_fireworks(CHAR_DATA *ch, const char* argument)
{
	string line;

	ifstream infile("../system/fireworks.txt");

	while ( getline(infile, line, '\n' ) )
	{
		act( AT_MAGIC, line.c_str(), ch, NULL, NULL, TO_ROOM);
	}

	infile.close();
}

void do_regoto(CHAR_DATA *ch, const char* argument)
{
	/*
		Fix by Ksilyan to make this work with new
		vnum format.

	char buf[MAX_STRING_LENGTH];

	sprintf(buf, "%d", ch->regoto);*/
	do_goto(ch, vnum_to_dotted(ch->regoto));
	return;
}

void do_at(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) )
    {
      if ( get_trust( ch ) < sysdata.level_override_private )
      {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
      }
      else
      {
	send_to_char( "Overriding private flag!\n\r", ch );
      }

    }

    original = ch->GetInRoom();
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = first_char; wch; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}

void do_rat(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    int Start, End, vnum;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
	return;
    }

    Start = dotted_to_vnum( ch->GetInRoom()->vnum, arg1 );	End = dotted_to_vnum( ch->GetInRoom()->vnum, arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > 1048576000 )
    {
	send_to_char( "Invalid range.\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "quit" ) )
    {
	send_to_char( "I don't think so!\n\r", ch );
	return;
    }

    original = ch->GetInRoom();
    for ( vnum = Start; vnum <= End; vnum++ )
    {
	if ( (location = get_room_index(vnum)) == NULL )
	  continue;
	char_from_room( ch );
	char_to_room( ch, location );
	interpret( ch, argument );
    }

    char_from_room( ch );
    char_to_room( ch, original );
    send_to_char( "Done.\n\r", ch );
    return;
}


void do_rstat(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	ExitData *pexit;
	int cnt;
	static const char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

	one_argument( argument, arg );

	if (!playerbuilderallowed(ch)
			|| playerbuilderbadvnum(ch,ch->GetInRoom()->vnum))
		return;
	if ( !str_cmp( arg, "exits" ) )
	{
		location = ch->GetInRoom();

		ch_printf( ch, "Exits for room '%s.' vnum %s\n\r",
				location->name_.c_str(),
				vnum_to_dotted(location->vnum) );

		for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
		{
			char vto_room[12];
			char vrexit[12];
			char rvnum[12];
			char key[12];

			sprintf(vto_room, "%s", vnum_to_dotted(pexit->to_room ? pexit->to_room->vnum : 0));
			sprintf(vrexit,   "%s", vnum_to_dotted(pexit->rexit   ? pexit->rexit->vnum   : 0));
			sprintf(rvnum,    "%s", vnum_to_dotted(pexit->rvnum                             ));
			sprintf(key,      "%s", vnum_to_dotted(pexit->key                               ));

			ch_printf( ch,
					"%2d) %2s to %s.  Key: %s  Flags: %s  Keywords: '%s'.\n\rDescription: %sExit links back to vnum: %s  Exit's RoomVnum: %s  Distance: %d\n\r",
					++cnt,
					dir_text[pexit->vdir],
					vto_room,
					key,
					exit_number_to_name(pexit->exit_info),
					pexit->keyword_.c_str(),
					pexit->description_.length() > 0
					? pexit->description_.c_str() : "(none).\n\r",
					vrexit,
					rvnum,
					pexit->distance );
		}
		return;
	}
	location = ( arg[0] == '\0' ) ? ch->GetInRoom() : find_location( ch, arg );
	if ( !location )
	{
		send_to_char( "No such location.\n\r", ch );
		return;
	}
	if ( location != ch->GetInRoom() && get_trust(ch) < LEVEL_IMMORTAL)
	{
		send_to_char( "You can only rstat your own location.\n\r", ch);
		return;
	}

	if ( ch->GetInRoom() != location && room_is_private( location ) )
	{
		if ( get_trust( ch ) < sysdata.level_override_private )
		{
			send_to_char( "That room is private right now.\n\r", ch );
			return;
		}
		else
		{
			send_to_char( "Overriding private flag!\n\r", ch );
		}

	}

	ch_printf( ch, "Name: %s.\n\r", location->name_.c_str());
	ch_printf( ch, "Area: %s  Filename: %s.  Vnum: %s (%d/0x%.8x)\n\r",
			location->area ? location->area->name : "None????",
			location->area ? location->area->filename : "None????",
			vnum_to_dotted(location->vnum),
			location->vnum,
			location->vnum);

	ch_printf( ch, "Vnum: %s.  Sector: %s.  Light: %d.  ",
			vnum_to_dotted(location->vnum),
			/*location->sector_type,*/sector_number_to_name(location->sector_type),
			location->light);
	ch_printf( ch, "TeleDelay: %d.  TeleVnum: %s  Tunnel: %d.\n\r",
			location->tele_delay,
			vnum_to_dotted(location->tele_vnum),
			location->tunnel );

	ch_printf( ch, "Scent: %d\n\r", location->scentID );

	ch_printf( ch, "Room flags: %s\n\r",
			flag_string(location->room_flags, r_flags) );
	ch_printf( ch, "Description:\n\r%s", location->description_.c_str() );
	if(location->QuestNoteId != 0)
		ch_printf( ch,"A questnote has been set.\n");

	if ( location->first_extradesc )
	{
		ExtraDescData *ed;

		send_to_char( "Extra description keywords: '", ch );
		for ( ed = location->first_extradesc; ed; ed = ed->next )
		{
			send_to_char( ed->keyword_.c_str(), ch );
			if ( ed->next )
				send_to_char( " ", ch );
		}
		send_to_char( "'.\n\r", ch );
	}

	send_to_char( "Characters:", ch );
	for ( rch = location->first_person; rch; rch = rch->next_in_room )
	{
		if ( can_see( ch, rch ) )
		{
			send_to_char( " ", ch );
			one_argument( rch->getName().c_str(), buf );
			send_to_char( buf, ch );
		}
	}

	send_to_char( ".\n\rObjects:   ", ch );
	for ( obj = location->first_content; obj; obj = obj->next_content )
	{
		send_to_char( " ", ch );
		one_argument( obj->name_.c_str(), buf );
		send_to_char( buf, ch );
	}
	send_to_char( ".\n\r", ch );

	if ( !location->mineMap.empty() )
	{
		for ( std::map<int, int>::iterator it = location->mineMap.begin(); it != location->mineMap.end(); ++it )
		{
			ch_printf( ch, "Mine vnum: %s\tRarity: %d\n\r", vnum_to_dotted((*it).first), (*it).second);
		}
	}

	if (location->TelevisionObjectId != 0)
	{
		ch_printf( ch, "Television object %s, in room %s (vnum: %d)\n\r",
				ObjectMap[location->TelevisionObjectId]->shortDesc_.c_str(),
				ObjectMap[location->TelevisionObjectId]->GetInRoom()->name_.c_str(),
				ObjectMap[location->TelevisionObjectId]->GetInRoom()->vnum);
	}

	if ( location->first_exit )
		send_to_char( "------------------- EXITS -------------------\n\r", ch );
	for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
	{
		ch_printf( ch,
				"%2d) %-2s to %7s.  ",
				++cnt,
				dir_text[pexit->vdir],
				vnum_to_dotted(pexit->to_room ? pexit->to_room->vnum : 0));
		ch_printf( ch, "Key: %s  Flags: %s  Keywords: %s.\n\r",
				vnum_to_dotted(pexit->key),
				exit_number_to_name(pexit->exit_info),
				pexit->keyword_.length() > 0 ? pexit->keyword_.c_str() : "(none)" );
	}
	return;
}

void do_ostat(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int mindamage, maxdamage, average;
	mindamage = maxdamage = average = 0;
	AREA_DATA* area;

	if(!playerbuilderallowed(ch))
		return;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Ostat what?\n\r", ch );
		return;
	}
	if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
		strcpy( arg, argument );

	if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
	{
		send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
		return;
	}

	area = get_area_obj(obj->pIndexData);

	if (get_trust(ch)<LEVEL_IMMORTAL)
	{
		if(ch->pcdata && ch->pcdata->area)
		{
			if(
					obj->pIndexData->vnum < ch->pcdata->area->low_o_vnum
					|| obj->pIndexData->vnum > ch->pcdata->area->hi_o_vnum
			  )
			{
				ch_printf(ch,"That object is outside your assigned range.\n");
				return;
			}

		}
		else
		{
			ch_printf(ch,"You must have an assigned area.\n");
			return;
		}
	}

	ch_printf( ch, "&cName: &C%s&c.\n\r", obj->name_.c_str() );

	ch_printf( ch, "&cVnum: &C%s&c.  Type: &C%s&c.  Count: &C%d  &cGcount: &C%d  &cTotal Cnt: &C%d\n\r",
			vnum_to_dotted(obj->pIndexData->vnum), itemtype_number_to_name( obj->item_type ), obj->pIndexData->count,
			obj->count, obj->pIndexData->total_count );

	ch_printf( ch, "&cSerial#: &C%d  &cTopIdxSerial#: &C%d  &cTopSerial#: &C%d  &cArea filename: &C%s\n\r",
			obj->serial, obj->pIndexData->serial, cur_obj_serial,
			area == NULL? "Unknown" : area->filename );

	ch_printf( ch, "&cShort description: &C%s&c\n\rLong description: &C%s\n\r",
			obj->shortDesc_.c_str(), obj->longDesc_.c_str() );

	if ( obj->actionDesc_.length() > 0 )
		ch_printf( ch, "&cAction description: &C%s&c.\n\r", obj->actionDesc_.c_str() );

	ch_printf( ch, "&cWear flags : &C%s\n\r", flag_string(obj->wear_flags, w_flags) );
	ch_printf( ch, "&cExtra flags: &C%s", flag_string(obj->extra_flags, o_flags) );
	ch_printf( ch, "  &C%s\n\r", flag_string(obj->extra_flags_2, o_flags_2) );

	ch_printf( ch, "&cNumber: &C%d&c/&C%d&c.  Weight: &C%d&c/&C%d&c.  Layers: &C%d\n\r",
			1,           get_obj_number( obj ),
			obj->weight, get_obj_weight( obj ), obj->pIndexData->layers );

#ifdef USE_OBJECT_LEVELS
	ch_printf( ch, "&cCost: &C%d&c.  &cRent: &C%d&c.  Timer: &C%d.  &cLevel: &C%d&c.    Condition:  &g%d&c/&G%d&c\n\r",
			obj->cost, obj->pIndexData->rent, obj->timer, obj->level, obj->condition, obj->max_condition );
#else
	ch_printf( ch, "&cCost: &C%d&c.  &cRent: &C%d&c.  Timer: &C%d.  &cRare:  &C%d&c     Condition:  &C%d&c/&C%d\n\r",
			obj->cost, obj->pIndexData->rent, obj->timer, obj->pIndexData->rare, obj->condition, obj->max_condition);
#endif

	ch_printf( ch,
			"&cIn room: &g%s&c.  &cIn object: &g%s&c.  &cCarried by: &g%s&c.  &cWear_loc: &C%d&c.\n\r",
			vnum_to_dotted(obj->GetInRoom()    == NULL    ?        0 : obj->GetInRoom()->vnum),
			obj->GetInObj()     == NULL    ? "(none)" : obj->GetInObj()->shortDesc_.c_str(),
			obj->GetCarriedBy() == NULL    ? "(none)" : obj->GetCarriedBy()->getName().c_str(),
			obj->wear_loc );

	switch(obj->item_type)
	{

		default:
			ch_printf( ch, "&cIndex Values : &C%d %d %d %d %d %d&c.\n\r",
					obj->pIndexData->value[0], obj->pIndexData->value[1],
					obj->pIndexData->value[2], obj->pIndexData->value[3],
					obj->pIndexData->value[4], obj->pIndexData->value[5] );
			ch_printf( ch, "&cObject Values: &C%d %d %d %d %d %d&c.\n\r",
					obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );
			break;
		case ITEM_GEM:
			ch_printf(ch, "&cGem quality: &g%s &c(&C%d&c).\n\r", gem_quality_number_to_name(obj->value[OBJECT_GEM_QUALITY]), obj->value[OBJECT_GEM_QUALITY]);
			ch_printf(ch, "&cGem size: &g%s &c(&C%d&c).\n\r", gem_size_number_to_name(obj->value[OBJECT_GEM_SIZE]), obj->value[OBJECT_GEM_SIZE]);
			ch_printf(ch, "&cGem worth: &C%d&c (base: &C%d&c).\n\r", GetGemWorth(obj), obj->pIndexData->value[OBJECT_GEM_VALUE]);
			ch_printf(ch, "&r------------------------\n\r");
			if ( IS_SET(obj->value[OBJECT_GEM_FLAGS], GEM_IMBUED) )
				ch_printf(ch, "&cGem imbued with &C%d &gcharges of &g%s.\n\r", obj->value[OBJECT_GEM_CHARGES], skill_table[obj->value[OBJECT_GEM_SPELL]]->name_.c_str());
			else if ( IS_SET(obj->value[OBJECT_GEM_FLAGS], GEM_PURIFIED) )
				ch_printf(ch, "&cThis gem has been purified but has not been imbued with anything.\n\r");
			else
				ch_printf(ch, "&cThis gem has not yet been purified.\n\r");
			break;
		case ITEM_COMPONENT:
			ch_printf(ch, "Quantity in bunch: %d", obj->value[OBJECT_COMPONENT_QUANTITY] );
			break;
		case ITEM_WEAPON:

			mindamage = obj->value[OBJECT_WEAPON_MINDAMAGE];
			maxdamage = obj->value[OBJECT_WEAPON_MAXDAMAGE];
			average = (mindamage + maxdamage) / 2;
			ch_printf(ch, "Damage: %d to %d (average %d)\n\r", mindamage, maxdamage, average);
			if (IS_RANGED_WEAPON(obj->value[OBJECT_WEAPON_WEAPONTYPE]))
			{
				/* Ranged weapon */
				ch_printf(ch, "Weapon type: %s - uses ammo type: %s - power: %d\n",
						weapontype_number_to_name(obj->value[OBJECT_WEAPON_WEAPONTYPE]),
						ammotype_number_to_name(obj->value[OBJECT_WEAPON_DAMAGETYPE]),
						obj->value[OBJECT_WEAPON_POWER] );
				/* remember that for ranged weapons, damagetype = ammo type. */
			}
			else
			{
				ch_printf(ch, "Damage is of type: %s\n\r", damagetype_number_to_name(obj->value[OBJECT_WEAPON_DAMAGETYPE]));
				ch_printf(ch, "Weapon type: %s\n", weapontype_number_to_name(obj->value[OBJECT_WEAPON_WEAPONTYPE]));
				ch_printf(ch, "Damage message: %s\n\r", damagemsg_number_to_name(obj->value[OBJECT_WEAPON_DAMAGEMESSAGE],0) );
			}
			break;
		case ITEM_PROJECTILE:
			mindamage = obj->value[OBJECT_PROJECTILE_MINDAMAGE];
			maxdamage = obj->value[OBJECT_PROJECTILE_MAXDAMAGE];
			average = (mindamage + maxdamage) / 2;
			ch_printf(ch, "Ammo type: %s\n\r", ammotype_number_to_name(obj->value[OBJECT_PROJECTILE_AMMOTYPE]) );
			ch_printf(ch, "Range: %d\n\r", obj->value[OBJECT_PROJECTILE_RANGE]);
			ch_printf(ch, "Damage: %d to %d (average %d)\n\r", mindamage, maxdamage, average);
			ch_printf(ch, "Damage type: %s\n\r", damagetype_number_to_name(obj->value[OBJECT_PROJECTILE_DAMAGETYPE]) );
			ch_printf(ch, "Damage message: %s\n\r", damagemsg_number_to_name(obj->value[OBJECT_PROJECTILE_DAMAGEMESSAGE],0) );
			break;
		case ITEM_ARMOR:
			ch_printf(ch, "Armor class: %d\n", obj->value[OBJECT_ARMOR_AC]);
			ch_printf(ch, "Classification: %d\n", obj->value[OBJECT_ARMOR_CLASSIFICATION]);
			break;
	}


	if ( obj->pIndexData->first_extradesc )
	{
		ExtraDescData *ed;

		send_to_char( "Primary description keywords:   '", ch );
		for ( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
		{
			send_to_char( ed->keyword_.c_str(), ch );
			if ( ed->next )
				send_to_char( " ", ch );
		}
		send_to_char( "'.\n\r", ch );
	}
	if ( obj->first_extradesc )
	{
		ExtraDescData *ed;

		send_to_char( "Secondary description keywords: '", ch );
		for ( ed = obj->first_extradesc; ed; ed = ed->next )
		{
			send_to_char( ed->keyword_.c_str(), ch );
			if ( ed->next )
				send_to_char( " ", ch );
		}
		send_to_char( "'.\n\r", ch );
	}

	// Object holders
	for ( int holderNum = 0; holderNum <= 1; holderNum++ )
	{
		if ( obj->Holders[holderNum]->Name.compare("") ) // not equal ""
		{
			ch_printf(ch, "Holder %d: %s (%s @ %s) on %s.\n\r", holderNum,
					obj->Holders[holderNum]->Name.c_str(), obj->Holders[holderNum]->Account.c_str(),
					obj->Holders[holderNum]->Host.c_str(), obj->Holders[holderNum]->Time.c_str()
					);
		}
	}

	for ( paf = obj->first_affect; paf; paf = paf->next )
		ch_printf( ch, "Affects %s by %d. (extra)\n\r",
				affect_loc_name( paf->location ), paf->modifier );

	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		ch_printf( ch, "Affects %s by %d.\n\r",
				affect_loc_name( paf->location ), paf->modifier );

	return;
}

/* New mstat command by Warp.
 * Same stuff displayed, but in a much niced format.
 * 28.06.2000
 *
 * Updated by Ksilyan feb 26 2002
 * to an even nicer format. :)
 *
 */
void do_mstat(CHAR_DATA* ch, const char* argument)
{
   char        	arg[MAX_INPUT_LENGTH];
   char         logintime[MAX_INPUT_LENGTH];
   char         savetime[MAX_INPUT_LENGTH];
   AFFECT_DATA* paf;
   Character *   vch;
   SkillType *   skill;
   int          linelen, iLang;
   bool			shortformat;
   AREA_DATA* area;

   if(!playerbuilderallowed(ch))
     return;

   set_char_color(AT_PLAIN, ch);
   one_argument(argument, arg);

   shortformat = FALSE;

	if (!str_prefix(arg, "short"))
	{
		shortformat = TRUE;
		strcpy(arg, "");
		argument = one_argument (argument, arg);
		one_argument(argument, arg);
	}

	if(arg[0] == '\0')
	{
		send_to_char("mstat whom?\n\r", ch);
		return;
	}

	if(arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg))
		strcpy(arg, argument);

   if((vch = get_char_world(ch, arg)) == NULL) {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if(!IS_NPC(vch))
   {
     if(playerbuildercannot(ch))
       return;
   }
   else
   {
     if(playerbuilderbadvnum(ch,vch->pIndexData->vnum))
       return;
   }

   if(get_trust(ch) < get_trust(vch) && !IS_NPC(vch)) {
      set_char_color(AT_IMMORT, ch);
      send_to_char("Their godly glow prevents you from getting a good look.\n\r", ch);
      return;
   }

	send_to_char("&c&r---------------------------------------------------------------------------\n\r", ch);

	if(!IS_NPC(vch))
	{
		if(!vch->GetConnection())
			ch_printf(ch, "&cName:  &C%-10s (Link-dead)\n\r", vch->getName().c_str());
		else
		{
			ch_printf(ch, "&cName:  &C%-10s  &cUser: &C%s@%s\n\r",
			vch->getName().c_str(), vch->GetConnection()->User.c_str(), vch->GetConnection()->GetHost());

			ch_printf(ch, "&cDescr: &C%-5d       &cPort: &C%-5d   &cAuthedBy: &C%-10s  &cPlayed: &C%d hours\n\r",
				vch->GetConnection()->GetDescriptor(), vch->GetConnection()->GetPort(),
				vch->pcdata->authedBy_.length() > 0 ?
				vch->pcdata->authedBy_.c_str() : "(unknown)",
				(get_age(vch) - 17) * 2);

			/* Get the time strings */
			sprintf(logintime, "%s", ctime(&(vch->secLogonTime)));
			sprintf(savetime, "%s", vch->secSaveTime ? ctime(&(vch->secSaveTime)) : "no save this session\n");

			/* The strings are "\n" terminated, remove LF from first string */
			memset(logintime+strlen(logintime)-1, 0, 1);

			ch_printf(ch, "&cLogin: &C%s  &cSaved: &C%s\r",logintime, savetime);
		}
		if(vch->pcdata->secReleaseDate != 0)
		ch_printf(ch, "&GHelled until %24.24s by %s.\n\r",
			ctime(&vch->pcdata->secReleaseDate),
			vch->pcdata->helledBy_.c_str());
	}
	else
	{
		area = get_area_mob(vch->pIndexData);
		/* this sucks, but vnum_to_dotted uses a static buffer */
		/* using logintime -- see below */
		strcpy(logintime, vnum_to_dotted(vch->pIndexData->vnum));
		ch_printf(ch, "&cName: &C%s   &cVnum: &C%s (%d/0x%.8x)  &cCount: &C%d\n\r",
		vch->getName().c_str(), logintime /*see above*/, vch->pIndexData->count );
		ch_printf(ch, "&cRoom: &C%s   &cArea Filename: &C%s\n\r",
		vnum_to_dotted(vch->GetInRoom() == NULL ? 0 : vch->GetInRoom()->vnum),
		area == NULL? "Unknown" : area->filename );
	}

	send_to_char("&r---------------------------------------------------------------------------\n\r", ch);

	/* Just misusing logintime variable because I don't want to define
	* a new var for this. This has to be done because capitalize has
	* static buffer. Consider it recycling :) -- Warp
	*/
	strcpy(logintime, capitalize(npc_race[vch->race]));
        ch_printf(ch, "&cLevel: &C%2d   &cRace: &C%-14s[%2d]     &cClass: &C%-14s[%2d]\n\r", vch->level, logintime, vch->race, capitalize(npc_class[vch->Class]), vch->Class);
	ch_printf(ch, "&cTrust: &C%2d   &cAge:  &C%-3d   &cSex: &C%s\n\r",
	vch->trust, get_age(vch),
	vch->sex == SEX_MALE   ? "Male" :
	vch->sex == SEX_FEMALE ? "Female" : "Neutral");

	if(!IS_NPC(vch))
	{
		ch_printf(ch, "&cTitle: &C%s\r\n",vch->pcdata->title_.c_str());
	}
	send_to_char("&r---------------------------------------------------------------------------\n\r", ch);

	ch_printf(ch, "&cStr: &C%2d     &cHP:     &R%5d &r/&R%5d&r/&R%5d   &cPos:     &C%s [%d]\n\r",
		vch->getStr(), vch->hit, vch->max_hit, vch->BaseMaxHp,
		positionstr(vch), vch->position);

	ch_printf(ch, "&cInt: &C%2d     &cMove:   &G%5d &g/&G%5d&g/&G%5d   &cAlign:   &C%s [%d]\n\r",
		vch->getInt(), vch->move, vch->max_move, vch->BaseMaxMove,
		alignstr(vch), vch->alignment);

	ch_printf(ch, "&cWis: &C%2d     &cMana:   &B%5d &b/&B%5d&b/&B%5d   &cGold:    &Y%d\n\r",
		vch->getWis(), vch->mana, vch->max_mana, vch->BaseMaxMana, vch->gold);

	if(IS_VAMPIRE(vch) && !IS_NPC(vch))
	{
		ch_printf(ch, "&cDex: &C%2d     &cBlood:  &R%5d &r/&R%5d         &cExp:     &C%d\n\r",
			vch->getDex(), vch->pcdata->condition[COND_BLOODTHIRST],
			10 + vch->level, vch->exp);
	}
	else
	{
		if(IS_NPC(vch))
			ch_printf(ch, "&cDex: &C%2d     &cAttacks:&C %4d                \n\r",
				vch->getDex(), vch->numattacks );
		else
			ch_printf(ch, "&cDex: &C%2d                                  &cExp:     &C%d\n\r",
				vch->getDex(), vch->exp);
	}

	if(!IS_NPC(vch))
	{
		ch_printf(ch, "&cCon: &C%2d     &cPrac:   &C%5d                &cHitroll: &C%d\n\r",
			vch->getCon(), vch->practice, vch->getHitRoll() );

		ch_printf(ch, "&cCha: &C%2d     &cGlory: &C%4d.%d [Total %-5d]  &cDamroll: &C%d\n\r",
			vch->getCha(), vch->pcdata->quest_curr,
			vch->pcdata->quest_deci, vch->pcdata->quest_accum,
			vch->getDamRoll() );
	}
	else
	{
		ch_printf(ch, "&cCon: &C%2d     &cHitdice: &C%2d&cd&C%-5d &c+&C %-5d    &cHitroll: &C%d\n\r",
			vch->getCon(), vch->pIndexData->hitnodice,
			vch->pIndexData->hitsizedice, vch->pIndexData->hitplus,
			vch->getHitRoll() );

			ch_printf(ch, "&cCha: &C%2d     &cDamdice: &C%2d&cd&C%-5d &c+&C %-5d    &cDamroll: &C%d\n\r",
			vch->getCha(), vch->barenumdie,
			vch->baresizedie, vch->damplus,
			vch->getDamRoll() );
	}

	ch_printf(ch, "&cLck: &C%2d     &cWimpy:  &C%5d                &cAC:      &C%d\n\r",
		vch->getLck(), vch->wimpy, vch->getAC() );

	if(!IS_NPC( vch ))
		ch_printf(ch, "&cBank: &C%d     &cRecall: &C%d\n\r",
			vch->pcdata->bank_gold, vch->pcdata->recall_room );
	else
	{
		ch_printf(ch, "&cExperience Gain Information-            Base XP worth: &C%d\n\r",
			get_exp_worth(vch));
			ch_printf(ch, "&cLevel gains: 2: &C%d   &c10: &C%d   &c20: &C%d   &c30: &C%d    &c40: &C%d    &c50: &C%d\n\r",
			get_exp_worth_for_level(vch, 2), get_exp_worth_for_level(vch, 10), get_exp_worth_for_level(vch, 20),
			get_exp_worth_for_level(vch, 30),get_exp_worth_for_level(vch, 40), get_exp_worth_for_level(vch, 50));
	}

	send_to_char("&r---------------------------------------------------------------------------\n\r", ch);

	if(!IS_NPC(vch))
	{
		if (!shortformat)
		{
			ch_printf(ch, "&cThirst: &C%5d  &cHunger: &C%5d  &cDrunk:   &C%5d  &cMstate:  &C%5d\n\r",
			          vch->pcdata->condition[COND_THIRST],
			          vch->pcdata->condition[COND_FULL],
			          vch->pcdata->condition[COND_DRUNK],
			          vch->mental_state);

			ch_printf(ch, "&cMkills: &C%5d  &cPkills: &R%5d  &cMdeaths: &C%5d  &cPdeaths: &R%5d\n\r",
			          vch->pcdata->mkills, vch->pcdata->pkills,
			          vch->pcdata->mdeaths, vch->pcdata->pdeaths);


			send_to_char("&r---------------------------------------------------------------------------\n\r", ch);
		}

		if(vch->pcdata->clan)
		{
			if(vch->pcdata->clan->clan_type == CLAN_ORDER)
				sprintf(logintime, "Order:");
			else if(vch->pcdata->clan->clan_type == CLAN_GUILD)
				sprintf(logintime, "Guild:");
			else
				sprintf(logintime, "Clan: ");

			ch_printf(ch, "&c%s &C%-10s  &cMember pkills: &C%-5d &cMember pdeaths: &C%-5d\n\r",
				logintime, vch->pcdata->clan->name_.c_str(), vch->pcdata->clan->pkills,
				vch->pcdata->clan->pdeaths);
		}

		if(vch->pcdata->deity)
		{
			ch_printf(ch, "&cDeity:&C %-10s  &cFavor: &C%d\n\r",
				vch->pcdata->deity->name_.c_str(), vch->pcdata->favor);
		}

		if(vch->pcdata->clan || vch->pcdata->deity)
			send_to_char("&r---------------------------------------------------------------------------\n\r", ch);
	}

	if (!shortformat)
	{
		send_to_char("&cLanguages: &g", ch);
		linelen = 11;

		for (iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++)
		{
			if(linelen >= 65)
			{ /* wrap and align with prev line */
				send_to_char("\n\r", ch);
				linelen = 11;
				send_to_char("           ", ch);
			}

			if(lang_array[iLang] & vch->speaking || (IS_NPC(vch) && !vch->speaking))
				set_char_color(AT_GREEN, ch);

			send_to_char(lang_names[iLang], ch);
			send_to_char(" ", ch);
			linelen += strlen(lang_names[iLang]) + 1;
			set_char_color(AT_DGREEN, ch);
		}

		send_to_char("\n\r&r---------------------------------------------------------------------------\n\r", ch);
	}

	if (!shortformat)
	{
		ch_printf(ch, "&cSaves:    &C%2d %2d %2d %2d %2d   &cItems: &C%5d&c/&C%-5d  &cWeight: &C%d&c/&C%d\n\r",
		     vch->saving_poison_death,
	    	 vch->saving_wand,
		     vch->saving_para_petri,
		     vch->saving_breath,
	    	 vch->saving_spell_staff,
		     vch->carry_number, can_carry_n(vch),
		     vch->carry_weight, can_carry_w(vch));


		ch_printf(ch, "&cFighting: &C%-15s  &cMaster: &C%-10s  &cLeader: &C%s\n\r",
		     vch->GetVictim() ? vch->GetVictim()->getName().c_str() : "(none)",
	    	 vch->GetMaster() != NULL ? vch->GetMaster()->getName().c_str()        : "(none)",
		     vch->GetLeader() != NULL ? vch->GetLeader()->getName().c_str()        : "(none)");

		send_to_char("&r---------------------------------------------------------------------------\n\r", ch);
	}

   ch_printf(ch, "&cAct: &C0x%08x  ", vch->act);

	if(IS_NPC(vch))
	{
		ch_printf(ch, "&cAct flags: &C%s\n\r", flag_string(vch->act, act_flags));
		if ( vch->HomeVnum != 0 )
		{
			ROOM_INDEX_DATA * room;
			room = get_room_index(vch->HomeVnum);
			ch_printf(ch, "&cMob home: &g%s &c(&C%s&c)\n\r", room->name_.c_str(), vnum_to_dotted(vch->HomeVnum) );
		}
	}
	else {
      ch_printf(ch, "&cPcflags: &C%s\n\r",
		flag_string(vch->pcdata->flags, pc_flags));
      ch_printf(ch, "&cPlrFlags: &C%s\n\r",
		flag_string(vch->act, plr_flags));
   }

   send_to_char("&r---------------------------------------------------------------------------\n\r", ch);

#if 0 /* Testaur: not conditional on vch->first_affect */
   if(vch->first_affect) {
      ch_printf(ch, "&cAffected by: &C%s\n\r",
		affect_bit_name(vch->affected_by));
   }
#else
   ch_printf(ch,"&cAffected by: &C%s\n\r",affect_bit_name(vch->affected_by));
#endif

	if ( !shortformat && vch->pcdata && vch->pcdata->bestowments_.length() > 0)
		ch_printf(ch, "&cBestowments: &C%s\n\r", vch->pcdata->bestowments_.c_str());

	if ( vch->getShort(false).length() > 0 )
		ch_printf(ch, "&cShort:       &C%s\n\r", vch->getShort(false).c_str());
	else if (IS_NPC(vch))
		ch_printf(ch, "&cShort:       &C(none)\n\r");

	if ( vch->hasStageName() )
		ch_printf(ch, "&cStage name:  &C%s\n\r", vch->getShort().c_str());

	if(vch->getLong().length() > 0 )
		ch_printf(ch, "&cLong:        &C%s", vch->getLong().c_str() );
	else if(IS_NPC(vch))
		ch_printf(ch, "&cLong:        &C(none)");

	if (!shortformat)
	{
		if(vch->exitDesc_.length() > 0)
			ch_printf(ch, "&cExitMsg:     &C%s\r\n", vch->exitDesc_.c_str());
		else if(IS_NPC(vch))
			ch_printf(ch, "&cExitMsg:     &C(none)\n\r");

		if(vch->enterDesc_.length() > 0 )
			ch_printf(ch, "&cEnterMsg:    &C%s\r\n", vch->enterDesc_.c_str());
		else if(IS_NPC(vch))
			ch_printf(ch, "&cEnterMsg:    &C(none)\n\r");

		if(IS_NPC(vch) && vch->spec_fun)
			ch_printf(ch, "&cMobFun:      &C%s\n\r", lookup_spec(vch->spec_fun));
	}

	if(IS_NPC(vch) || vch->immune)
		ch_printf(ch, "&cImmune:      &C%s\n\r", flag_string(vch->immune, ris_flags));

	if(IS_NPC(vch) || vch->resistant)
		ch_printf(ch, "&cResistant:   &C%s\n\r", flag_string(vch->resistant, ris_flags));

	if(IS_NPC(vch) || vch->susceptible)
		ch_printf(ch, "&cSusceptible: &C%s\n\r", flag_string(vch->susceptible, ris_flags));

	if(IS_NPC(vch))
	{
		ch_printf(ch, "&cAttacks:     &C%s\n\r", flag_string(vch->attacks, attack_flags));
		ch_printf(ch, "&cDefenses:    &C%s\n\r", flag_string(vch->defenses, defense_flags));
		ch_printf(ch, "&cBody Parts:  &C%s\n\r", flag_string(vch->xflags, part_flags));
	}

	for(paf = vch->first_affect; paf; paf = paf->next)
		if((skill=get_skilltype(paf->type)) != NULL)
			ch_printf(ch, "&c%s: &G'%s'&g modifies &G%s&g by &G%d&g for &G%d&g rounds with bits &G%s&g\n\r",
			          skill_tname[skill->type], skill->name_.c_str(),
			          affect_loc_name(paf->location),
			          paf->modifier, paf->duration,
			          affect_bit_name(paf->bitvector));

	return;
}

void do_mfind(CHAR_DATA *ch, const char* argument)
{
/*  extern int top_mob_index; */
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
/*  int vnum; */
    int hash;
    int nMatch;
    bool fAll;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
    send_to_char( "Mfind whom?\n\r", ch );
    return;
    }

    fAll	= !str_cmp( arg, "all" );
    nMatch	= 0;
    set_pager_color( AT_PLAIN, ch );

    /*
    * Yeah, so iterating over all vnum's takes 10,000 loops.
    * Get_mob_index is fast, and I don't feel like threading another link.
    * Do you?
    * -- Furey
    */
    /*  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
        {
            if ( fAll || is_name( arg, pMobIndex->player_name ) )
            {
            nMatch++;
            sprintf( buf, "[%5d] %s\n\r",
                pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
            send_to_char( buf, ch );
            }
        }
    }
    */

    /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_mob_index()... which loops itself, an average of 1-2 times...
    * So theoretically, the above routine may loop well over 40,000 times,
    * and my routine bellow will loop for as many index_mobiles are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
    {
        for ( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( fAll || nifty_is_name( arg, pMobIndex->playerName_.c_str() ) )
            {
                nMatch++;
                pager_printf( ch, "[%11s] %s\n\r",
                    vnum_to_dotted(pMobIndex->vnum), capitalize( pMobIndex->shortDesc_.c_str() ) );
            }
        }
    }

    if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}

void do_rfind(CHAR_DATA *ch, const char* argument)
{
    /*  extern int top_obj_index; */
        char arg[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *pRoomIndex;
    /*  int vnum; */
        int hash;
        int nMatch;
        bool fAll;

        one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
        send_to_char( "Rfind what?\n\r", ch );
        return;
        }

        set_pager_color( AT_PLAIN, ch );
        fAll	= !str_cmp( arg, "all" );
        nMatch	= 0;

        for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash];
              pRoomIndex;
              pRoomIndex = pRoomIndex->next )
            if ( fAll || nifty_is_name( arg, pRoomIndex->name_.c_str() ) )
            {
            nMatch++;
            pager_printf( ch, "[%11s] %s\n\r",
                vnum_to_dotted(pRoomIndex->vnum), capitalize( pRoomIndex->name_.c_str() ) );
            }

        if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
        else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

        return;
}

void do_ofind(CHAR_DATA *ch, const char* argument)
{
    /*  extern int top_obj_index; */
        char arg[MAX_INPUT_LENGTH];
        OBJ_INDEX_DATA *pObjIndex;
    /*  int vnum; */
        int hash;
        int nMatch;
        bool fAll;

        one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
        send_to_char( "Ofind what?\n\r", ch );
        return;
        }

        set_pager_color( AT_PLAIN, ch );
        fAll	= !str_cmp( arg, "all" );
        nMatch	= 0;
    /*  nLoop	= 0; */

        /*
         * Yeah, so iterating over all vnum's takes 10,000 loops.
         * Get_obj_index is fast, and I don't feel like threading another link.
         * Do you?
         * -- Furey
        for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        {
        nLoop++;
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
            if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
            {
            nMatch++;
            sprintf( buf, "[%5d] %s\n\r",
                pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
            send_to_char( buf, ch );
            }
        }
        }
         */

        /*
         * This goes through all the hash entry points (1024), and is therefore
         * much faster, though you won't get your vnums in order... oh well. :)
         *
         * Tests show that Furey's method will usually loop 32,000 times, calling
         * get_obj_index()... which loops itself, an average of 2-3 times...
         * So theoretically, the above routine may loop well over 50,000 times,
         * and my routine bellow will loop for as many index_objects are on
         * your mud... likely under 3000 times.
         * -Thoric
         */
        for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash];
              pObjIndex;
              pObjIndex = pObjIndex->next )
            if ( fAll || nifty_is_name( arg, pObjIndex->name_.c_str() ) )
            {
            nMatch++;
            pager_printf( ch, "[%11s] %s\n\r",
                vnum_to_dotted(pObjIndex->vnum), capitalize( pObjIndex->shortDesc_.c_str() ) );
            }

        if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
        else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

        return;
    }



void do_mwhere(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	bool found;

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Mwhere whom?\n\r", ch );
		return;
	}

	set_pager_color( AT_PLAIN, ch );
	found = FALSE;
	for ( vch = first_char; vch; vch = vch->next )
	{
		if ( IS_NPC(vch)
				&&   vch->GetInRoom()
				&&   nifty_is_name( arg, vch->getName().c_str() ) )
		{
			char buf[12];

			/* have to do this, 'cause v2d uses static buf */
			strcpy(buf, vnum_to_dotted(vch->pIndexData->vnum));

			found = TRUE;
			pager_printf( ch, "[%11s] %-28s [%11s] %s\n\r",
					buf,
					vch->getShort().c_str(),
					vnum_to_dotted(vch->GetInRoom()->vnum),
					vch->GetInRoom()->name_.c_str() );
		}
	}

	if ( !found )
		act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

	return;
}


void do_bodybag(CHAR_DATA *ch, const char* argument)
{
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	bool found;

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Bodybag whom?\n\r", ch );
		return;
	}

	/* make sure the buf3 is clear? */
	sprintf(buf3, " ");
	/* check to see if vict is playing? */
	sprintf(buf2,"the corpse of %s",arg);
	found = FALSE;
	for ( obj = first_object; obj; obj = obj->next )
	{
		if ( obj->GetInRoom()
				&& !str_cmp( buf2, obj->shortDesc_.c_str() )
				&& (obj->pIndexData->vnum == 11 ) )
		{
			found = TRUE;
			ch_printf( ch, "Bagging body: [%5d] %-28s [%5d] %s\n\r",
					obj->pIndexData->vnum,
					obj->shortDesc_.c_str(),
					obj->GetInRoom()->vnum,
					obj->GetInRoom()->name_.c_str() );
			obj_from_room(obj);
			obj = obj_to_char(obj, ch);
			obj->timer = -1;
			save_char_obj( ch );
		}
	}

	if ( !found )
		ch_printf(ch," You couldn't find any %s\n\r",buf2);
	return;
}


/* New owhere by Altrag, 03/14/96 */
void do_owhere(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool found;
    int icnt = 0;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Owhere what?\n\r", ch );
        return;
    }
    argument = one_argument(argument, arg1);

    set_pager_color( AT_PLAIN, ch );
    if ( arg1[0] != '\0' && !str_prefix(arg1, "nesthunt") )
    {
        if ( !(obj = get_obj_world(ch, arg)) )
        {
            send_to_char( "Nesthunt for what object?\n\r", ch );
            return;
        }
        for ( ; obj->GetInObj(); obj = obj->GetInObj() )
        {
            /* v2d uses static buffer :( */
            strcpy(buf, vnum_to_dotted(obj->pIndexData->vnum));
            pager_printf(ch, "[%11s] %-28s in object [%11s] %s\n\r",
                        buf, obj_short(obj),
                        vnum_to_dotted(obj->GetInObj()->pIndexData->vnum), obj->GetInObj()->shortDesc_.c_str());
            ++icnt;
        }
        sprintf(buf, "[%11s] %-28s in ", vnum_to_dotted(obj->pIndexData->vnum),
                obj_short(obj));
        if ( obj->GetCarriedBy() )
            sprintf(buf+strlen(buf), "invent [%11s] %s\n\r",
                    vnum_to_dotted((IS_NPC(obj->GetCarriedBy()) ? obj->GetCarriedBy()->pIndexData->vnum
                    : 0)), PERS(obj->GetCarriedBy(), ch));
        else if ( obj->GetInRoom() )
            sprintf(buf+strlen(buf), "room   [%11s] %s\n\r",
                    vnum_to_dotted(obj->GetInRoom()->vnum), obj->GetInRoom()->name_.c_str());
        else if ( obj->GetInObj() )
        {
            bug("do_owhere: obj->in_obj after NULL!",0);
            strcat(buf, "object??\n\r");
        }
        else
        {
            bug("do_owhere: object doesnt have location!",0);
            strcat(buf, "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
        ++icnt;
        pager_printf(ch, "Nested %d levels deep.\n\r", icnt);
        return;
    }

    found = FALSE;
    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( !nifty_is_name( arg, obj->name_.c_str() ) )
            continue;
        found = TRUE;

        sprintf(buf, "(%3d) [%11s] %-28s in ", ++icnt, vnum_to_dotted(obj->pIndexData->vnum),
                obj_short(obj));
        if ( obj->GetCarriedBy() )
            sprintf(buf+strlen(buf), "invent [%11s] %s\n\r",
                    vnum_to_dotted((IS_NPC(obj->GetCarriedBy()) ? obj->GetCarriedBy()->pIndexData->vnum
                    : 0)), PERS(obj->GetCarriedBy(), ch));
        else if ( obj->GetInRoom() )
            sprintf(buf+strlen(buf), "room   [%11s] %s\n\r",
                    vnum_to_dotted(obj->GetInRoom()->vnum), obj->GetInRoom()->name_.c_str());
        else if ( obj->GetInObj() )
            sprintf(buf+strlen(buf), "object [%11s] %s\n\r",
                    vnum_to_dotted(obj->GetInObj()->pIndexData->vnum), obj_short(obj->GetInObj()));
        else
        {
            bug("do_owhere: object doesnt have location!",0);
            strcat(buf, "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
    }

    if ( !found )
        act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    else
        pager_printf(ch, "%d matches.\n\r", icnt);

    return;
}


void do_reboo(CHAR_DATA *ch, const char* argument)
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *vch;

	if ( str_cmp( argument, "mud now" )
        &&   str_cmp( argument, "nosave" )
        &&   str_cmp( argument, "and sort skill table" ) )
	{
        send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\n\r", ch );
        return;
	}

	if ( auction->item )
        do_auction( ch, "stop");

	sprintf( buf, "Reboot by %s.", ch->getName().c_str() );
	do_echo( ch, buf );

	if ( !str_cmp(argument, "and sort skill table") )
	{
        sort_skill_table();
        save_skill_table();
	}

	/* Save all characters before booting. */
	if ( str_cmp(argument, "nosave") )
	{
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

	gGameRunning = false;
	return;
}



void do_shutdow(CHAR_DATA *ch, const char* argument)
{
	send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
	return;
}



void do_shutdown(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *vch;

	if ( str_cmp( argument, "mud now" ) && str_cmp(argument, "nosave") )
	{
        send_to_char( "Syntax: 'shutdown mud now' or 'shutdown nosave'\n\r", ch );
        return;
	}

	if ( auction->item )
        do_auction( ch, "stop");

	sprintf( buf, "Shutdown by %s.", ch->getName().c_str() );
	append_file( ch, SHUTDOWN_FILE, buf );
	strcat( buf, "\n\r" );
	do_echo( ch, buf );

	/* Save all characters before booting. */
	if ( str_cmp(argument, "nosave") )
	{
		AREA_DATA *tarea;
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
	gGameRunning = false;
	return;
}


void do_snoop(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;


	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
        send_to_char( "Snoop whom?\n\r", ch );
        return;
	}

	if ( ( vch = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( !ch->GetConnection() )
	{
		ch->sendText("You have no descriptor.\n\r", false);
		return;
	}

	/*if ( !vch->GetConnection() )
	{
        send_to_char( "No descriptor to snoop.\n\r", ch );
        return;
	}*/

	if ( vch == ch )
	{
		send_to_char( "Cancelling all snoops.\n\r", ch );

		ch->GetConnection()->CancelAllSnoops();
		return;
	}

	/*
	 * Minimum snoop level... a secret mset value
	 * makes the snooper think that the vch is already being snooped
	 */
	if ( !IS_NPC(vch) )
	{
		if ( get_trust( vch ) >= get_trust( ch )
			||  (vch->pcdata && vch->pcdata->min_snoop > get_trust( ch )) )
		{
			send_to_char( "Somebody is already snooping that character.\n\r", ch );
			return;
		}

		if ( vch->GetConnection() &&
			  (vch->GetConnection()->IsSnooping(ch->GetConnection()->OriginalCharId ) ||
			    vch->GetConnection()->IsSnooping(ch->GetConnection()->CurrentCharId) )
			)
		{
			ch->sendText( "No snoop loops.\n\r", false);
			return;
		}
	}

	if ( ch->GetConnection()->IsSnooping(vch) )
	{
		ch->sendText("You're already snooping that character.\n\r");
		return;
	}

	/*if ( ch->GetConnection() )
	{
		PlayerConnection * d;

        for ( d = ch->GetConnection()->snoop_by; d; d = d->snoop_by )
            if ( d->character == vch || d->original == vch )
            {
				send_to_char( "No snoop loops.\n\r", ch );
				return;
            }
	}*/

    /*  Snoop notification for higher imms, if desired, uncomment this
	if ( get_trust(vch) > LEVEL_GOD && get_trust(ch) < LEVEL_SUPREME )
		write_to_descriptor( vch->GetConnection()->descriptor,
		                     "\n\rYou feel like someone is watching your every move...\n\r",
		                     0 );
	*/

	vch->StartSnoopedBy(ch->GetConnection());
	ch->GetConnection()->StartSnooping(vch);

    ch->sendText("Ok.\n\r", false);
    return;
}



void do_switch(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Switch into whom?\n\r", ch );
		return;
	}

	if ( !ch->GetConnection() )
		return;

	if ( ch->GetConnection()->OriginalCharId != 0 )
	{
		send_to_char( "You are already switched.\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Ok.\n\r", ch );
		return;
	}

	if ( victim->GetConnection() )
	{
		send_to_char( "Character in use.\n\r", ch );
		return;
	}

	if ( !IS_NPC(victim) && ch->level < LEVEL_STONE_MASTER )
	{
		send_to_char( "You cannot switch into a player!\n\r", ch );
		return;
	}

	ch->GetConnection()->CurrentCharId = victim->GetId();
	ch->GetConnection()->OriginalCharId = ch->GetId();

	ch->GetConnection()->SetInputReceiver(victim);

	// Do not mess with reference counts since the
	// reference is directly transferred.

	victim->SetConnection(ch->GetConnection());
	ch->SetConnection(NULL);

	ch->SwitchedCharId = victim->GetId();
	send_to_char( "Ok.\n\r", victim );
	return;
}



void do_return(CHAR_DATA *ch, const char* argument)
{
    if ( !ch->GetConnection() )
	return;

    if ( !ch->GetConnection()->GetOriginalCharacter() )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    if (IS_SET(ch->act, ACT_POLYMORPHED))
    {
      send_to_char("Use revert to return from a polymorphed mob.\n\r", ch);
      return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
	if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_POSSESS ) )
	{
		affect_strip( ch, gsn_possess );
		REMOVE_BIT( ch->affected_by, AFF_POSSESS );
	}
/*    if ( IS_NPC( ch->GetConnection()->character ) )
      REMOVE_BIT( ch->GetConnection()->character->affected_by, AFF_POSSESS );*/
    ch->GetConnection()->CurrentCharId = ch->GetConnection()->GetOriginalCharacter()->GetId();
    ch->GetConnection()->OriginalCharId = 0;
    ch->GetConnection()->GetCharacter()->SetConnection( ch->GetConnection() );
    ch->GetConnection()->GetCharacter()->SwitchedCharId = 0;
	ch->GetConnection()->SetInputReceiver(ch->GetConnection()->GetCharacter());
    ch->SetConnection(NULL);

    return;
}



void do_mload(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;
	int vnum;

	if(!playerbuilderallowed(ch))
		return;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Syntax: mload <vnum>.\n\r", ch );
		return;
	}

	if ( (vnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg)) <= 0 )
	{
		char arg2[MAX_INPUT_LENGTH];
		int  hash, cnt;
		int  count = number_argument( arg, arg2 );

		vnum = -1;
		for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
		{
			for ( pMobIndex = mob_index_hash[hash];
					pMobIndex;
					pMobIndex = pMobIndex->next )
			{
				if ( nifty_is_name( arg2, pMobIndex->playerName_.c_str() )
						&&   ++cnt == count )
				{
					vnum = pMobIndex->vnum;
					break;
				}
			}
		}
		if ( vnum == -1 )
		{
			send_to_char( "No such mobile exists.\n\r", ch );
			return;
		}
	}

	if ( get_trust(ch) < LEVEL_IMMORTAL )
	{
		AREA_DATA *pArea;

		if ( IS_NPC(ch) )
		{
			send_to_char( "Huh?\n\r", ch );
			return;
		}

		if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
		{
			send_to_char( "You must have an assigned area to load this mobile.\n\r", ch );
			return;
		}
		if ( vnum < pArea->low_m_vnum
				||   vnum > pArea->hi_m_vnum )
		{
			send_to_char( "That number is not in your allocated range.\n\r", ch );
			return;
		}
	}

	if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
	{
		send_to_char( "No mobile has that vnum.\n\r", ch );
		return;
	}

	/* Commented out by Narn, not sure what it was for.  Anyway, it's a
	   pain for those who use instazone.
	   if ( IS_SET( pMobIndex->act, ACT_PROTOTYPE )
	   &&	 pMobIndex->count > 5 )
	   {
	   send_to_char( "That mobile is at its limit.\n\r", ch );
	   return;
	   }
	 */
	victim = create_mobile( pMobIndex );
	char_to_room( victim, ch->GetInRoom() );
	act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );
	return;
}



void do_oload(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int vnum;
#ifdef USE_OBJECT_LEVELS
   int level;
#endif

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(!playerbuilderallowed(ch))
      return;

    if ( arg1[0] == '\0' )
    {
#ifdef USE_OBJECT_LEVELS
       send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
#else
       send_to_char("Syntax: oload <vnum>.\r\n", ch);
#endif
	return;
    }

#ifdef USE_OBJECT_LEVELS
   if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	if ( !is_number( arg2 ) )
	{
	    send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	    return;
	}
	level = atoi( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
	{
	    send_to_char( "Limited to your trust level.\n\r", ch );
	    return;
        }
    }
#endif

    if ( (vnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg1)) <= 0 )
    {
        char arg[MAX_INPUT_LENGTH];
        int  hash, cnt;
        int  count = number_argument( arg1, arg );

        vnum = -1;

        for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
        {
            for ( pObjIndex = obj_index_hash[hash];
                  pObjIndex; pObjIndex = pObjIndex->next )
            {
                if ( nifty_is_name( arg, pObjIndex->name_.c_str() )
                &&   ++cnt == count )
                {
                    vnum = pObjIndex->vnum;
                    break;
                }
            }
        }

        if ( vnum == -1 )
        {
            send_to_char( "No such object exists.\n\r", ch );
            return;
        }
    }

    if ( get_trust(ch) < LEVEL_IMMORTAL )
    {
	AREA_DATA *pArea;

	if ( IS_NPC(ch) )
	{
	  send_to_char( "Huh?\n\r", ch );
	  return;
	}

	if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
	{
	  send_to_char( "You must have an assigned area to load this object.\n\r", ch );
	  return;
	}
	if ( vnum < pArea->low_o_vnum
	||   vnum > pArea->hi_o_vnum )
	{
	  send_to_char( "That number is not in your allocated range.\n\r", ch );
	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
        sprintf(log_buf, "No object has that vnum (%d).\r\n", vnum);
        send_to_char( log_buf, ch );
        return;
    }

/* Commented out by Narn, it seems outdated
    if ( IS_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE )
    &&	 pObjIndex->count > 5 )
    {
	send_to_char( "That object is at its limit.\n\r", ch );
	return;
    }
*/

    if ( pObjIndex->total_count >= pObjIndex->rare && pObjIndex->rare >= 1 ) {
        send_to_char("That object is at its rare limit.  Purge this one, or die!\r\n", ch);
    }

#ifdef USE_OBJECT_LEVELS
   obj = create_object( pObjIndex, level );
#else
   obj = create_object( pObjIndex, 0);
#endif
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
	obj = obj_to_char( obj, ch );
    }
    else
    {
	obj = obj_to_room( obj, ch->GetInRoom() );
	act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
    }
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->GetInRoom()->first_person; victim; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && victim != ch && !IS_SET(victim->act, ACT_POLYMORPHED))
		extract_char( victim, TRUE );
	}

	for ( obj = ch->GetInRoom()->first_content; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    extract_obj( obj, TRUE );
	}

	act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );

        /* Regular storeroom check */
        if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
            save_storeitems_room(ch);
        }

	return;
    }
    victim = NULL; obj = NULL;

    /* fixed to get things in room first -- i.e., purge portal (obj),
     * no more purging mobs with that keyword in another room first
     * -- Tri */
    if ( ( victim = get_char_room( ch, arg ) ) == NULL
    && ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
      if ( ( victim = get_char_world( ch, arg ) ) == NULL
      &&   ( obj = get_obj_world( ch, arg ) ) == NULL )  /* no get_obj_room */
      {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    }

/* Single object purge in room for high level purge - Scryn 8/12*/
    if ( obj )
    {
	separate_obj( obj );
	act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM);
	act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR);
	extract_obj( obj, TRUE);
	return;
    }


    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
    	send_to_char( "You cannot purge yourself!\n\r", ch );
    	return;
    }

    if (IS_SET(victim->act, ACT_POLYMORPHED))
    {
      send_to_char("You cannot purge a polymorphed player.\n\r", ch);
      return;
    }
    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}


void do_low_purge(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Purge what?\n\r", ch );
	return;
    }

    victim = NULL; obj = NULL;
    if ( ( victim = get_char_room( ch, arg ) ) == NULL
    &&	 ( obj    = get_obj_here ( ch, arg ) ) == NULL )
    {
	send_to_char( "You can't find that here.\n\r", ch );
	return;
    }

    if ( obj )
    {
	separate_obj( obj );
	act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
	act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
	extract_obj( obj, TRUE );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
    	send_to_char( "You cannot purge yourself!\n\r", ch );
    	return;
    }

    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
    extract_char( victim, TRUE );
    return;
}


void do_balzhur(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    AREA_DATA *pArea;
    int sn;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who is deserving of such a fate?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't playing.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim->level >= get_trust( ch ) )
    {
	send_to_char( "I wouldn't even think of that if I were you...\n\r", ch );
	return;
    }

	set_char_color( AT_WHITE, ch );
	send_to_char( "You summon the demon Balzhur to wreak your wrath!\n\r", ch );
	send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r", ch );
	set_char_color( AT_IMMORT, victim );
	send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\n\r", victim );
	sprintf( buf, "Balzhur screams, 'You are MINE %s!!!'", victim->getName().c_str() );
	echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
	victim->level    = 1;
	victim->trust	 = 0;
	victim->exp      = 2000;
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	for ( sn = 0; sn < top_sn; sn++ )
	    victim->pcdata->learned[sn] = 0;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;


    sprintf( buf, "%s%s", GOD_DIR, capitalize(victim->getName().c_str()) );

    if ( !remove( buf ) )
      send_to_char( "Player's immortal data destroyed.\n\r", ch );
    else if ( errno != ENOENT )
    {
      ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Ydnat\n\r",
              errno, strerror( errno ) );
      sprintf( buf2, "%s balzhuring %s", ch->getName().c_str(), buf );
      perror( buf2 );
    }
    sprintf( buf2, "%s.are", capitalize(arg) );
    for ( pArea = first_build; pArea; pArea = pArea->next )
      {
	if ( !strcmp( pArea->filename, buf2 ) )
	  {
	    sprintf( buf, "%s%s", BUILD_DIR, buf2 );
	    if ( IS_SET( pArea->status, AREA_LOADED ) )
	      fold_area( pArea, buf, FALSE );
	    close_area( pArea );
	    sprintf( buf2, "%s.bak", buf );
	    set_char_color( AT_RED, ch ); /* Log message changes colors */
	    if ( !rename( buf, buf2 ) )
	      send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch);
	    else if ( errno != ENOENT )
	      {
		ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Tarin.\n\r",
			   errno, strerror( errno ) );
		sprintf( buf2, "%s destroying %s", ch->getName().c_str(), buf );
		perror( buf2 );
	      }

	    break;
	  }
      }


        make_wizlist();
	advance_level( victim );
	do_help(victim, "M_BALZHUR_" );
	set_char_color( AT_WHITE, victim );
	send_to_char( "You awake after a long period of time...\n\r", victim );
	while ( victim->first_carrying )
	     extract_obj( victim->first_carrying, TRUE );
    return;
}

void do_advance(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;
	int iLevel;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
	{
		send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That player is not here.\n\r", ch);
		return;
	}

	if ( IS_NPC(victim) )
	{
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	}

	/* You can demote yourself but not someone else at your own trust. -- Narn */
	if ( get_trust( ch ) <= get_trust( victim ) && ch != victim )
	{
		send_to_char( "You can't do that.\n\r", ch );
		return;
	}

	if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
	{
		send_to_char( "Level must be 1 to 60.\n\r", ch );
		return;
	}

	if ( level > get_trust( ch ) )
	{
		send_to_char( "Limited to your trust level.\n\r", ch );
		return;
	}

	/*
	 * Lower level:
	 *   Reset to level 1.
	 *   Then raise again.
	 *   Currently, an imp can lower another imp.
	 *   -- Swiftest
	 */
	if ( level <= victim->level )
	{
		int sn;

		send_to_char( "Lowering a player's level!\n\r", ch );
		set_char_color( AT_IMMORT, victim );
		send_to_char( "Cursed and forsaken! The gods have lowered your level.\n\r", victim );
		victim->level    = 1;
		victim->exp      = exp_level(victim, 1);
		victim->max_hit  = 10;
		victim->max_mana = 100;
		victim->max_move = 100;
		for ( sn = 0; sn < top_sn; sn++ )
			victim->pcdata->learned[sn] = 0;
		victim->practice = 0;
		victim->hit      = victim->max_hit;
		victim->mana     = victim->max_mana;
		victim->move     = victim->max_move;
		advance_level( victim );

		/* Rank fix added by Narn. */
		victim->pcdata->rank_ = "";

		/* Stuff added to make sure players wizinvis level doesnt stay higher
		 * than their actual level and to take wizinvis away from advance below 50
		 */
		if (IS_SET (victim->act, PLR_WIZINVIS) )
			victim->pcdata->wizinvis = victim->trust;

		if (IS_SET (victim->act, PLR_WIZINVIS)
				&& (victim->level <= LEVEL_HERO_MIN))
		{
			REMOVE_BIT(victim->act, PLR_WIZINVIS);
			victim->pcdata->wizinvis = victim->trust;
		}
	}
	else
	{
		send_to_char( "Raising a player's level!\n\r", ch );
		if (victim->level >= LEVEL_HERO_MAX)
		{
			/* be sure we pick the appropriate ethos! */
			if(ch->alignment >= 350)
				victim->pcdata->ethos = ETHOS_HERO;
			else if(ch->alignment <= -350)
				victim->pcdata->ethos = ETHOS_VILLAIN;
			else
				victim->pcdata->ethos = ETHOS_SAGE;

			set_char_color( AT_IMMORT, victim );
			act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at you!",
					ch, NULL, victim, TO_VICT );
			act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at $N!",
					ch, NULL, victim, TO_NOTVICT );
			set_char_color( AT_WHITE, victim );
			send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
			set_char_color( AT_LBLUE, victim );
		}

#if 0
		switch(level)
		{
			default:
				send_to_char( "The gods feel fit to raise your level!\n\r", victim );
				break;
				/*	case LEVEL_IMMORTAL:
					do_help(victim, "M_GODLVL1_" );
					set_char_color( AT_WHITE, victim );
					send_to_char( "You awake... all your possessions are gone.\n\r", victim );

					while ( victim->first_carrying )
					extract_obj( victim->first_carrying );
					break;
				 */	case LEVEL_OLD_ACOLYTE:
				do_help(victim, "M_GODLVL2_" );
				break;
			case LEVEL_CREATOR:
				do_help(victim, "M_GODLVL3_" );
				break;
			case LEVEL_SAVIOR:
				do_help(victim, "M_GODLVL4_" );
				break;
			case LEVEL_DEMI:
				do_help(victim, "M_GODLVL5_" );
				break;
			case LEVEL_TRUEIMM:
				do_help(victim, "M_GODLVL6_" );
				break;
			case LEVEL_LESSER:
				do_help(victim, "M_GODLVL7_" );
				break;
			case LEVEL_GOD:
				do_help(victim, "M_GODLVL8_" );
				break;
			case LEVEL_GREATER:
				do_help(victim, "M_GODLVL9_" );
				break;
			case LEVEL_ASCENDANT:
				do_help(victim, "M_GODLVL10_" );
				break;
			case LEVEL_SUB_IMPLEM:
				do_help(victim, "M_GODLVL11_" );
				break;
			case LEVEL_IMPLEMENTOR:
				do_help(victim, "M_GODLVL12_" );
				break;
			case LEVEL_ETERNAL:
				do_help(victim, "M_GODLVL13_" );
				break;
			case LEVEL_INFINITE:
				do_help(victim, "M_GODLVL14_" );
				break;
			case LEVEL_SUPREME:
				do_help(victim, "M_GODLVL15_" );
		}
#endif
	}

	for ( iLevel = victim->level ; iLevel < level; iLevel++ )
	{
		if (level < LEVEL_IMMORTAL)
			send_to_char( "You raise a level!!\n\r", victim );
		victim->level += 1;
		advance_level( victim );
	}
	victim->exp   = exp_level( victim, victim->level );
	victim->trust = 0;
	return;
}

void do_immortalize(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: immortalize <char>\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim->level < LEVEL_HERO_MAX )
    {
	send_to_char( "This player is not worthy of immortality yet.\n\r", ch );
	return;
    }

    send_to_char( "Immortalizing a player...\n\r", ch );
    set_char_color( AT_IMMORT, victim );
    act( AT_IMMORT, "$n begins to chant softly... then raises $s arms to the sky...",
	 ch, NULL, NULL, TO_ROOM );
    set_char_color( AT_WHITE, victim );
    send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
    set_char_color( AT_LBLUE, victim );

    do_help(victim, "M_GODLVL1_" );
    set_char_color( AT_WHITE, victim );
    send_to_char( "You awake... all your possessions are gone.\n\r", victim );
    while ( victim->first_carrying )
	extract_obj( victim->first_carrying, TRUE );

    victim->level = LEVEL_IMMORTAL;
    advance_level( victim );

    victim->exp   = exp_level( victim, victim->level );
    victim->trust = 0;
    return;
}



void do_trust(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your own trust.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    victim->trust = level;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_restore(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Restore whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

        if ( !ch->pcdata )
          return;

        if ( get_trust( ch ) < sysdata.level_restore_all )
        {
          if ( IS_NPC( ch ) )
          {
  	    send_to_char( "You can't do that.\n\r", ch );
 	    return;
          }
          else
          {
            /* Check if the player did a restore all within the last 18 hours. */
            if ( secCurrentTime - secLastRestoreAllTime < RESTORE_INTERVAL )
            {
              send_to_char( "Sorry, you can't do a restore all yet.\n\r", ch );
              do_restoretime( ch, "" );
              return;
            }
          }
        }
        secLastRestoreAllTime = secCurrentTime;
        ch->pcdata->secRestoreTime = secCurrentTime;
        save_char_obj( ch );
        send_to_char( "Ok.\n\r", ch);
	for ( vch = first_char; vch; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC( vch ) && !IS_IMMORTAL( vch )
              && !in_arena( vch ) )
	    {
   		vch->hit = vch->max_hit;
		vch->mana = vch->max_mana;
		vch->move = vch->max_move;
		vch->pcdata->condition[COND_BLOODTHIRST] = (10 + vch->level);
		update_pos (vch);
		act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT);
	    }
	}
    }
    else
    {

    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( ch ) < sysdata.level_restore_on_player
      &&  victim != ch
      && !( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) ) )
    {
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    if ( victim->pcdata )
      victim->pcdata->condition[COND_BLOODTHIRST] = (10 + victim->level);
    update_pos( victim );
    if ( ch != victim )
      act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );
    send_to_char( "Ok.\n\r", ch );
    return;
    }
}

void do_restoretime(CHAR_DATA *ch, const char* argument)
{
  time_t time_passed;
  int hour, minute;

  if ( !secLastRestoreAllTime)
     ch_printf( ch, "There has been no restore all since reboot\n\r");
  else
     {
     time_passed = secCurrentTime - secLastRestoreAllTime;
     hour = (int) ( time_passed / 3600 );
     minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
     ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\n\r",
                  hour, minute );
     }

  if ( !ch->pcdata )
    return;

  if ( !ch->pcdata->secRestoreTime )
  {
    send_to_char( "You have never done a restore all.\n\r", ch );
    return;
  }

  time_passed = secCurrentTime - ch->pcdata->secRestoreTime;
  hour = (int) ( time_passed / 3600 );
  minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
  ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\n\r",
                  hour, minute );
  return;
}

void do_freeze(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj( victim );

    return;
}



void do_log(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}


void do_litterbug(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Set litterbug flag on whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_LITTERBUG) )
    {
	REMOVE_BIT(victim->act, PLR_LITTERBUG);
	send_to_char( "You can drop items again.\n\r", victim );
	send_to_char( "LITTERBUG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LITTERBUG);
	send_to_char( "You a strange force prevents you from dropping any more items!\n\r", victim );
	send_to_char( "LITTERBUG set.\n\r", ch );
    }

    return;
}


void do_noemote(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_EMOTE) )
    {
	REMOVE_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NO_EMOTE removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_EMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NO_EMOTE set.\n\r", ch );
    }

    return;
}



void do_notell(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_NO_TELL) )
    {
	REMOVE_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NO_TELL removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_NO_TELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NO_TELL set.\n\r", ch );
    }

    return;
}


void do_notitle(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notitle whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->pcdata->flags, PCFLAG_NOTITLE) )
    {
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
        send_to_char( "You can set your own title again.\n\r", victim );
        send_to_char( "NOTITLE removed.\n\r", ch );
    }
    else
	{
		SET_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
		sprintf( buf, "the %s",
				title_table [victim->Class] [victim->level]
				[victim->sex == SEX_FEMALE ? 1 : 0] );
		set_title( victim, buf );
		send_to_char( "You can't set your own title!\n\r", victim );
		send_to_char( "NOTITLE set.\n\r", ch );
	}

    return;
}

void do_silence(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Silence whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

	/* Ksilyan:
	 * Changed from >= to > so that mags can silence 50s.
	 */
    if ( get_trust( victim ) > get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	send_to_char( "Player already silenced, use unsilence to remove.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can't use channels!\n\r", victim );
	send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}

void do_unsilence(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unsilence whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
	REMOVE_BIT(victim->act, PLR_SILENCE);
	send_to_char( "You can use channels again.\n\r", victim );
	send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
	send_to_char( "That player is not silenced.\n\r", ch );
    }

    return;
}




void do_peace(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *rch;

	act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
	for ( rch = ch->GetInRoom()->first_person; rch; rch = rch->next_in_room )
	{
		if ( rch->IsFighting() )
		{
			rch->StopAllFights();
			//stop_fighting( rch, TRUE );
			do_sit( rch, "" );
		}

		/* Added by Narn, Nov 28/95 */
		stop_hating( rch );
		stop_hunting( rch );
		stop_fearing( rch );
	}

	send_to_char( "Ok.\n\r", ch );
	return;
}



BAN_DATA *		first_ban;
BAN_DATA *		last_ban;


void save_banlist( void )
{
  BAN_DATA *pban;
  FILE *fp;

  fclose( fpReserve );
  if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "w" )) )
  {
    bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
    perror(BAN_LIST);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( pban = first_ban; pban; pban = pban->next )
    fprintf( fp, "%s~~%s~\n", pban->site, pban->ban_time );
  fprintf( fp, "End~\n" );
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}



void do_ban(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;
    int bnum;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );

    set_pager_color( AT_PLAIN, ch );
    if ( arg[0] == '\0' )
    {
	send_to_pager( "Banned sites:\n\r", ch );
	send_to_pager( "[ #] Time                     Site\n\r", ch );
	send_to_pager( "---- ------------------------ ---------------\n\r", ch );
	for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
	    pager_printf(ch, "[%2d] %-24s %s\n\r", bnum,
	            pban->ban_time, pban->site);
	return;
    }

    if ( !str_cmp(arg, "help") )
    {
      send_to_char( "Syntax: ban <site address>\n\r", ch );
      return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->site ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    CREATE( pban, BAN_DATA, 1 );
    LINK( pban, first_ban, last_ban, next, prev );
    pban->site	= str_dup( arg );
    sprintf(buf, "%24.24s", ctime(&secCurrentTime));
    pban->ban_time = str_dup( buf );
    save_banlist( );
    send_to_char( "Ban created.  Mortals banned from site.\n\r", ch );
    return;
}


void do_allow(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->site ) )
	{
	    UNLINK( pban, first_ban, last_ban, next, prev );
	    if ( pban->ban_time )
	      DISPOSE(pban->ban_time);
	    DISPOSE( pban->site );
	    DISPOSE( pban );
	    save_banlist( );
	    send_to_char( "Site no longer banned.\n\r", ch );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}

void do_wizlock(CHAR_DATA *ch, const char* argument)
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
	send_to_char( "Game wizlocked.\n\r", ch );
    else
	send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}

void do_newbielock(CHAR_DATA *ch, const char* argument)
{
  extern bool newbielock;
  newbielock = !newbielock;

  ch_printf(ch,"Game now %snewbielocked.\n\r"
              ,(newbielock ? "" : "un-")
           );
}



void do_noresolve(CHAR_DATA *ch, const char* argument)
{
    sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

    if ( sysdata.NO_NAME_RESOLVING )
	send_to_char( "Name resolving disabled.\n\r", ch );
    else
	send_to_char( "Name resolving enabled.\n\r", ch );

    return;
}


void do_users(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	int count;
	char arg[MAX_INPUT_LENGTH];

	one_argument (argument, arg);
	count	= 0;
	buf[0]	= '\0';

	set_pager_color( AT_PLAIN, ch );
	sprintf(buf,
	             "  Vnum  |Desc|Con|Idle| Port | Player       Account             @HostIP                 ");
	strcat(buf, "\n\r");
	strcat(buf,  "--------+----+---+----+------+--------------------------------------------------");
	strcat(buf, "\n\r");
	send_to_pager(buf, ch);

	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if (arg[0] == '\0')
		{
			if ( (d->GetCharacter() && can_who( ch, d->GetCharacter() )) )
			{
				count++;
				sprintf( buf,
					"%8s|%4d|%3d|%4d|%6d| %-12s %-30s @%-16s ",
					vnum_to_dotted(d->GetCharacter() ? d->GetCharacter()->GetInRoom() ? d->GetCharacter()->GetInRoom()->vnum : 0 : 0),
					d->GetDescriptor(),
					d->ConnectedState,
					d->GetIdleSeconds() /* / 4 */,
					d->GetPort(),
					d->GetOriginalCharacter() ? d->GetOriginalCharacter()->getName().c_str() : "(none)",
					d->Account ? d->Account->email : "(none)",
					d->GetHost());
				strcat(buf, "\n\r");
				send_to_pager( buf, ch );
			}
		}
		else
		{
			if ( (d->GetCharacter() && can_see( ch, d->GetCharacter() ))
				&&	 ( !str_prefix( arg, d->GetHost() )
				||	 ( d->GetCharacter() && !str_prefix( arg, d->GetCharacter()->getName().c_str() ) ) ) )
			{
				count++;
				pager_printf( ch,
					"%8s|%4d|%3d|%4d|%6d| %-12s %-18s @%-16s ",
					vnum_to_dotted(d->GetCharacter()
						? (d->GetCharacter()->GetInRoom() ? d->GetCharacter()->GetInRoom()->vnum : 0)
						: 0),
					d->GetDescriptor(),
					d->ConnectedState,
					d->GetIdleSeconds() /*/ 4*/,
					d->GetPort(),
					d->GetOriginalCharacter() ? d->GetOriginalCharacter()->getName().c_str() : "(none)",
					d->Account ? d->Account->email : "(none)",
					d->GetHost()
					);
				buf[0] = '\0';
				strcat(buf, "\n\r");
				send_to_pager( buf, ch );
			}
		}
	}
	pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
	return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	bool mobsonly;
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Force whom to do what?\n\r", ch );
		return;
	}

	if ( !str_cmp( argument, "save" ) )
		mobsonly = get_trust( ch ) < sysdata.level_forcepc - 2; /* little fixie to allow me to save :P -Ksilyan*/
	else
		mobsonly = get_trust( ch ) < sysdata.level_forcepc;

	if ( !str_cmp( arg, "all" ) )
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		if ( mobsonly )
		{
			send_to_char( "Force whom to do what?\n\r", ch );
			return;
		}

		for ( vch = first_char; vch; vch = vch_next )
		{
			vch_next = vch->next;

			if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
			{
				act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
				interpret( vch, argument );
			}
		}
	}
	else
	{
		CHAR_DATA *victim;

		victim = get_char_room(ch, arg);

		if (victim == NULL)
			victim = get_char_world(ch, arg);


		if ( victim == NULL )
		{
			send_to_char( "They aren't here.\n\r", ch );
			return;
		}

		if ( victim == ch )
		{
			send_to_char( "Aye aye, right away!\n\r", ch );
			return;
		}

		if ( ( get_trust( victim ) >= get_trust( ch ) )
				|| ( mobsonly && !IS_NPC( victim ) ) )
		{
			send_to_char( "Do it yourself!\n\r", ch );
			return;
		}

		act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
		interpret( victim, argument );
	}

	send_to_char( "Ok.\n\r", ch );
	return;
}


void do_invis(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	sh_int level;

	/*
	if ( IS_NPC(ch))
	return;
	*/

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
		if ( !is_number( arg ) )
		{
			send_to_char( "Usage: invis | invis <level>\n\r", ch );
			return;
		}
		level = atoi( arg );
		if ( level < 1 || level > get_trust( ch ) )
		{
			send_to_char( "Invalid level.\n\r", ch );
			return;
		}

		if (!IS_NPC(ch))
		{
			ch->pcdata->wizinvis = level;
			ch_printf( ch, "Wizinvis level set to %d.\n\r", level );
		}

		if (IS_NPC(ch))
		{
			ch->mobinvis = level;
			ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
		}
		return;
	}

	if (!IS_NPC(ch))
	{
		if ( ch->pcdata->wizinvis < 2 )
			ch->pcdata->wizinvis = ch->level;
	}

	if (IS_NPC(ch))
	{
		if ( ch->mobinvis < 2 )
			ch->mobinvis = ch->level;
	}


	if ( IS_SET(ch->act, PLR_WIZINVIS) )
	{
		act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly fade back into existence.\n\r", ch );
		REMOVE_BIT(ch->act, PLR_WIZINVIS);
	}
	else
	{
		act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly vanish into thin air.\n\r", ch );
		SET_BIT(ch->act, PLR_WIZINVIS);
	}


	return;
}


void do_holylight(CHAR_DATA *ch, const char* argument)
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_rassign(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  r_lo, r_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    r_lo = dotted_to_vnum(ch->GetInRoom()->vnum, arg2 );
	r_hi = dotted_to_vnum(ch->GetInRoom()->vnum, arg3 );

    if ( arg1[0] == '\0' || r_lo < 0 || r_hi < 0 )
    {
	send_to_char( "Syntax: rassign <who> <low> <high>\n\r", ch );
	return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
	send_to_char( "They don't seem to be around.\n\r", ch );
	return;
    }
    if ( IS_NPC( victim ) /* Testaur allows this:  ||  get_trust( victim ) < LEVEL_ENGINEER */ )
    {
	send_to_char( "They wouldn't know what to do with a room range.\n\r", ch );
	return;
    }
    if ( r_lo > r_hi )
    {
	send_to_char( "Unacceptable room range.\n\r", ch );
	return;
    }
    if ( r_lo == 0 )
       r_hi = 0;
    victim->pcdata->r_range_lo = r_lo;
    victim->pcdata->r_range_hi = r_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the room vnum range %d - %d.\n\r",
		ch->getName().c_str(), r_lo, r_hi );
    assign_area( victim );	/* Put back by Thoric on 02/07/96 */
    if ( !victim->pcdata->area )
    {
	bug( "rassign: assign_area failed", 0 );
    	return;
    }

    if (r_lo == 0)				/* Scryn 8/12/95 */
    {
	REMOVE_BIT ( victim->pcdata->area->status, AREA_LOADED );
	SET_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    else
    {
        SET_BIT( victim->pcdata->area->status, AREA_LOADED );
	REMOVE_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    return;
}

void do_oassign(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  o_lo, o_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    o_lo = dotted_to_vnum(ch->GetInRoom()->vnum, arg2 );
	o_hi = dotted_to_vnum(ch->GetInRoom()->vnum, arg3 );

    if ( arg1[0] == '\0' || o_lo < 0 || o_hi < 0 )
    {
	send_to_char( "Syntax: oassign <who> <low> <high>\n\r", ch );
	return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
	send_to_char( "They don't seem to be around.\n\r", ch );
	return;
    }
    if ( IS_NPC( victim ) /* || get_trust( victim ) < LEVEL_TINKERER */ )
    {
	send_to_char( "They wouldn't know what to do with an object range.\n\r", ch );
	return;
    }
    if ( o_lo > o_hi )
    {
	send_to_char( "Unacceptable object range.\n\r", ch );
	return;
    }
    victim->pcdata->o_range_lo = o_lo;
    victim->pcdata->o_range_hi = o_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the object vnum range %d - %d.\n\r",
		ch->getName().c_str(), o_lo, o_hi );
    return;
}

void do_massign(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  m_lo, m_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    m_lo = dotted_to_vnum(ch->GetInRoom()->vnum, arg2 );
	m_hi = dotted_to_vnum(ch->GetInRoom()->vnum, arg3 );

    if ( arg1[0] == '\0' || m_lo < 0 || m_hi < 0 )
    {
	send_to_char( "Syntax: massign <who> <low> <high>\n\r", ch );
	return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
	send_to_char( "They don't seem to be around.\n\r", ch );
	return;
    }
    if ( IS_NPC( victim ) /* || get_trust( victim ) < LEVEL_CREATOR */ )
    {
	send_to_char( "They wouldn't know what to do with a monster range.\n\r", ch );
	return;
    }
    if ( m_lo > m_hi )
    {
	send_to_char( "Unacceptable monster range.\n\r", ch );
	return;
    }
    victim->pcdata->m_range_lo = m_lo;
    victim->pcdata->m_range_hi = m_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the monster vnum range %d - %d.\n\r",
		ch->getName().c_str(), m_lo, m_hi );
    return;
}

void do_cmdtable(CHAR_DATA *ch, const char* argument)
{
    int hash, cnt;
    CMDTYPE *cmd;

    set_pager_color( AT_PLAIN, ch );
    send_to_pager("Commands and Number of Uses This Run\n\r", ch);

    for ( cnt = hash = 0; hash < 126; hash++ )
	for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	{
	    if ((++cnt)%4)
		pager_printf(ch,"%-6.6s %4d\t",cmd->name,cmd->userec.num_uses);
	    else
		pager_printf(ch,"%-6.6s %4d\n\r", cmd->name,cmd->userec.num_uses );
	}
    return;
}

/*
 * Load up a player file
 */
void do_loadup(CHAR_DATA *ch, const char* argument)
{
	char fname[1024];
	char name[256];
	struct stat fst;
	bool loaded;
	int old_room_vnum;
	char buf[MAX_STRING_LENGTH];

	one_argument( argument, name );
	if ( name[0] == '\0' )
	{
		send_to_char( "Usage: loadup <playername>\n\r", ch );
		return;
	}

	name[0] = UPPER(name[0]);

	sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
		capitalize( name ) );
	if ( stat( fname, &fst ) != -1 )
	{
		PlayerConnection * d = new PlayerConnection(0);

		loaded = load_char_obj( d, name, FALSE );
		add_char( d->GetCharacter() );
		old_room_vnum = d->GetCharacter()->GetInRoom()->vnum;
		char_to_room( d->GetCharacter(), ch->GetInRoom() );
		if ( get_trust(d->GetCharacter()) >= get_trust( ch ) )
		{
			do_say( d->GetCharacter(), "Do *NOT* disturb me again!" );
			send_to_char( "I think you'd better leave that player alone!\n\r", ch );
			d->GetCharacter()->SetConnection(NULL);
			do_quit( d->GetCharacter(), "" );
			return;
		}
		d->GetCharacter()->SetConnection(NULL);
		d->GetCharacter()->retran	= old_room_vnum;
		d->CurrentCharId		= 0;
		d->Account			= NULL;

		delete d;
		ch_printf(ch, "Player %s loaded from room %d.\n\r", capitalize( name ),old_room_vnum );
		sprintf(buf, "%s appears from nowhere, eyes glazed over.\n\r", capitalize( name ) );
		act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	/* else no player file */
	send_to_char( "No such player.\n\r", ch );
	return;
}

void do_fixchar(CHAR_DATA *ch, const char* argument)
{
    char name[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, name );
    if ( name[0] == '\0' )
    {
	send_to_char( "Usage: fixchar <playername>\n\r", ch );
	return;
    }
    victim = get_char_room( ch, name );
    if ( !victim )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    fix_char( victim );

    send_to_char( "Done.\n\r", ch );
}

void do_fixme(Character * ch, const char* argument)
{
	void new_fix_char(Character * doer, Character * victim);

	new_fix_char(ch, ch);
}

void do_newbieset(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument (argument, arg2);

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: newbieset <char>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( victim->level < 1 ) || ( victim->level > 5 ) )
    {
     send_to_char( "Level of victim must be 1 to 5.\n\r", ch );
	return;
    }
     obj = create_object( get_obj_index(OBJ_VNUM_NEWBIE_SHIELD), 1 );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_NEWBIE_DAGGER), 1 );
     obj_to_char(obj, victim);

     obj = create_object( get_obj_index(OBJ_VNUM_NEWBIE_BOOTS), 1 );
     obj_to_char(obj, victim);

   obj = create_object( get_obj_index(OBJ_VNUM_NEWBIE_GLOVES), 1 );
     obj_to_char(obj, victim);



    act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT);
    ch_printf( ch, "You have re-equipped %s.\n\r", victim->getName().c_str() );
    return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names (const char *inp, char *out)
{
	char buf[MAX_INPUT_LENGTH], *pbuf=buf;
	int  len;

	*out='\0';
	while (inp && *inp)
	{
		inp = one_argument(inp, buf);
		if ( (len=strlen(buf)) >= 5 && !strcmp(".are", pbuf+len-4) )
		{
			if (*out) strcat (out, " ");
			strcat (out, buf);
		}
	}
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names (const char *inp, char *out)
{
	char buf[MAX_INPUT_LENGTH], *pbuf=buf;
	int  len;

	*out='\0';
	while (inp && *inp)
	{
		inp = one_argument(inp, buf);
		if ( (len=strlen(buf)) < 5 || strcmp(".are", pbuf+len-4) )
		{
			if (*out) strcat (out, " ");
			strcat (out, buf);
		}
	}
}

/*
 * Allows members of the Area Council to add Area names to the bestow field.
 * Area names mus end with ".are" so that no commands can be bestowed.
 */
void do_bestowarea(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int  arg_len;

	argument = one_argument( argument, arg );

	if ( ch->pcdata->councilName_ != "Area Council"
			&&   get_trust (ch) < LEVEL_STONE_ADEPT )
	{
		send_to_char( "Sorry. You are not on the Area Council.\n\r", ch );
		return;
	}

	if ( !*arg )
	{
		send_to_char(
				"Syntax:\n\r"
				"bestowarea <victim> <filename>.are\n\r"
				"bestowarea <victim> none             removes bestowed areas\n\r"
				"bestowarea <victim> list             lists bestowed areas\n\r"
				"bestowarea <victim>                  lists bestowed areas\n\r", ch);
		return;
	}

	if ( !(victim = get_char_world( ch, arg )) )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_NPC( victim ) )
	{
		send_to_char( "You can't give special abilities to a mob!\n\r", ch );
		return;
	}

	if ( get_trust(victim) < LEVEL_IMMORTAL )
	{
		send_to_char( "They aren't an immortal.\n\r", ch );
		return;
	}

	if ( !*argument || !str_cmp (argument, "list") )
	{
		extract_area_names (victim->pcdata->bestowments_.c_str(), buf);
		ch_printf( ch, "Bestowed areas: %s\n\r", buf);
		return;
	}

	if ( !str_cmp (argument, "none") )
	{
		remove_area_names (victim->pcdata->bestowments_.c_str(), buf);
		victim->pcdata->bestowments_ = buf;
		send_to_char( "Done.\n\r", ch);
		return;
	}

	arg_len = strlen(argument);
	if ( arg_len < 5
			|| argument[arg_len-4] != '.' || argument[arg_len-3] != 'a'
			|| argument[arg_len-2] != 'r' || argument[arg_len-1] != 'e' )
	{
		send_to_char( "You can only bestow an area name\n\r", ch );
		send_to_char( "E.G. bestow joe sam.are\n\r", ch );
		return;
	}

	sprintf( buf, "%s %s", victim->pcdata->bestowments_.c_str(), argument );
	victim->pcdata->bestowments_ = buf;
	ch_printf( victim, "%s has bestowed on you the area: %s\n\r",
			ch->getName().c_str(), argument );
	send_to_char( "Done.\n\r", ch );
}

void do_bestow(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Bestow whom with what?\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( IS_NPC( victim ) )
	{
		send_to_char( "You can't give special abilities to a mob!\n\r", ch );
		return;
	}

	if ( get_trust( victim ) > get_trust( ch ) )
	{
		send_to_char( "You aren't powerful enough...\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' || !str_cmp( argument, "list" ) )
	{
		ch_printf( ch, "Current bestowed commands on %s: %s.\n\r",
				victim->getName().c_str(), victim->pcdata->bestowments_.c_str() );
		return;
	}

	if ( !str_cmp( argument, "none" ) )
	{
		victim->pcdata->bestowments_ = "";
		ch_printf( ch, "Bestowments removed from %s.\n\r", victim->getName().c_str() );
		ch_printf( victim, "%s has removed your bestowed commands.\n\r", ch->getName().c_str() );
		return;
	}

	sprintf( buf, "%s %s", victim->pcdata->bestowments_.c_str(), argument );
	victim->pcdata->bestowments_ = buf;
	ch_printf( victim, "%s has bestowed on you the command(s): %s\n\r",
			ch->getName().c_str(), argument );
	send_to_char( "Done.\n\r", ch );
}

struct tm *update_time ( struct tm *old_time )
{
   time_t time;

   time = mktime(old_time);
   return localtime(&time);
}

void do_set_boot_time(CHAR_DATA *ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   bool check;

   check = FALSE;

   argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch);
	send_to_char( "        setboot manual {0/1}\n\r", ch);
	send_to_char( "        setboot default\n\r", ch);
        ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\n\r"
	,reboot_time, set_boot_time->manual );
	return;
    }

    if ( !str_cmp(arg, "time") )
    {
      struct tm *now_time;

      argument = one_argument(argument, arg);
      argument = one_argument(argument, arg1);
      if ( !*arg || !*arg1 || !is_number(arg) || !is_number(arg1) )
      {
	send_to_char("You must input a value for hour and minute.\n\r", ch);
 	return;
      }
      now_time = localtime(&secCurrentTime);

      if ( (now_time->tm_hour = atoi(arg)) < 0 || now_time->tm_hour > 23 )
      {
        send_to_char("Valid range for hour is 0 to 23.\n\r", ch);
        return;
      }

      if ( (now_time->tm_min = atoi(arg1)) < 0 || now_time->tm_min > 59 )
      {
        send_to_char("Valid range for minute is 0 to 59.\n\r", ch);
        return;
      }

      argument = one_argument(argument, arg);
      if ( *arg != '\0' && is_number(arg) )
      {
        if ( (now_time->tm_mday = atoi(arg)) < 1 || now_time->tm_mday > 31 )
        {
	  send_to_char("Valid range for day is 1 to 31.\n\r", ch);
	  return;
        }
        argument = one_argument(argument, arg);
        if ( *arg != '\0' && is_number(arg) )
        {
          if ( (now_time->tm_mon = atoi(arg)) < 1 || now_time->tm_mon > 12 )
          {
            send_to_char( "Valid range for month is 1 to 12.\n\r", ch );
            return;
          }
          now_time->tm_mon--;
          argument = one_argument(argument, arg);
          if ( (now_time->tm_year = atoi(arg)-1900) < 0 ||
                now_time->tm_year > 199 )
          {
            send_to_char( "Valid range for year is 1900 to 2099.\n\r", ch );
            return;
          }
        }
      }
      now_time->tm_sec = 0;
      if ( mktime(now_time) < secCurrentTime )
      {
        send_to_char( "You can't set a time previous to today!\n\r", ch );
        return;
      }
      if (set_boot_time->manual == 0)
 	set_boot_time->manual = 1;
      new_boot_time = update_time(now_time);
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check(mktime(new_boot_time));
      get_reboot_string();

      ch_printf(ch, "Boot time set to %s\n\r", reboot_time);
      check = TRUE;
    }
    else if ( !str_cmp(arg, "manual") )
    {
      argument = one_argument(argument, arg1);
      if (arg1[0] == '\0')
      {
	send_to_char("Please enter a value for manual boot on/off\n\r", ch);
	return;
      }

      if ( !is_number(arg1))
      {
	send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	return;
      }

      if (atoi(arg1) < 0 || atoi(arg1) > 1)
      {
	send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	return;
      }

      set_boot_time->manual = atoi(arg1);
      ch_printf(ch, "Manual bit set to %s\n\r", arg1);
      check = TRUE;
      get_reboot_string();
      return;
    }

    else if (!str_cmp( arg, "default" ))
    {
      set_boot_time->manual = 0;
    /* Reinitialize new_boot_time */
      new_boot_time = localtime(&secCurrentTime);
      new_boot_time->tm_mday += 1;
      if (new_boot_time->tm_hour > 12)
      new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time(new_boot_time);

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char("Reboot time set back to normal.\n\r", ch);
      check = TRUE;
    }

    if (!check)
    {
      send_to_char("Invalid argument for setboot.\n\r", ch);
      return;
    }

    else
    {
      get_reboot_string();
      secNewBoot_time_t = mktime(new_boot_time);
    }
}
/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to
 * pfiles and the correct password
 */
void do_form_password(CHAR_DATA *ch, const char* argument)
{
   char arg[MAX_STRING_LENGTH];

   argument = one_argument(argument, arg);

   ch_printf(ch, "Those two arguments encrypted would result in: %s",
   crypt(arg, argument));
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro(CHAR_DATA *ch, const char* argument)
{
  set_char_color( AT_RED, ch );
  send_to_char("If you want to destroy a character, spell it out!\n\r",ch);
  return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA *pArea )
{
	extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
	extern OBJ_INDEX_DATA   *obj_index_hash[MAX_KEY_HASH];
	extern MOB_INDEX_DATA   *mob_index_hash[MAX_KEY_HASH];
	CHAR_DATA *ech;
	CHAR_DATA *ech_next;
	OBJ_DATA *eobj;
	OBJ_DATA *eobj_next;
	int icnt;
	ROOM_INDEX_DATA *rid;
	ROOM_INDEX_DATA *rid_next;
	OBJ_INDEX_DATA *oid;
	OBJ_INDEX_DATA *oid_next;
	MOB_INDEX_DATA *mid;
	MOB_INDEX_DATA *mid_next;
	RESET_DATA *ereset;
	RESET_DATA *ereset_next;
	ExtraDescData *eed;
	ExtraDescData *eed_next;
	ExitData *exit;
	ExitData *exit_next;
	MPROG_ACT_LIST *mpact;
	MPROG_ACT_LIST *mpact_next;
	MPROG_DATA *mprog;
	TRAIN_DATA * train;
	TRAIN_LIST_DATA* list;
	TRAIN_LIST_DATA* list_next;
	MPROG_DATA *mprog_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for ( ech = first_char; ech; ech = ech_next )
	{
		ech_next = ech->next;

		if ( ech->IsFighting() )
			ech->StopAllFights(); //stop_fighting( ech, TRUE );
		if ( IS_NPC(ech) )
		{
			/* if mob is in area, or part of area. */
			if ( URANGE(pArea->low_m_vnum, ech->pIndexData->vnum,
						pArea->hi_m_vnum) == ech->pIndexData->vnum ||
					(ech->GetInRoom() && ech->GetInRoom()->area == pArea) )
				extract_char( ech, TRUE );
			continue;
		}
		if ( ech->GetInRoom() && ech->GetInRoom()->area == pArea )
			do_recall( ech, "" );
	}
	for ( eobj = first_object; eobj; eobj = eobj_next )
	{
		eobj_next = eobj->next;
		/* if obj is in area, or part of area. */
		if ( URANGE(pArea->low_o_vnum, eobj->pIndexData->vnum,
					pArea->hi_o_vnum) == eobj->pIndexData->vnum ||
				(eobj->GetInRoom() && eobj->GetInRoom()->area == pArea) )
			extract_obj( eobj, TRUE );
	}
	for ( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
	{
		for ( rid = room_index_hash[icnt]; rid; rid = rid_next )
		{
			rid_next = rid->next;

			for ( exit = rid->first_exit; exit; exit = exit_next )
			{
				exit_next = exit->next;
				if ( rid->area == pArea || exit->to_room->area == pArea )
				{
					UNLINK( exit, rid->first_exit, rid->last_exit, next, prev );
					delete exit;
				}
			}
			if ( rid->area != pArea )
				continue;
			if ( rid->first_person )
			{
				bug( "close_area: room with people #%s", vnum_to_dotted(rid->vnum) );
				for ( ech = rid->first_person; ech; ech = ech_next )
				{
					ech_next = ech->next_in_room;
					if ( ech->IsFighting() )
						ech->StopAllFights(); //stop_fighting( ech, TRUE );
					if ( IS_NPC(ech) )
						extract_char( ech, TRUE );
					else
						do_recall( ech, "" );
				}
			}
			if ( rid->first_content )
			{
				bug( "close_area: room with contents #%s", vnum_to_dotted(rid->vnum) );
				for ( eobj = rid->first_content; eobj; eobj = eobj_next )
				{
					eobj_next = eobj->next_content;
					extract_obj( eobj, TRUE );
				}
			}
			for ( eed = rid->first_extradesc; eed; eed = eed_next )
			{
				eed_next = eed->next;
				delete eed;
			}
			for ( mpact = rid->mpact; mpact; mpact = mpact_next )
			{
				mpact_next = mpact->next;
				STRFREE( mpact->buf );
				DISPOSE( mpact );
			}
			for ( mprog = rid->mudprogs; mprog; mprog = mprog_next )
			{
				mprog_next = mprog->next;
				STRFREE( mprog->arglist );
				STRFREE( mprog->comlist );
				DISPOSE( mprog );
			}
			if ( rid == room_index_hash[icnt] )
				room_index_hash[icnt] = rid->next;
			else
			{
				ROOM_INDEX_DATA *trid;

				for ( trid = room_index_hash[icnt]; trid; trid = trid->next )
					if ( trid->next == rid )
						break;
				if ( !trid )
					bug( "Close_area: rid not in hash list %s", vnum_to_dotted(rid->vnum) );
				else
					trid->next = rid->next;
			}
			delete rid;
		}

		for ( mid = mob_index_hash[icnt]; mid; mid = mid_next )
		{
			mid_next = mid->next;

			if ( mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum )
				continue;

			if ( mid->pShop )
			{
				UNLINK( mid->pShop, first_shop, last_shop, next, prev );
				DISPOSE( mid->pShop );
			}
			if ( mid->rShop )
			{
				UNLINK( mid->rShop, first_repair, last_repair, next, prev );
				DISPOSE( mid->rShop );
			}


			if ( mid->train )
			{
				train = mid->train;

				if (train->first_in_list)
				{
					for ( list = train->first_in_list; list; list = list_next )
					{
						list_next = list->next;
						UNLINK( list, train->first_in_list, train->last_in_list, next, prev );
						DISPOSE(list);
					}
				}
				UNLINK( train, first_train, last_train, next, prev );
				DISPOSE( train );
			}


			for ( mprog = mid->mudprogs; mprog; mprog = mprog_next )
			{
				mprog_next = mprog->next;
				STRFREE(mprog->arglist);
				STRFREE(mprog->comlist);
				DISPOSE(mprog);
			}
			if ( mid == mob_index_hash[icnt] )
				mob_index_hash[icnt] = mid->next;
			else
			{
				MOB_INDEX_DATA *tmid;

				for ( tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next )
					if ( tmid->next == mid )
						break;
				if ( !tmid )
					bug( "Close_area: mid not in hash list %s", vnum_to_dotted(mid->vnum) );
				else
					tmid->next = mid->next;
			}
			DISPOSE(mid);
		}

		for ( oid = obj_index_hash[icnt]; oid; oid = oid_next )
		{
			oid_next = oid->next;

			if ( oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum )
				continue;


			for ( eed = oid->first_extradesc; eed; eed = eed_next )
			{
				eed_next = eed->next;
				delete eed;
			}
			for ( paf = oid->first_affect; paf; paf = paf_next )
			{
				paf_next = paf->next;
				DISPOSE(paf);
			}
			for ( mprog = oid->mudprogs; mprog; mprog = mprog_next )
			{
				mprog_next = mprog->next;
				STRFREE(mprog->arglist);
				STRFREE(mprog->comlist);
				DISPOSE(mprog);
			}
			if ( oid == obj_index_hash[icnt] )
				obj_index_hash[icnt] = oid->next;
			else
			{
				OBJ_INDEX_DATA *toid;

				for ( toid = obj_index_hash[icnt]; toid; toid = toid->next )
					if ( toid->next == oid )
						break;
				if ( !toid )
					bug( "Close_area: oid not in hash list %s", vnum_to_dotted(oid->vnum) );
				else
					toid->next = oid->next;
			}
			delete oid;
		}
	}
	for ( ereset = pArea->first_reset; ereset; ereset = ereset_next )
	{
		ereset_next = ereset->next;
		DISPOSE(ereset);
	}
	DISPOSE(pArea->name);
	DISPOSE(pArea->filename);
	STRFREE(pArea->author);
	UNLINK( pArea, first_build, last_build, next, prev );
	UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
	delete pArea;
}

void do_destroy(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Destroy what player file?\n\r", ch );
		return;
	}

	for ( victim = first_char; victim; victim = victim->next )
	{
		if ( !IS_NPC(victim) && victim->getName().ciEqual(arg) )
			break;
		if ( !victim )
		{
			itorSocketId itor;

			/* Make sure they aren't halfway logged in. */
			for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
			{
				// we know that the world's player connection list only holds player connections IDs,
				// so we can safely cast it to PlayerConnection*
				PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

				if ( (victim = d->GetCharacter()) && !IS_NPC(victim) && victim->getName().ciEqual(arg) )
				{
					// remove immediately.
					gConnectionManager->RemoveSocket(d, true);
					break;
				}
			}
		}
		else
		{
			int x, y;

			quitting_char = victim;
			save_char_obj( victim );
			saving_char = NULL;
			extract_char( victim, TRUE );
			for ( x = 0; x < MAX_WEAR; x++ )
				for ( y = 0; y < MAX_LAYERS; y++ )
					save_equipment[x][y] = NULL;
		}

		sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
			capitalize( arg ) );
		sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
			capitalize( arg ) );
		if ( !rename( buf, buf2 ) )
		{
			AREA_DATA *pArea;

			set_char_color( AT_RED, ch );
			send_to_char( "Player destroyed.  Pfile saved in backup directory.\n\r", ch );
			sprintf( buf, "%s%s", GOD_DIR, capitalize(arg) );
			if ( !remove( buf ) )
				send_to_char( "Player's immortal data destroyed.\n\r", ch );
			else if ( errno != ENOENT )
			{
				ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Ydnat.\n\r",
					errno, strerror( errno ) );
				sprintf( buf2, "%s destroying %s", ch->getName().c_str(), buf );
				perror( buf2 );
			}

			sprintf( buf2, "%s.are", capitalize(arg) );
			for ( pArea = first_build; pArea; pArea = pArea->next )
				if ( !strcmp( pArea->filename, buf2 ) )
				{
					sprintf( buf, "%s%s", BUILD_DIR, buf2 );
					if ( IS_SET( pArea->status, AREA_LOADED ) )
						fold_area( pArea, buf, FALSE );
					close_area( pArea );
					sprintf( buf2, "%s.bak", buf );
					set_char_color( AT_RED, ch ); /* Log message changes colors */
					if ( !rename( buf, buf2 ) )
						send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch );
					else if ( errno != ENOENT )
					{
						ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Ydnat.\n\r",
							errno, strerror( errno ) );
						sprintf( buf2, "%s destroying %s", ch->getName().c_str(), buf );
						perror( buf2 );
					}
				}
		}
		else if ( errno == ENOENT )
		{
			set_char_color( AT_PLAIN, ch );
			send_to_char( "Player does not exist.\n\r", ch );
		}
		else
		{
			set_char_color( AT_WHITE, ch );
			ch_printf( ch, "Unknown error #%d - %s.  Report to Ydnat.\n\r",
				errno, strerror( errno ) );
			sprintf( buf, "%s destroying %s", ch->getName().c_str(), arg );
			perror( buf );
		}
	}
	return;
}
extern ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH]; /* db.c */


/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example:

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char * name_expand (CHAR_DATA *ch)
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch))
		return ch->getName().c_str();

	one_argument (ch->getName().c_str(), name); /* copy the first word into name */

	if (!name[0]) /* weird mob .. no keywords */
	{
		strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}

	/* ->people changed to ->first_person -- TRI */
	for (rch = ch->GetInRoom()->first_person; rch && (rch != ch);rch =
	    rch->next_in_room)
		if (is_name (name, rch->getName().c_str()))
			count++;


	sprintf (outbuf, "%d.%s", count, name);
	return outbuf;
}


void do_for(CHAR_DATA *ch, const char* argument)
{
	char range[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
	int i;

	argument = one_argument (argument, range);

	if (!range[0] || !argument[0]) /* invalid usage? */
	{
		do_help (ch, "for");
		return;
	}

	if (!str_prefix("quit", argument))
	{
		send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
		return;
	}


	if (!str_cmp (range, "all"))
	{
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_cmp (range, "gods"))
		fGods = TRUE;
	else if (!str_cmp (range, "mortals"))
		fMortals = TRUE;
	else if (!str_cmp (range, "mobs"))
		fMobs = TRUE;
	else if (!str_cmp (range, "everywhere"))
		fEverywhere = TRUE;
	else
		do_help (ch, "for"); /* show syntax */

	/* do not allow # to make it easier */
	if (fEverywhere && strchr (argument, '#'))
	{
		send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
		return;
	}

	if (strchr (argument, '#')) /* replace # ? */
	{
		/* char_list - last_char, p_next - gch_prev -- TRI */
		for (p = last_char; p ; p = p_prev )
		{
			p_prev = p->prev;  /* TRI */
		/*	p_next = p->next; */ /* In case someone DOES try to AT MOBS SLAY # */
			found = FALSE;

			if (!(p->GetInRoom()) || room_is_private(p->GetInRoom()) || (p == ch))
				continue;

			if (IS_NPC(p) && fMobs)
				found = TRUE;
			else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
				found = TRUE;
			else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
				found = TRUE;

			/* It looks ugly to me.. but it works :) */
			if (found) /* p is 'appropriate' */
			{
				const char *pSource = argument; /* head of buffer to be parsed */
				char *pDest = buf; /* parse into this */

				while (*pSource)
				{
					if (*pSource == '#') /* Replace # with name of target */
					{
						const char *namebuf = name_expand (p);

						if (namebuf) /* in case there is no mob name ?? */
							while (*namebuf) /* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* while */
				*pDest = '\0'; /* Terminate */

				/* Execute */
				old_room = ch->GetInRoom();
				char_from_room (ch);
				char_to_room (ch,p->GetInRoom());
				interpret (ch, buf);
				char_from_room (ch);
				char_to_room (ch,old_room);

			} /* if found */
		} /* for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
		for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
			for (room = room_index_hash[i] ; room ; room = room->next)
			{
				found = FALSE;

				/* Anyone in here at all? */
				if (fEverywhere) /* Everywhere executes always */
					found = TRUE;
				else if (!room->first_person) /* Skip it if room is empty */
					continue;
				/* ->people changed to first_person -- TRI */

				/* Check if there is anyone here of the requried type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				/* ->people to ->first_person -- TRI */
				for (p = room->first_person; p && !found; p = p->next_in_room)
				{

					if (p == ch) /* do not execute on oneself */
						continue;

					if (IS_NPC(p) && fMobs)
						found = TRUE;
					else if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
						found = TRUE;
					else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
						found = TRUE;
				} /* for everyone inside the room */

				if (found && !room_is_private(room)) /* Any of the required type here AND room not private? */
				{
					/* This may be ineffective. Consider moving character out of old_room
					   once at beginning of command then moving back at the end.
					   This however, is more safe?
					*/

					old_room = ch->GetInRoom();
					char_from_room (ch);
					char_to_room (ch, room);
					interpret (ch, argument);
					char_from_room (ch);
					char_to_room (ch, old_room);
				} /* if found */
			} /* for every room in a bucket */
	} /* if strchr */
} /* do_for */

void save_sysdata   ( SYSTEM_DATA sys ) ;

void do_cset(CHAR_DATA *ch, const char* argument)
{
  char arg[MAX_STRING_LENGTH];
  sh_int level;

  set_char_color( AT_IMMORT, ch );

  if (argument[0] == '\0')
  {
    ch_printf(ch, "Mail:\n\r  Read all mail: %d. Read mail for free: %d. Write mail for free: %d.\n\r",
	    sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
    ch_printf(ch, "  Take all mail: %d.\n\r",
	    sysdata.take_others_mail);
    ch_printf(ch, "Channels:\n\r  Muse: %d. Think: %d. Log: %d. Build: %d.\n\r",
 	    sysdata.muse_level, sysdata.think_level, sysdata.log_level,
	    sysdata.build_level);
    ch_printf(ch, "Building:\n\r  Prototype modification: %d.  Player msetting: %d.\n\r",
	    sysdata.level_modify_proto, sysdata.level_mset_player );
    ch_printf(ch, "Modify prototype flag: %d\r\n", sysdata.level_modify_proto_flag);
    ch_printf(ch, "Guilds:\n\r  Overseer: %s.  Advisor: %s.\n\r",
            sysdata.guild_overseer, sysdata.guild_advisor );
    ch_printf(ch, "Other:\n\r  Force on players: %d.  ", sysdata.level_forcepc);
    ch_printf(ch, "Private room override: %d.\n\r", sysdata.level_override_private);
    ch_printf(ch, "  Penalty to regular stun chance: %d.  ", sysdata.stun_regular );
    ch_printf(ch, "Penalty to stun plr vs. plr: %d.\n\r", sysdata.stun_plr_vs_plr );
    ch_printf(ch, "  Percent damage plr vs. plr: %3d.  ", sysdata.dam_plr_vs_plr );
    ch_printf(ch, "Percent damage plr vs. mob: %d.\n\r", sysdata.dam_plr_vs_mob );
    ch_printf(ch, "  Percent damage mob vs. plr: %3d.  ", sysdata.dam_mob_vs_plr );
    ch_printf(ch, "Percent damage mob vs. mob: %d.\n\r", sysdata.dam_mob_vs_mob );
    ch_printf(ch, "  Get object without take flag: %d.  ", sysdata.level_getobjnotake);
    ch_printf(ch, "Autosave frequency (minutes): %d.\n\r", sysdata.save_frequency );
    ch_printf(ch, "  Save flags: %s\n\r", flag_string( sysdata.save_flags, save_flag ) );
    ch_printf(ch, "Level_interrupt_buffer: %d\r\n", sysdata.level_interrupt_buffer);
    ch_printf(ch, "Level_Monitor: %d\r\n", sysdata.level_monitor);
    ch_printf(ch, "Level_restore_all: %d\r\n", sysdata.level_restore_all);
    ch_printf(ch, "Level_restore_on_player: %d\r\n", sysdata.level_restore_on_player);
    return;
  }

  argument = one_argument( argument, arg );

  if (!str_cmp(arg, "help"))
  {
     do_help(ch, "controls");
     return;
  }

  if (!str_cmp(arg, "save"))
  {
     save_sysdata(sysdata);
     return;
  }

  if (!str_cmp(arg, "saveflag"))
  {
	int x = get_saveflag( argument );

	if ( x == -1 )
	    send_to_char( "Not a save flag.\n\r", ch );
	else
	{
	    TOGGLE_BIT( sysdata.save_flags, 1 << x );
	    send_to_char( "Ok.\n\r", ch );
	}
	return;
  }

  if (!str_prefix( arg, "guild_overseer" ) )
  {
    STRFREE( sysdata.guild_overseer );
    sysdata.guild_overseer = str_dup( argument );
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_prefix( arg, "guild_advisor" ) )
  {
    STRFREE( sysdata.guild_advisor );
    sysdata.guild_advisor = str_dup( argument );
    send_to_char("Ok.\n\r", ch);
    return;
  }

  level = (sh_int) atoi(argument);

    if ( !str_prefix(arg, "level_interrupt_buffer") ) {
        sysdata.level_interrupt_buffer = level;
        send_to_char("Ok.\r\n", ch);
        return;
    }
    if ( !str_prefix(arg, "level_monitor") ) {
        sysdata.level_monitor = level;
        send_to_char("Ok.\r\n", ch);
        return;
    }
    if ( !str_prefix(arg, "level_restore_all") ) {
        sysdata.level_restore_all = level;
        send_to_char("Ok.\r\n", ch);
        return;
    }
    if ( !str_prefix(arg, "level_restore_on_player") ) {
        sysdata.level_restore_on_player = level;
        send_to_char("Ok.\r\n", ch);
        return;
    }

  if (!str_prefix( arg, "savefrequency" ) )
  {
    sysdata.save_frequency = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "stun"))
  {
    sysdata.stun_regular = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "stun_pvp"))
  {
    sysdata.stun_plr_vs_plr = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "dam_pvp"))
  {
    sysdata.dam_plr_vs_plr = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "get_notake"))
  {
    sysdata.level_getobjnotake = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "dam_pvm"))
  {
    sysdata.dam_plr_vs_mob = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "dam_mvp"))
  {
    sysdata.dam_mob_vs_plr = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "dam_mvm"))
  {
    sysdata.dam_mob_vs_mob = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (level < 0 || level > MAX_LEVEL)
  {
    send_to_char("Invalid value for new control.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "read_all"))
  {
    sysdata.read_all_mail = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if (!str_cmp(arg, "read_free"))
  {
    sysdata.read_mail_free = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "write_free"))
  {
    sysdata.write_mail_free = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "take_all"))
  {
    sysdata.take_others_mail = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "muse"))
  {
    sysdata.muse_level = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "think"))
  {
    sysdata.think_level = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "log"))
  {
    sysdata.log_level = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "build"))
  {
    sysdata.build_level = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "protoflag_modify"))
  {
      sysdata.level_modify_proto_flag = level;
      send_to_char("Ok.\r\n", ch);
      return;
  }
  if (!str_cmp(arg, "proto_modify"))
  {
    sysdata.level_modify_proto = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "override_private"))
  {
    sysdata.level_override_private = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "forcepc"))
  {
    sysdata.level_forcepc = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "mset_player"))
  {
    sysdata.level_mset_player = level;
    send_to_char("Ok.\n\r", ch);
    return;
  }
  else
  {
    send_to_char("Invalid argument.\n\r", ch);
    return;
  }
}

void get_reboot_string(void)
{
  sprintf(reboot_time, "%s", asctime(new_boot_time));
}


void do_orange(CHAR_DATA *ch, const char* argument)
{
  send_to_char( "Function under construction.\n\r", ch );
  return;
}

void do_mrange(CHAR_DATA *ch, const char* argument)
{
  send_to_char( "Function under construction.\n\r", ch );
  return;
}

void do_hell(CHAR_DATA *ch, const char* argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  sh_int time;
  bool h_d = FALSE;
  struct tm *tms;

  argument = one_argument(argument, arg);
  if ( !*arg )
  {
    send_to_char( "Hell who, and for how long?\n\r", ch );
    return;
  }
  if ( !(victim = get_char_world(ch, arg)) || IS_NPC(victim) )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  if ( IS_IMMORTAL(victim) )
  {
    send_to_char( "There is no point in helling an immortal.\n\r", ch );
    return;
  }
  if ( victim->pcdata->secReleaseDate!= 0 )
  {
    ch_printf(ch, "They are already in hell until %24.24s, by %s.\n\r",
            ctime(&victim->pcdata->secReleaseDate), victim->pcdata->helledBy_.c_str());
    return;
  }
  argument = one_argument(argument, arg);
  if ( !*arg || !is_number(arg) )
  {
    send_to_char( "Hell them for how long?\n\r", ch );
    return;
  }
  time = atoi(arg);
  if ( time <= 0 )
  {
    send_to_char( "You cannot hell for zero or negative time.\n\r", ch );
    return;
  }
  argument = one_argument(argument, arg);
  if ( !*arg || !str_prefix(arg, "hours") )
    h_d = TRUE;
  else if ( str_prefix(arg, "days") )
  {
    send_to_char( "Is that value in hours or days?\n\r", ch );
    return;
  }
  else if ( time > 30 )
  {
    send_to_char( "You may not hell a person for more than 30 days at a time.\n\r", ch );
    return;
  }
  tms = localtime(&secCurrentTime);
  if ( h_d )
    tms->tm_hour += time;
  else
    tms->tm_mday += time;
  victim->pcdata->secReleaseDate = mktime(tms);
  victim->pcdata->helledBy_ = ch->getName();
  ch_printf(ch, "%s will be released from hell at %24.24s.\n\r", victim->getName().c_str(),
          ctime(&victim->pcdata->secReleaseDate));
  act(AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT);
  char_from_room(victim);
  char_to_room(victim, get_room_index(358));
  act(AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT);
  do_look(victim, "auto");
  ch_printf(victim, "The immortals are not pleased with your actions.\n\r"
          "You shall remain in hell for %d %s%s.\n\r", time,
          (h_d ? "hour" : "day"), (time == 1 ? "" : "s"));
  save_char_obj(victim);	/* used to save ch, fixed by Thoric 09/17/96 */
  return;
}

void do_unhell(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;

	argument = one_argument(argument, arg);
	if ( !*arg )
	{
		send_to_char( "Unhell whom..?\n\r", ch );
		return;
	}
	if( !ch )
		return;
	location = ch->GetInRoom();
	ch->InRoomId = get_room_index(358)->GetId();
	victim = get_char_room(ch, arg);
	if( !victim )
	{
		send_to_char( "That person is not playing...\r\n", ch );
		return;
	}
	ch->InRoomId = location->GetId();			 /* The case of unhell self, etc. */
	if ( !victim || IS_NPC(victim) || victim->GetInRoom()->vnum != 358 )
	{
		send_to_char( "No one like that is in hell.\n\r", ch );
		return;
	}
	location=NULL;
	if ( victim->pcdata->clan )
		location = get_room_index(victim->pcdata->clan->recall);
	if ( victim->pcdata->recall_room )
		location = get_room_index(victim->pcdata->recall_room);

	if(!location)
		location = get_room_index(ROOM_VNUM_TEMPLE);
	if ( !location )
		location = ch->GetInRoom();

	MOBtrigger = FALSE;
	act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
	char_from_room(victim);
	char_to_room(victim, location);
	send_to_char( "The gods have smiled on you and released you from hell early!\n\r", victim );
	do_look(victim, "auto");
	send_to_char( "They have been released.\n\r", ch );

	if ( victim->pcdata->helledBy_.length() > 0 )
	{
		if( ch->getName() != victim->pcdata->helledBy_ )
			ch_printf(ch, "(You should probably write a note to %s, explaining the early release.)\n\r",
			victim->pcdata->helledBy_.c_str());
		victim->pcdata->helledBy_ = "";
	}

	MOBtrigger = FALSE;
	act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
	victim->pcdata->secReleaseDate = 0;
	save_char_obj(victim);
	return;
}

/* Vnum search command by Swordbearer */
void do_vsearch(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    int obj_counter = 1;
    int argi;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  vsearch <vnum>.\n\r", ch );
        return;
    }

    set_pager_color( AT_PLAIN, ch );
    argi=dotted_to_vnum(ch->GetInRoom()->vnum, arg);
    if (argi<=0 && argi>1048576000)
    {
	send_to_char( "Vnum out of range.\n\r", ch);
	return;
    }
    for ( obj = first_object; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ))
	  continue;

	found = TRUE;
	for ( in_obj = obj; in_obj->GetInObj() != NULL;
	  in_obj = in_obj->GetInObj() );

	if ( in_obj->CarriedById != 0 )
	 {
#ifdef USE_OBJECT_LEVELS
	    pager_printf( ch, "[%2d] Level %d %s carried by %s.\n\r",
		obj_counter,
		obj->level, obj_short(obj),
		PERS( in_obj->GetCarriedBy(), ch ) );
#else
	    pager_printf( ch, "[%2d] %s carried by %s.\n\r",
		obj_counter,
		obj_short(obj),
		PERS( in_obj->GetCarriedBy(), ch ) );
#endif
	 }
	else
	 {
	  pager_printf( ch, "[%2d] [%11s] %s in %s.\n\r", obj_counter,
		vnum_to_dotted( ( in_obj->GetInRoom() ) ? in_obj->GetInRoom()->vnum : 0 ),
		obj_short(obj), ( in_obj->GetInRoom() == NULL ) ?
		"somewhere" : in_obj->GetInRoom()->name_.c_str() );
	 }

	obj_counter++;
    }

    if ( !found )
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r" , ch );

    return;
}

/*
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96
 */
void do_sober(CHAR_DATA *ch, const char* argument)
{
  CHAR_DATA *victim;
  char arg1 [MAX_INPUT_LENGTH];

  argument = smash_tilde_static( argument );
  argument = one_argument( argument, arg1 );
  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC( victim ) )
  {
    send_to_char( "Not on mobs.\n\r", ch );
    return;
  }

  if ( victim->pcdata )
    victim->pcdata->condition[COND_DRUNK] = 0;
  send_to_char( "Ok.\n\r", ch );
  send_to_char( "You feel sober again.\n\r", victim );
  return;
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE *command )
{
    if ( command->name )
      DISPOSE( command->name );
    DISPOSE( command );
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE *command )
{
    CMDTYPE *tmp, *tmp_next;
    int hash;

    if ( !command )
    {
	bug( "Unlink_command NULL command", 0 );
	return;
    }

    hash = command->name[0]%126;

    if ( command == (tmp=command_hash[hash]) )
    {
	command_hash[hash] = tmp->next;
	return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
	tmp_next = tmp->next;
	if ( command == tmp_next )
	{
	    tmp->next = tmp_next->next;
	    return;
	}
    }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE *command )
{
    int hash, x;
    CMDTYPE *tmp, *prev;

    if ( !command )
    {
	bug( "Add_command: NULL command", 0 );
	return;
    }

    if ( !command->name )
    {
	bug( "Add_command: NULL command->name", 0 );
	return;
    }

    if ( !command->do_fun )
    {
	bug( "Add_command: NULL command->do_fun", 0 );
	return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; command->name[x] != '\0'; x++ )
	command->name[x] = LOWER(command->name[x]);

    hash = command->name[0] % 126;

    if ( (prev = tmp = command_hash[hash]) == NULL )
    {
	command->next = command_hash[hash];
	command_hash[hash] = command;
	return;
    }

    /* add to the END of the list */
    for ( ; tmp; tmp = tmp->next )
	if ( !tmp->next )
	{
	    tmp->next = command;
	    command->next = NULL;
	}
    return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 */
void do_cedit(CHAR_DATA *ch, const char* argument)
{
	CMDTYPE *command;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = smash_tilde_static ( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	set_char_color( AT_IMMORT, ch );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: cedit save\n\r", ch );
		send_to_char( "Syntax: cedit <command> create [code]\n\r", ch );
		send_to_char( "Syntax: cedit <command> delete\n\r", ch );
		send_to_char( "Syntax: cedit <command> show\n\r", ch );
		send_to_char( "Syntax: cedit <command> [field]\n\r", ch );
		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( "  level position log code\n\r", ch );
		return;
	}

	if ( !str_cmp( arg1, "save" ) )
	{
		save_commands();
		send_to_char( "Saved.\n\r", ch );
		return;
	}

	command = find_command( arg1 );

	if ( !str_cmp( arg2, "create" ) )
	{
		if ( command )
		{
			send_to_char( "That command already exists!\n\r", ch );
			return;
		}
		CREATE( command, CMDTYPE, 1 );
		command->name = str_dup( arg1 );
		command->level = get_trust(ch);
		if ( *argument )
			one_argument(argument, arg2);
		else
			sprintf( arg2, "do_%s", arg1 );
		command->do_fun = skill_function( arg2 );
		add_command( command );
		send_to_char( "Command added.\n\r", ch );
		if ( command->do_fun == skill_notfound )
			ch_printf( ch, "Code %s not found.  Set to no code.\n\r", arg2 );
		return;
	}

	if ( !command )
	{
		send_to_char( "Command not found.\n\r", ch );
		return;
	}
	else if ( command->level > get_trust(ch) )
	{
		send_to_char( "You cannot touch this command.\n\r", ch );
		return;
	}

	if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
	{
		ch_printf( ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rCode:     %s\n\r",
				command->name, command->level, command->position, command->log,
				skill_name(command->do_fun) );
		if ( command->userec.num_uses )
			send_timer(&command->userec, ch);
		return;
	}

	if ( !str_cmp( arg2, "delete" ) )
	{
		unlink_command( command );
		free_command( command );
		send_to_char( "Deleted.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "code" ) )
	{
		DO_FUN *fun = skill_function( argument );

		if ( fun == skill_notfound )
		{
			send_to_char( "Code not found.\n\r", ch );
			return;
		}
		command->do_fun = fun;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "level" ) )
	{
		int level = atoi( argument );

		if ( level < 0 || level > get_trust(ch) )
		{
			send_to_char( "Level out of range.\n\r", ch );
			return;
		}
		command->level = level;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "log" ) )
	{
		int log = atoi( argument );

		if ( log < 0 || log > LOG_COMM )
		{
			send_to_char( "Log out of range.\n\r", ch );
			return;
		}
		command->log = log;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "position" ) )
	{
		int position = atoi( argument );

		if ( position < 0 || position > POS_DRAG )
		{
			send_to_char( "Position out of range.\n\r", ch );
			return;
		}
		command->position = position;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "name" ) )
	{
		bool relocate;

		one_argument( argument, arg1 );
		if ( arg1[0] == '\0' )
		{
			send_to_char( "Cannot clear name field!\n\r", ch );
			return;
		}
		if ( arg1[0] != command->name[0] )
		{
			unlink_command( command );
			relocate = TRUE;
		}
		else
			relocate = FALSE;
		if ( command->name )
			DISPOSE( command->name );
		command->name = str_dup( arg1 );
		if ( relocate )
			add_command( command );
		send_to_char( "Done.\n\r", ch );
		return;
	}

	/* display usage message */
	do_cedit( ch, "" );
}

/*
 * Display class information					-Thoric
 */
void do_showclass(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ClassType *Class;
	int cl, low, hi;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	set_char_color( AT_IMMORT, ch );
	if ( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: showclass <class> [level range]\n\r", ch );
		return;
	}
	if ( is_number(arg1) && (cl=atoi(arg1)) >= 0 && cl < MAX_CLASS )
		Class = class_table[cl];
	else
	{
		Class = NULL;
		for ( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
			if ( class_table[cl]->whoName_.ciEqual(arg1) )
			{
				Class = class_table[cl];
				break;
			}
	}
	if ( !Class )
	{
		send_to_char( "No such class.\n\r", ch );
		return;
	}
	set_pager_color( AT_IMMORT, ch );
	pager_printf( ch, "Class: %s\n\rPrime Attribute: %-14s Weapon: %-5d  Guild:   %d\n\r"
			"Max Skill Adept: %-3d            Thac0:  %-5d  Thac32:  %d\n\r"
			"Hp Min / Hp Max: %d/%-2d           Mana:   %s    ExpBase: %d\n\r",
			Class->whoName_.c_str(), affect_loc_name(Class->attr_prime), Class->weapon, Class->guild,
			Class->skill_adept, Class->thac0_00, Class->thac0_32,
			Class->hp_min, Class->hp_max, Class->fMana ? "yes" : "no ", Class->exp_base );
	if ( arg2[0] != '\0' )
	{
		int x, y, cnt;

		low = UMIN( 0, atoi(arg2) );
		hi  = URANGE( low, atoi(argument), MAX_LEVEL );
		for ( x = low; x <= hi; x++ )
		{
			set_pager_color( AT_LBLUE, ch );
			pager_printf( ch, "Male: %-30s Female: %s\n\r",
					title_table[cl][x][0], title_table[cl][x][1] );
			cnt = 0;
			set_pager_color( AT_BLUE, ch );
			for ( y = gsn_first_spell; y < gsn_top_sn; y++ )
				if ( skill_table[y]->skill_level[cl] == x )
				{
					pager_printf( ch, "  %-7s %-19s%3d     ",
							skill_tname[skill_table[y]->type],
							skill_table[y]->name_.c_str(), skill_table[y]->skill_adept[cl] );
					if ( ++cnt % 2 == 0 )
						send_to_pager( "\n\r", ch );
				}
			if ( cnt % 2 != 0 )
				send_to_pager( "\n\r", ch );
			send_to_pager( "\n\r", ch );
		}
	}
}

/*
 * Edit class information					-Thoric
 */
void do_setclass(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ClassType *Class;
	int cl;

	argument = smash_tilde_static ( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	set_char_color( AT_IMMORT, ch );
	if ( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: setclass <class> <field> <value>\n\r",	ch );
		send_to_char( "\n\rField being one of:\n\r",			ch );
		send_to_char( "  name prime weapon guild thac0 thac32\n\r",	ch );
		send_to_char( "  hpmin hpmax mana expbase mtitle ftitle\n\r", ch );
		return;
	}
	if ( is_number(arg1) && (cl=atoi(arg1)) >= 0 && cl < MAX_CLASS )
		Class = class_table[cl];
	else
	{
		Class = NULL;
		for ( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
			if ( class_table[cl]->whoName_.ciEqual(arg1) )
			{
				Class = class_table[cl];
				break;
			}
	}
	if ( !Class )
	{
		send_to_char( "No such class.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "save" ) )
	{
		write_class_file( cl );
		send_to_char( "Saved.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "name" ) )
	{
		Class->whoName_ = argument;
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "prime" ) )
	{
		int x = get_atype( argument );

		if ( x < APPLY_STR || (x > APPLY_CON && x != APPLY_LCK) )
			send_to_char( "Invalid prime attribute!\n\r", ch );
		else
		{
			Class->attr_prime = x;
			send_to_char( "Done.\n\r", ch );
		}
		return;
	}
	if ( !str_cmp( arg2, "weapon" ) )
	{
		Class->weapon = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "guild" ) )
	{
		Class->guild = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "thac0" ) )
	{
		Class->thac0_00 = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "thac32" ) )
	{
		Class->thac0_32 = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "hpmin" ) )
	{
		Class->hp_min = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "hpmax" ) )
	{
		Class->hp_max = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "mana" ) )
	{
		if ( UPPER(argument[0]) == 'Y' )
			Class->fMana = TRUE;
		else
			Class->fMana = FALSE;
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "expbase" ) )
	{
		Class->exp_base = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}
	if ( !str_cmp( arg2, "mtitle" ) )
	{
		char arg3[MAX_INPUT_LENGTH];
		int x;

		argument = one_argument( argument, arg3 );
		if ( arg3[0] == '\0' || argument[0] == '\0' )
		{
			send_to_char( "Syntax: setclass <class> mtitle <level> <title>\n\r", ch );
			return;
		}
		if ( (x=atoi(arg3)) < 0 || x > MAX_LEVEL )
		{
			send_to_char( "Invalid level.\n\r", ch );
			return;
		}
		DISPOSE( title_table[cl][x][0] );
		title_table[cl][x][0] = str_dup( argument );
		return;
	}
	if ( !str_cmp( arg2, "ftitle" ) )
	{
		char arg3[MAX_INPUT_LENGTH];
		int x;

		argument = one_argument( argument, arg3 );
		if ( arg3[0] == '\0' || argument[0] == '\0' )
		{
			send_to_char( "Syntax: setclass <class> ftitle <level> <title>\n\r", ch );
			return;
		}
		if ( (x=atoi(arg3)) < 0 || x > MAX_LEVEL )
		{
			send_to_char( "Invalid level.\n\r", ch );
			return;
		}
		DISPOSE( title_table[cl][x][1] );
		title_table[cl][x][1] = str_dup( argument );
		return;
	}
	do_setclass( ch, "" );
}


/*
 * Glory command - reworked version of qpset
 *     -Ksilyan
 */
void do_glory(Character * ch, const char* argument)
{
	if ( IS_NPC(ch) )
	{
		ch->sendText( "Can't manipulate glory as an NPC.\n\r" );
		return;
	}

	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	string reason = argument;

	short amount = 0, decimal = 0;
	string amountarg = arg3;
	string::size_type pos = amountarg.find(".");
	amount = atoi( amountarg.substr(0, pos).c_str() ); // get the whole number part
	if ( pos != string::npos )
		decimal = atoi( amountarg.substr(pos + 1, 1).c_str() ); // get the decimal part
	                                 // - only length 1 to limit to a single digit

	if ( arg1[0] == '\0' || arg2[0] == '\0'
			|| float(amount)+float(decimal)/10.0 <= 0.0
			|| reason.length() == 0
	  )
	{
		string give_take = ( get_trust(ch) <= LEVEL_HERO_MAX ? "give" : "give/take" );
		ch->sendText( string("Syntax: glory <character> <") + give_take + "> <amount> <reason>\n\r" );
		ch->sendText( "Amount must be a positive number greater than 0.\n\r" );
		return;
	}

	Character * victim = NULL;
	// Loop through connections looking for victim
	itorSocketId it;
	for ( it = gTheWorld->GetBeginConnection(); it != gTheWorld->GetEndConnection(); it++ )
	{
		PlayerConnection * c = (PlayerConnection*) SocketMap[*it];
		if ( !c->GetOriginalCharacter() || c->ConnectedState < CON_PLAYING )
			continue;

		Character * v = c->GetOriginalCharacter();
		if ( ci_equal_to() ( arg1, v->getName().c_str() ) )
		{
			victim = v;
			break;
		}
	}

	if ( !victim )
	{
		ch->sendText( string(arg1) + " not found. Make sure you type the *full* name.\n\r" );
		return;
	}

	bool give;
	if ( ci_equal_to() ( arg2, "give") )
		give = true;
	else if ( ci_equal_to() ( arg2, "take") )
		give = false;
	else
	{
		do_glory(ch, ""); // give syntax message
		return;
	}

	if ( !give && !IS_IMMORTAL(ch) )
	{
		ch->sendText("Only immortals can take glory.\n\r");
		return;
	}

	if ( give )
	{
		if ( amount > 1 && !IS_IMMORTAL(ch) )
		{
			ch->sendText("Avatars cannot give more than one glory at a time.\n\r");
			return;
		}

		victim->pcdata->quest_curr  += amount;
		victim->pcdata->quest_accum += amount;

		victim->pcdata->quest_deci += decimal;
		if ( victim->pcdata->quest_deci >= 10 )
		{
			victim->pcdata->quest_deci -= 10;
			victim->pcdata->quest_curr++;
			victim->pcdata->quest_accum++;
		}

		ch_printf(victim, "Your glory has been increased by %d.%d.\n\r",
				amount, decimal);

		ostringstream os;
		os << victim->getName().str() << "'s glory has been increased by " << amount
			<< "." << decimal << " (" << ch->getName().str() << ": " << reason << "\n\r";
		log_string ( os.str().c_str() );
	}
	else
	{
		victim->pcdata->quest_deci -= decimal;
		if ( victim->pcdata->quest_deci < 0 )
		{
			victim->pcdata->quest_deci += 10;
			victim->pcdata->quest_curr--;
		}
		victim->pcdata->quest_curr -= amount;

		ch_printf(victim, "Your glory has been decreased by %d.%d.\n\r",
				amount, decimal);

		ostringstream os;
		os << victim->getName().str() << "'s glory has been decreased by " << amount
			<< "." << decimal << " (" << ch->getName().str() << ": " << reason << "\n\r";
		log_string ( os.str().c_str() );
	}
}

void do_stagename(Character * ch, const char* argument)
{
	Character * victim;
	char arg[MAX_INPUT_LENGTH];

	if ( argument[0] == '\0' )
	{
		ch->sendText("Set whose stage name to what?\n\r");
		return;
	}

	argument = one_argument(argument, arg);

	victim = get_char_room( ch, arg );

	if ( !victim )
	{
		ch->sendText(string("Could not find ") + arg + " here.\n\r");
		return;
	}

	victim->setStageName(argument);
	ch->sendText( string("Stage name set to ") + argument + " on " + victim->getName().str() + "\n\r" );
}

/*
 * quest point set - TRI
 * syntax is: qpset char give/take amount
 */

void do_qpset(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int amount;
	bool give = TRUE;

	if ( IS_NPC(ch) )
	{
		send_to_char( "Cannot qpset as an NPC.\n\r", ch );
		return;
	}

	set_char_color( AT_LOG, ch );
	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	amount = atoi( arg3 );

	if ( arg[0] == '\0' || arg2[0] == '\0' || amount <= 0 || (get_trust(ch) <= LEVEL_HERO_MAX && arg[4] == '\0') )
	{
		if ( get_trust(ch) <= LEVEL_HERO_MAX ) {
			send_to_char( "Syntax: qpset <character> give <amount> <reason>\n\r", ch );
		}
		else {
			send_to_char( "Syntax: qpset <character> <give/take> <amount>\n\r", ch );
		}
		send_to_char( "Amount must be a positive number greater than 0.\n\r", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "There is no such player currently playing.\n\r", ch );
		return;
	}

	if ( IS_NPC( victim ) )
	{
		send_to_char( "Glory cannot be given to or taken from a "
				"mob.\n\r", ch );
		return;
	}

	if ( nifty_is_name_prefix( arg2, "give" ) )
		give = TRUE;
	else if ( nifty_is_name_prefix( arg2, "take" ) && IS_IMMORTAL(ch) )
		give = FALSE;
	else
	{
		do_qpset( ch, "" );
		return;
	}

	if ( give )
	{
		if ( get_trust(ch) <= LEVEL_HERO_MAX )
		{
			victim->pcdata->quest_deci += amount;

			if ( victim->pcdata->quest_deci >= 10 )
			{
				victim->pcdata->quest_deci   = victim->pcdata->quest_deci % 10;
				victim->pcdata->quest_curr  += 1;
				victim->pcdata->quest_accum += 1;
			}

			ch_printf( victim, "Your glory has been increased by 0.%d.\n\r", amount );
		}
		else
		{
			victim->pcdata->quest_curr += amount;
			victim->pcdata->quest_accum += amount;
			ch_printf( victim, "Your glory has been increased by %d.\n\r", amount );
		}
	}
	else
	{
		victim->pcdata->quest_curr -= amount;
		ch_printf( victim, "Your glory has been decreased by %d.\n\r", amount );
	}

	send_to_char( "Ok.\n\r", ch );
	return;
}

void do_test(CHAR_DATA* ch, const char* argument) {
    ch->position = POS_MEDITATING;
}

void do_trainstat(CHAR_DATA* ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *vict = NULL;
    TRAIN_DATA *train = NULL;
    TRAIN_LIST_DATA *tlist = NULL;
    SkillType *skill = NULL;

    int count = 0;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char("Syntax: trainstat <mob vnum/name>\r\n", ch);
        return;
    }

    argument = one_argument(argument, arg1);

    if ( (vict = get_char_world(ch, arg1)) == NULL )
    {
	    send_to_char("Invalid mob!\r\n", ch);
	    return;
    }

    if ( !vict->pIndexData || !IS_NPC(vict))
    {
        send_to_char("That is not a mob!\r\n", ch);
        return;
    }

    if ( (train = vict->pIndexData->train) == NULL )
    {
        send_to_char("That mob has no train data!\r\n", ch);
        return;
    }

    ch_printf(ch, "Train Data for mob:  %s\r\n", vict->pIndexData->playerName_.c_str());
    ch_printf(ch, "Vnum:                %s\r\n", vnum_to_dotted(vict->pIndexData->vnum));

    if ( train->min_level == -1 )
        ch_printf(ch, "Min Level:          None\r\n");
    else
        ch_printf(ch, "Min Level:          %d\r\n", train->min_level);

    if ( train->max_level == -1 )
        ch_printf(ch, "Max Level:          None\r\n");
    else
        ch_printf(ch, "Max Level:          %d\r\n", train->max_level);

    if ( train->_class == -1 )
	    ch_printf(ch, "Train Class:        All\r\n");
    else
        ch_printf(ch, "Train Class:        %s\r\n", class_table[train->_class]->whoName_.c_str());

    switch ( train->alignment )
    {
        case -1:
            ch_printf(ch, "Train Alignment:    ALL\r\n");
            break;
        case  0:
        	ch_printf(ch, "Train Alignment:    EVIL\r\n");
            break;
        case 1:
            ch_printf(ch, "Train Alignment:    NEUTRAL\r\n");
            break;
        case 2:
            ch_printf(ch, "Train Alignment:    GOOD\r\n");
            break;
    }

    ch_printf(ch, "Train Base Cost:    %d\r\n", train->base_cost);
    ch_printf(ch, "Gain Cost:          %d\r\n", train->gain_cost);


    tlist = train->first_in_list;

    if ( tlist ) {
        send_to_char("\r\n--------- TRAINER'S SKILLS ----------\r\n", ch);
    } else {
        send_to_char("\r\nThis mob is set to teach all skills available to it.\r\n", ch);
    }

    for ( count = 0; tlist; count++ )
    {
	    skill = get_skilltype(tlist->sn);
        ch_printf(ch, "[#%2d] Name: %15s [%3d] Max: %3d Cost: %3d\r\n",
		          count,
		          (skill) ? skill->name_.c_str() : "UNKNOWN",
		          tlist->sn, tlist->max, tlist->cost);
        tlist = tlist->next;
    }
}

void do_trainset(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *vict = NULL;
    TRAIN_DATA *train = NULL;
    TRAIN_LIST_DATA *tlist = NULL;
    SkillType *skill = NULL;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char("Syntax: trainset <mob vnum/name> [command]\r\n", ch);
        send_to_char("Command is one of the following:\r\n", ch);
        ch_printf(ch, "%10s %10s %10s %10s\r\n%10s %10s %10s %10s\r\n%10s\r\n",
		  "create", "delete", "class", "min_level",
		  "max_level", "base_cost", "alignment", "skills",
          "gain_cost");
    	return;
    }

    argument = one_argument(argument, arg1);

    if ( (vict = get_char_world(ch, arg1)) == NULL )
    {
    	send_to_char("Invalid mob!\r\n", ch);
    	return;
    }

    if ( !vict->pIndexData || !IS_NPC(vict))
    {
        send_to_char("That is not a mob!\r\n", ch);
        return;
    }

    if ( argument && argument[0] != '\0' ) {
        argument = one_argument(argument, arg1);
    } else {
    	do_trainset(ch, "");
        return;
    }

    train = vict->pIndexData->train;

    if ( !train && str_cmp(arg1, "create"))
    {
        send_to_char("That mob has no train data.\r\n", ch);
        send_to_char("Use the command:\r\n\ttrainset <mob> create\r\n", ch);
        return;
    }

    if ( !str_cmp(arg1, "create" ) )
    {
        if ( train )
        {
            send_to_char("That mob already has train data.\r\n", ch);
            return;
        }

        CREATE( train, TRAIN_DATA, 1 );
        train->alignment = -1;
        train->_class = -1;
        train->vnum = 0;
        train->max_level = -1;
        train->min_level = -1;
        train->base_cost = -1;
        train->gain_cost = -1;
        train->first_in_list = NULL;
        train->last_in_list = NULL;
        vict->pIndexData->train = train;

        LINK( train, first_train, last_train, next, prev );

        send_to_char("Train data added.\r\n", ch);
    }
    else if ( !str_cmp(arg1, "delete" ) )
    {
        if ( !train )
        {
            send_to_char("That mob has no train data to delete!\r\n", ch);
            return;
        }

	    UNLINK(train, first_train, last_train, next, prev);
        vict->pIndexData->train = NULL;
        send_to_char("Train data removed!\r\n", ch);
    }
    else if ( !str_cmp(arg1, "class" ) )
    {
        int _class;
        bool found;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Syntax: trainset <mob> class <class name>\r\n", ch);
            return;
        }

        argument = one_argument(argument, arg1);

        if ( is_number(arg1) )
        {
            _class = atoi(arg1);

            if ( (_class > MAX_CLASS || _class < 0) && _class != -1 )
            {
                send_to_char("Invalid class!\r\n", ch);
                return;
            }

            train->_class = _class;
            send_to_char("Train class set.\r\n", ch);
            return;
        }

        if ( !str_cmp(arg1, "ALL") || !str_cmp(arg1, "ANY") )
        {
            train->_class = -1;
            send_to_char("Class set to ALL.\r\n", ch);
            return;
        }

	    for ( _class = 0; _class < MAX_CLASS; _class++ )
        {
            if ( class_table[_class]->whoName_.ciEqual(arg1) )
            {
                found = TRUE;
                break;
            }
        }

	    if ( found )
        {
            train->_class = _class;
            send_to_char("Train class set.\r\n", ch);
            return;
        }
        else
        {
            send_to_char("Invalid class!\r\n", ch);
            return;
        }
    }
    else if (!str_cmp(arg1, "min_level") )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Syntax: trainset <mob> min_level (<level>/NONE)\r\n",
                         ch);

            return;
        }

        argument = one_argument(argument, arg1);

        if ( !str_cmp(arg1, "NONE") )
        {
            train->min_level = -1;
            send_to_char("Min level set to NONE.\r\n", ch);
            return;
        }
        else if (!is_number(arg1) )
        {
            send_to_char("Syntax: trainset <mob> min_level (<level>/NONE)\r\n", ch);
            return;
        }
        else
        {
            train->min_level = atoi(arg1);

            if ( train->min_level < -1 || train->min_level > MAX_LEVEL )
            {
                send_to_char("Invalid level!\r\n", ch);
                return;
            }

            ch_printf(ch, "Min Level set to %d\r\n", train->min_level);
        }
    }
    else if (!str_cmp(arg1, "max_level") )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Syntax: trainset <mob> max_level (<level>/NONE)\r\n",
                         ch);
            return;

	    }

        argument = one_argument(argument, arg1);

        if ( !str_cmp(arg1, "NONE") )
        {
            train->max_level = -1;
            send_to_char("Max level set to NONE.\r\n", ch);
            return;
        }
        else if (!is_number(arg1) )
        {
            send_to_char("Syntax: trainset <mob> min_level (<level>/NONE)\r\n",
                         ch);
            return;
        }
        else
        {
            train->max_level = atoi(arg1);

            if ( train->max_level < -1 || train->max_level > MAX_LEVEL )
            {
                send_to_char("Invalid level!\r\n", ch);
                return;
            }

            ch_printf(ch, "Level set to %d\r\n", train->max_level);
        }
    }
    else if ( !str_cmp(arg1, "gain_cost") )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Syntax: trainset <mob> gain_cost <cost>\r\n", ch);
            return;
        }

        argument = one_argument(argument, arg1);

	    if ( !is_number(arg1) )
        {
            send_to_char("Syntax: trainset <mob> gain_cost <cost>\r\n", ch);
            return;
        }

        train->gain_cost = atoi(arg1);

        if ( train->gain_cost == -1 || train->base_cost == 0)
        {
            train->gain_cost = 0;
            send_to_char("Gain cost set to 0 coins.\r\n", ch);
            return;
        }
        else if ( train->gain_cost < -1 )
        {
            train->gain_cost *= -1;

            ch_printf(ch, "Sorry, you can't _GIVE_ them money for training!\r\n");
            ch_printf(ch, "Gain cost set to %d coins.\r\n");
            return;
        }
    }
    else if ( !str_cmp(arg1, "base_cost") )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Syntax: trainset <mob> base_cost <cost>\r\n", ch);
            return;
        }

        argument = one_argument(argument, arg1);

	    if ( !is_number(arg1) )
        {
            send_to_char("Syntax: trainset <mob> base_cost <cost>\r\n", ch);
            return;
        }

        train->base_cost = atoi(arg1);

        if ( train->base_cost == -1 || train->base_cost == 0)
        {
            train->base_cost = 0;
            send_to_char("Base cost set to 0 coins.\r\n", ch);
            return;
        }
        else if ( train->base_cost < -1 )
        {
            train->base_cost *= -1;

            ch_printf(ch, "Sorry, you can't _GIVE_ them money for training!\r\n");
            ch_printf(ch, "Base cost set to %d coins.\r\n");
            return;
        }
    }
    else if (!str_cmp(arg1, "alignment") || !str_cmp(arg1, "align") )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Syntax: trainset <mob> alignment <NONE/GOOD/EVIL/NEUTRAL>\r\n", ch);
            return;
        }

        argument = one_argument(argument, arg1);

        if (!str_cmp(arg1, "GOOD") )
        {
            send_to_char("Train alignment set to GOOD.\r\n", ch);
            train->alignment = 2;
            return;
        }
        else if ( !str_cmp(arg1, "EVIL" ))
        {
            send_to_char("Train alignment set to EVIL.\r\n", ch);
            train->alignment = 0;
            return;
        }
        else if (!str_cmp(arg1, "NEUTRAL" ))
        {
            send_to_char("Train alignment set to NEUTRAL.\r\n", ch);
            train->alignment = 1;
            return;
        }
        else
        {
            send_to_char("Train alignment set to NONE.\r\n", ch);
            train->alignment = -1;
            return;
        }
    }
    else if (!str_cmp(arg1, "skills"))
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Syntax: trainset <mob> skills <command>\r\n", ch);
            send_to_char("Command is one of the following:\r\n", ch);
            ch_printf(ch, "%10s %10s %10s %10s\r\n",
                          "add", "delete", "set", "list");
            return;
        }

        argument = one_argument(argument, arg1);

        if ( !str_cmp(arg1, "add") )
        {
            int sn = 0;

            if ( !argument || argument[0] == '\0' )
            {
                send_to_char("Syntax: trainset <mob> skills add <skill>\r\n", ch);
                return;
            }

            if (is_number(argument) )
            {
                argument = one_argument(argument, arg1);
                sn = atoi(arg1);
                skill = get_skilltype(sn);

                if ( !skill )
                {
                    send_to_char("Invalid skill number!\r\n", ch);
                    return;
                }
            }
            else
            {
                if ( (sn = skill_lookup(argument)) )
                {
                    skill = get_skilltype(sn);

                    if (!skill)
                    {
                        send_to_char("Invalid skill!\r\n", ch);
                        return;
                    }
                }
                else
                {
                    send_to_char("Invalid skill!\r\n", ch);
                    return;
                }
            }

            for ( tlist = train->first_in_list; tlist; tlist = tlist->next )
            {
                if ( tlist->sn == sn )
                {
                    send_to_char("Mob already has skill data for that skill.\r\n", ch);
                    return;
                }
            }

            CREATE(tlist, TRAIN_LIST_DATA, 1);
            tlist->sn = sn;
            tlist->max = 0;
            tlist->cost = 0;

            LINK(tlist, train->first_in_list, train->last_in_list, next, prev);

            ch_printf(ch, "Skill %s (%d) added.\r\n", (skill) ? skill->name_.c_str() : "", sn);
        }
        else if ( !str_cmp(arg1, "set" ) )
        {
            int skill_num = 0;
            int count = 0;

            if (!argument || argument[0] == '\0')
            {
                send_to_char("Syntax: trainset <mob> skills set <skill number> <field> <value>\r\n", ch);
                ch_printf(ch, "Where field is one of the following:\r\n%10s %10s\r\n", "max_percent", "cost");
                return;
            }

            argument = one_argument(argument, arg1);

            if (!is_number(arg1) || !argument || argument[0] == '\0')
            {
                send_to_char("Syntax: <mob> skills set <skill number> <field> <value>\r\n", ch);
                ch_printf(ch, "Where field is one of the following:\r\n%10s %10s\r\n", "max_percent", "cost");
                return;
            }
            else
            {
                skill_num = atoi(arg1);
            }

	        for ( count = 0, tlist = train->first_in_list; tlist; tlist = tlist->next, count++ )
            {
                if ( count == skill_num )
                {
                    break;
                }
            }

            if ( count < skill_num )
            {
                send_to_char("Invalid skill number!\r\n", ch);
                return;
            }

	        argument = one_argument(argument, arg1);

            if ( !str_cmp(arg1, "max_percent") || !str_cmp(arg1, "max"))
            {
                int percent = 0;

                if ( !argument || argument[0] == '\0' )
                {
                    send_to_char("Syntax: <mob> skills set <skill number> max_percent <value>\r\n", ch);
                    return;
                }

                argument = one_argument(argument, arg1);

		        if (!is_number(arg1))
                {
                    send_to_char("Syntax: <mob> skills set <skill number> max_percent <value>\r\n", ch);
                    return;
                }

		        percent = atoi(arg1);

                if ( percent <= 0 || percent >= 101 )
                {
                    send_to_char("Invalid percent!\r\n", ch);
                    return;
                }
                else
		        {
                    tlist->max = percent;
                    ch_printf(ch, "Max percent for skill #%d set to %d.\r\n", skill_num, tlist->max);
                }
            }
            else if (!str_cmp(arg1, "cost") )
            {
                int cost = 0;

                if ( !argument || argument[0] == '\0' )
                {
                    send_to_char("Syntax: trainset <mob> skills set <skill num> cost <amount>\r\n", ch);
                    return;
                }

		        argument = one_argument(argument, arg1);

                if ( !is_number(arg1) )
                {
                    send_to_char("Syntax: trainset <mob> skills set <skill num> cost <amount>\r\n", ch);
                    return;
                }

		        cost = atoi(arg1);

                if ( cost == -1 || cost == 0 )
                {
                    cost = 0;
                }
                else if ( cost < -1 )
                {
                    cost *= -1;
                }

                ch_printf(ch, "Cost for skill #%d now set to %d coins.\r\n", skill_num, cost);
                tlist->cost = cost;

                return;
            }
            else
            {
                send_to_char("Syntax: trainset <mob> skills set <skill number> <field> <value>\r\n", ch);
                ch_printf(ch, "Where field is one of the following:\r\n%10s %10s\r\n", "max_percent", "cost");
                return;
            }
        }
        else if ( !str_cmp(arg1, "delete") )
        {
            int count = 0;
            int skill_num = 0;

            if ( !argument || argument[0] == '\0' )
            {
                send_to_char("Syntax: trainset <mob> skills delete <skill num>\r\n", ch);
                return;
            }

            argument = one_argument(argument, arg1);

            if (!is_number(arg1))
            {
                send_to_char("Syntax: trainset <mob> skills delete <skill num>\r\n", ch);
                return;
            }
            else
            {
                skill_num = atoi(arg1);
            }

            for ( count = 0, tlist = train->first_in_list; tlist; tlist = tlist->next, count++ )
            {
                if ( count == skill_num )
                {
                    break;
                }
            }

            if ( count < skill_num || skill_num < 0)
            {
                send_to_char("Invalid skill num!\r\n", ch);
                return;
            }

            UNLINK(tlist, train->first_in_list, train->last_in_list, next, prev);
            DISPOSE(tlist);
	        ch_printf(ch, "Skill #%d removed.\r\n", skill_num);
        }
        else if ( !str_cmp(arg1, "list") )
        {
            int count;

            tlist = train->first_in_list;

            if ( train ) {
                send_to_char("\r\n--------- TRAINER'S SKILLS ----------\r\n", ch);
            } else {
                send_to_char("\r\nThis mob is set to teach all skills available to it.\r\n", ch);
            }

            for ( count = 0; tlist; count++ )
            {
                skill = get_skilltype(tlist->sn);
                ch_printf(ch, "[#%2d] Name: %15s [%3d] Max: %3d Cost: %3d\r\n",
            		      count,
		                  (skill) ? skill->name_.c_str() : "UNKNOWN",
                          tlist->sn, tlist->max, tlist->cost);
                          tlist = tlist->next;
            }
    	}
        else
    	{
            send_to_char("Syntax: trainset <mob> skills set <skill number> <field> <value>\r\n", ch);
            ch_printf(ch, "Where field is one of the following:\r\n%10s %10s\r\n", "max_percent", "cost");
            return;
        }
    }
    else
    {
        do_trainset(ch, "");
    }

    return;
}

void do_doas(CHAR_DATA* ch, const char* argument)
{
	CHAR_DATA *vic;
	PlayerConnection *vorig;
	PlayerConnection *corig;
	char name[MAX_INPUT_LENGTH];

	argument = one_argument(argument, name);

	if ( name[0] == '\0' ) {
		send_to_char("Usage: doas <victim> <command>\r\n", ch);
		return;
	}

	vic = get_char_world(ch, name);

	if ( !vic ) {
		send_to_char("No such victim.\r\n", ch);
		return;
	}

	if ( !IS_NPC(vic) && get_trust(ch) < sysdata.level_forcepc ) {
		send_to_char("You can only do that sort of stuff on mobs.\r\n", ch);
		return;
	}

	if ( get_trust(ch) < get_trust(vic) ) {
		send_to_char("You try, but just can't seem to.\r\n", ch);
		return;
	}

	if ( can_see(vic, ch) )
	{
		ch_printf(vic, "You feel %s taking over your soul for a few seconds.\r\n",
			PERS(ch, vic));
	}

	vorig = vic->GetConnection();
	corig = ch->GetConnection();

	vic->SetConnection(ch->GetConnection());
	ch->SetConnection(NULL);

	interpret (vic, argument);

	ch->SetConnection(corig);
	vic->SetConnection(vorig);

	if ( IS_IMMORTAL(vic) && can_see(vic, ch)) {
		ch_printf(vic, "%s performed the following commands: '%s'\r\n",
			PERS(ch, vic), argument);
	}
}

void do_mudconnector(CHAR_DATA *ch, const char* argument)
{
    FILE*fp;
    char buf[MAX_INPUT_LENGTH];

    if ( (fp = fopen("../system/stats.txt", "r")) == NULL ) {
        send_to_char("Can't open the stats file!\r\n", ch);
        return;
    }

    while ( !feof(fp) ) {
        if ( fgets(buf, MAX_INPUT_LENGTH, fp) )
            send_to_char(buf, ch);
    }

    fclose(fp);
}

void do_spam(CHAR_DATA *ch, const char* argument) {
    char target [MAX_INPUT_LENGTH];
    char topic  [MAX_INPUT_LENGTH];
    CHAR_DATA* vch;
    HELP_DATA* pHelp;

    if ( !argument || argument[0] == '\0' ) {
        send_to_char("Syntax: spam <target> <helptopic>\n", ch);
        send_to_char("Where <target> is one of: imms, mortals, room, area, or a player name\n", ch);
        return;
    }

    argument = one_argument(argument, target);

    if ( !argument || argument[0] == '\0' ) {
        do_spam(ch, "");
        return;
    }

    argument = one_argument(argument, topic);

    if ( !(pHelp = get_help(ch, topic)) ) {
        send_to_char("That help topic doesn't exist.\r\n", ch);
        return;
    }

    if ( (vch = get_char_world(ch, target)) ) {
        if ( get_trust(ch) >= get_trust(vch) ) {
            ch_printf(ch, "Spamming %s with: %s\r\n", vch->getName().c_str(), pHelp->keyword);
            set_char_color(AT_PLAIN, vch);
            send_to_char(pHelp->text, vch);
        } else {
            send_to_char("Nah.\r\n", ch);
        }
        return;
    } else if ( !str_prefix("imms", target) ) {
        ch_printf(ch, "Spamming all immortals with: %s\r\n", pHelp->keyword);

        for ( vch = first_char; vch; vch = vch->next ) {
            if ( IS_IMMORTAL(vch) && !IS_NPC(vch) ) {
                set_char_color(AT_PLAIN, vch);
                send_to_char(pHelp->text, vch);
            }
        }
    } else if ( !str_prefix("mortals", target) ) {
        ch_printf(ch, "Spamming all mortals with: %s\r\n", pHelp->keyword);

        for ( vch = first_char; vch; vch = vch->next ) {
            if ( !IS_IMMORTAL(vch) && !IS_NPC(vch) ) {
                set_char_color(AT_PLAIN, vch);
                send_to_char(pHelp->text, vch);
            }
        }
    } else if ( !str_prefix("room", target) ) {
        ch_printf(ch, "Spamming everyone in the room with: %s\r\n", pHelp->keyword);

        for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next_in_room ) {
            if ( !IS_NPC(vch) ) {
                set_char_color(AT_PLAIN, vch);
                send_to_char(pHelp->text, vch);
            }
        }
    } else {
        ch_printf(ch, "Spamming EVERYBODY with: %s\r\n", pHelp->keyword);

        for ( vch = first_char; vch; vch = vch->next ) {
            if ( !IS_NPC(vch) ) {
                set_char_color(AT_PLAIN, vch);
                send_to_char(pHelp->text, vch);
            }
        }
    }
}


// update mud.h to make knob 0 the no-loot
struct qvar qvar[QVAR_MAX]=
{
   {0,"Log mistwalks, astrals, etc."}
  ,{0,"No player looting"}
  ,{0,"reserved"}
};

void do_qstat(CHAR_DATA *ch, const char* argument)
{
	int i;
	for(i=0;i<QVAR_MAX;i++)
	{
		ch_printf(ch,"Knob %d: %s (%s)\r\n",
			i, qvar[i].desc,
			(qvar[i].value ? "on" : "off") );
	}
}

void do_qset(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *res;
	int idx;

	argument = one_argument(argument,arg);
	if(arg[0] == 0)
		res = "Qset which to what?";
	else
	{
		idx = atoi(arg);
		if(idx < 0 || idx >= QVAR_MAX)
			res = "Bad qset argument";
		else
		{
			argument = one_argument(argument,arg);
			if(arg[0])
			{
				qvar[idx].value = atoi(arg);
				res="Ok.";
			}
			else
				res="Bad qset value";
		}
	}
	ch_printf(ch,"%s\r\n",res);
}

AREA_DATA *get_area_obj( OBJ_INDEX_DATA * pObjIndex )
{
   AREA_DATA *pArea;

   if( !pObjIndex )
   {
      bug( "get_area_obj: pObjIndex is NULL." );
      return NULL;
   }
   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      if( pObjIndex->vnum >= pArea->low_o_vnum && pObjIndex->vnum <= pArea->hi_o_vnum )
         break;
   }
   return pArea;
}

AREA_DATA *get_area_mob( MOB_INDEX_DATA * pMobIndex )
{
   AREA_DATA *pArea;

   if( !pMobIndex )
   {
      bug( "get_area_mob: pMobIndex is NULL." );
      return NULL;
   }
   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      if( pMobIndex->vnum >= pArea->low_m_vnum && pMobIndex->vnum <= pArea->hi_m_vnum )
         break;
   }
   return pArea;
}

