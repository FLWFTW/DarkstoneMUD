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
 *			 Command interpretation module			    *
 ****************************************************************************/

#include <sys/types.h>
#ifdef unix
	#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "connection.h"

/*
 * Externals
 */
void refresh_page( CHAR_DATA *ch );
void subtract_times( struct timeval *etime, struct timeval *stime );



bool	check_social	( CHAR_DATA *ch, const char *command,
			    const char *argument );


/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;


CMDTYPE	   *command_hash[126];	/* hash table for cmd_table */
// SOCIALTYPE *social_index[27];   /* hash table for socials   */

/*
 * Character not in position for command?
 */
bool check_pos( CHAR_DATA *ch, sh_int position )
{
    if ( ch->position == POS_MEDITATING ) {
        send_to_char("You break your concentration, and stand up.\r\n", ch);
        act(AT_ACTION, "$n unties $mself and stands up.", ch, NULL, NULL,
                TO_ROOM);
        ch->position = POS_STANDING;
    }

    if ( ch->position < position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "A little difficult to do when you are DEAD...\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "You can't do that sitting down.\n\r", ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return FALSE;
    }
    return TRUE;
}

char buildpowers[]="savearea goto redit rstat rlist rdelete mcreate mset mstat mlist mload mdelete ocreate oset ostat olist oload odelete";

extern char lastplayercmd[MAX_INPUT_LENGTH*2];
extern CHAR_DATA *lastplayer;

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */


void interpret( Character *ch, const char *argument )
{
	char command[MAX_INPUT_LENGTH];
	char logline[MAX_INPUT_LENGTH];
	char logname[MAX_INPUT_LENGTH];
	//char newcommand[MAX_INPUT_LENGTH];
	//char arg2[MAX_INPUT_LENGTH];
	TIMER *timer = NULL;
	CMDTYPE *cmd = NULL;
	int trust;
	int loglvl;
	bool found;
	struct timeval time_used;
	long tmptime;

	signed int fcontinue;

	if ( !ch )
	{
		bug( "interpret: null ch!", 0 );
		return;
	}

	found = FALSE;
	if ( ch->substate == SUB_REPEATCMD )
	{
		DO_FUN *fun;

		if ( (fun=ch->last_cmd) == NULL )
		{
			ch->substate = SUB_NONE;
			bug( "interpret: SUB_REPEATCMD with NULL last_cmd", 0 );
			return;
		}
		else
		{
			int x;

			/*
			 * yes... we lose out on the hashing speediness here...
			 * but the only REPEATCMDS are wizcommands (currently)
			 */
			for ( x = 0; x < 126; x++ )
			{
				for ( cmd = command_hash[x]; cmd; cmd = cmd->next )
					if ( cmd->do_fun == fun )
					{
						found = TRUE;
						break;
					}
				if ( found )
					break;
			}
			if ( !found )
			{
				cmd = NULL;
				bug( "interpret: SUB_REPEATCMD: last_cmd invalid", 0 );
				return;
			}
			sprintf( logline, "(%s) %s", cmd->name, argument );
		}
	}

	// Not repeating a command - look it up in the tables.
	if ( !cmd )
	{
		/* Changed the order of these ifchecks to prevent crashing. */
		if ( !argument || !strcmp(argument,"") ) 
		{
			bug( "interpret: null argument!", 0 );
			return;
		}

		/*
		 * Strip leading spaces.
		 */
		while ( isspace(*argument) )
			argument++;
		if ( argument[0] == '\0' )
			return;

		timer = get_timerptr( ch, TIMER_DO_FUN );

		/* REMOVE_BIT( ch->affected_by, AFF_HIDE ); */

		/*
		 * Implement freeze command.
		 */
		if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
		{
			send_to_char( "You're totally frozen!\n\r", ch );
			return;
		}

		/*
		 * Grab the command word.
		 * Special parsing so ' can be a command,
		 *	also no spaces needed after punctuation.
		 */
		strcpy( logline, argument );
		if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
		{
			command[0] = argument[0];
			command[1] = '\0';
			argument++;
			while ( isspace(*argument) )
				argument++;
		}
		else
			argument = one_argument( (char *) argument, command );



		/* I'm tired, so if this looks as bad as I think it does, please change it */
		/* Like I told Faux, this kind of thing would get me kicked out of Prog 101 :) */
		if ( (LOWER(command[0]) == 's' && LOWER(command[1]) == 't') &&
				((LOWER(command[2] == 'a') && LOWER(command[3] == '\0')) || LOWER(command[2]) == '\0') ) {
			strcpy(command, "stand");
		}

		/*
		 * Look for command in command table.
		 * Check for council powers and/or bestowments
		 */
		trust = get_trust( ch );
		for ( cmd = command_hash[LOWER(command[0])%126]; cmd; cmd = cmd->next )
		{
			if (
					!str_prefix( command, cmd->name )
					&& (cmd->level <= trust
						||	(!IS_NPC(ch)
							&& ch->pcdata->council
							&& is_name( cmd->name, ch->pcdata->council->powers)
							&& cmd->level <= (trust+MAX_CPD)
							)
						||  (!IS_NPC(ch)
							&& ch->pcdata->bestowments_.length() > 0
							&& is_name( cmd->name, ch->pcdata->bestowments_.c_str() )
							)
						||  (!IS_NPC(ch)
							&& ch->pcdata->area
							&& get_trust(ch)<LEVEL_IMMORTAL
							&& is_name( cmd->name, buildpowers )
							)
					   )
			   )
			{
				found = TRUE;
				break;
			}
		}

		/*
		 * Turn off afk bit when any command performed.
		 *
		 * Made sure this only affects players, not NPCs.   - Ksilyan
		 */
		if ( !IS_NPC(ch) && IS_SET ( ch->act, PLR_AFK)  && (str_cmp(command, "AFK")))
		{
			REMOVE_BIT( ch->act, PLR_AFK );
			act( AT_GREY, "$n's eyes snap back into focus and $e looks around, bewildered.", ch, NULL, NULL, TO_ROOM );
		}
	}

	/*
	 * Log and snoop.
	 */
	sprintf( lastplayercmd, "** %s: %s", ch->getName().c_str(), logline );
	lastplayer=ch;

	if ( found && cmd->log == LOG_NEVER )
		strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );

	if ( found && cmd->name )
	{
		if (strlen(argument) > 0)
			sprintf(logline, "%s %s", cmd->name, argument);
		else
			strcpy(logline, cmd->name);
	}

	loglvl = found ? cmd->log : LOG_NORMAL;

	if ( 
			(
			 !IS_NPC(ch) &&
			 (IS_SET(ch->act, PLR_LOG) ||
			  (ch->GetConnection() && ch->GetConnection()->Account && IS_SET(ch->GetConnection()->Account->flags, ACCOUNT_LOGGED) )
			 )
			)
			||	 fLogAll
			||	 loglvl == LOG_BUILD
			||	 loglvl == LOG_HIGH
			||	 loglvl == LOG_ALWAYS )
	{
		/* Added by Narn to show who is switched into a mob that executes
		   a logged command.  Check for descriptor in case force is used. */
		if ( ch->GetConnection() && ch->GetConnection()->OriginalCharId != 0 ) 
			sprintf( log_buf, "Log [%s] %s (%s): %s", vnum_to_dotted(ch->GetInRoom()->vnum),
					ch->getName().c_str(), ch->GetConnection()->GetOriginalCharacter()->getName().c_str(), logline );
		else
			sprintf( log_buf, "Log [%s] %s: %s", vnum_to_dotted(ch->GetInRoom()->vnum),
					ch->getName().c_str(), logline );

		/*
		 * Make it so a 'log all' will send most output to the log
		 * file only, and not spam the log channel to death	-Thoric
		 */
		if ( fLogAll && loglvl == LOG_NORMAL
				&&	(IS_NPC(ch) || !IS_SET(ch->act, PLR_LOG)) )
			loglvl = LOG_ALL;

		/* This is handled in get_trust already */
		/*	if ( ch->Connection && ch->Connection->original )
			log_string_plus( log_buf, loglvl,
			ch->Connection->original->level );
			else*/
		log_string_plus( log_buf, loglvl, get_trust(ch) );
	}

	sprintf( logname, "%s", ch->getName().c_str());
	ch->SendCommandToSnoopers(logname, logline);


	if ( timer )
	{
		int tempsub;

		tempsub = ch->substate;
		ch->substate = SUB_TIMER_DO_ABORT;
		(timer->do_fun)(ch,"");
		if ( char_died(ch) )
			return;
		if ( ch->substate != SUB_TIMER_CANT_ABORT )
		{
			ch->substate = tempsub;
			extract_timer( ch, timer );
		}
		else
		{
			ch->substate = tempsub;
			return;
		}
	}

	fcontinue = -1;

	fcontinue = prog_command_trigger(ch, command, 1);

	if ( found && fcontinue == -1 ) {
		fcontinue = prog_command_trigger(ch, cmd->name, 1);
	}

	if ( fcontinue == -1 && !found ) {
		fcontinue = prog_command_trigger(ch, command, 0);
	}

	if ( fcontinue != 0 ) {
		/*
		 * Look for command in skill and socials table.
		 */
		if ( !found )
		{
			if ( !check_skill( ch, command, argument )
					&&   !check_social( ch, command, argument ) )
			{
				ExitData *pexit;

				/* check for an auto-matic exit command */
				if ( (pexit = find_door( ch, command, TRUE )) != NULL
						&&   IS_SET( pexit->exit_info, EX_xAUTO ))
				{
					if ( IS_SET(pexit->exit_info, EX_CLOSED)
							&& (!IS_AFFECTED(ch, AFF_PASS_DOOR)
								||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
					{
						if ( !IS_SET( pexit->exit_info, EX_SECRET ) )
							act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );
						else
							send_to_char( "You cannot do that here.\n\r", ch );
						return;
					}
					move_char( ch, pexit, 0 );
					return;
				}
				send_to_char( "Huh?\n\r", ch );
			}
			return;
		}

		/*
		 * Character not in position for command?
		 */
		if ( !check_pos( ch, cmd->position ) )
			return;

		/* Berserk check for flee.. maybe add drunk to this?.. but too much
		   hardcoding is annoying.. -- Altrag */
		if ( !str_cmp(cmd->name, "flee") &&
				IS_AFFECTED(ch, AFF_BERSERK) )
		{
			send_to_char( "You aren't thinking very clearly..\n\r", ch);
			return;
		}

		/*
		 * Dispatch the command.
		 */
		ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
		ch->last_cmd = cmd->do_fun;
		start_timer(&time_used);
		(*cmd->do_fun) ( ch, (char *) argument );
		end_timer(&time_used);
		/*
		 * Update the record of how many times this command has been used (haus)
		 */
		update_userec(&time_used, &cmd->userec);
		tmptime = UMIN(time_used.tv_sec,19) * 1000000 + time_used.tv_usec;

		/* laggy command notice: command took longer than 1.5 seconds */
		if ( tmptime > 1500000 )
		{
			sprintf(log_buf, "[*****] LAG: %s: %s %s (R:%s S:%ld.%06ld)", ch->getName().c_str(),
					cmd->name, (cmd->log == LOG_NEVER ? "XXX" : argument),
					vnum_to_dotted(ch->GetInRoom() ? ch->GetInRoom()->vnum : 0),
					time_used.tv_sec, time_used.tv_usec );
			log_string_plus(log_buf, LOG_NORMAL, get_trust(ch));
		}
	}

	tail_chain( );
}

CMDTYPE *find_command( const char *command )
{
    CMDTYPE *cmd;
    int hash;

    hash = LOWER(command[0]) % 126;

    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	if ( !str_prefix( command, cmd->name ) )
	    return cmd;

    return NULL;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number( const char *arg )
{
    if ( *arg == '\0' )
	return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
	if ( !isdigit(*arg) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( const char *argument, char *arg )
{
	const char *pdot;
	int number;

	for ( pdot = argument; *pdot != '\0'; pdot++ )
	{
		if ( *pdot == '.' )
		{
			string s = argument;
			// the number is everything until the dot
			number = atoi( s.substr(0, pdot-argument).c_str() );
			// then the argument is everything after the dot
			strcpy( arg, s.substr((pdot-argument)+1).c_str() );
			return number;
		}
	}

	strcpy( arg, argument );
	return 1;
}

#if 0
/*
 * Split a command into multiple commands.
 * Split character is a ;
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;
    char cPrev;
    sh_int count;

    count = 0;

    cEnd = ';';

    while ( *argument != '\0' || ++count >= 1024 )
    {
    	if ( *argument == cEnd && cPrev != '\\' )
    	{
    	    argument++;
    	    break;
    	}
    	*arg_first = *argument;
        cPrev = *argument;
    	arg_first++;
    	argument++;
    }

    *arg_first = '\0';

    return argument;
}
#endif
/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( const char *argument, char *arg_first )
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
		*arg_first = LOWER(*argument);
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( isspace(*argument) )
		argument++;

	// TODO: This is a horrible, HORRIBLE hack.
	// We *REALLY* shouldn't be casting from const char * to char * like this!!!
	// But to fix this requires changing basically every do_fun...
	return (char*) argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.  Delimiters = { ' ', '-' }
 */
char *one_argument2( const char *argument, char *arg_first )
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
		if ( *argument == cEnd || *argument == '-' )
		{
			argument++;
			break;
		}
		*arg_first = LOWER(*argument);
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( isspace(*argument) )
		argument++;

	// TODO: This is a horrible, HORRIBLE hack.
	// We *REALLY* shouldn't be casting from const char * to char * like this!!!
	// But to fix this requires changing basically every do_fun...
	return (char*)argument;
}

void do_timecmd(CHAR_DATA *ch, const char* argument)
{
  struct timeval stime;
  struct timeval etime;
  static bool timing;
  extern CHAR_DATA *timechar;
  char arg[MAX_INPUT_LENGTH];
  
  send_to_char("Timing\n\r",ch);
  if ( timing )
    return;
  one_argument(argument, arg);
  if ( !*arg )
  {
    send_to_char( "No command to time.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "update") )
  {
    if ( timechar )
      send_to_char( "Another person is already timing updates.\n\r", ch );
    else
    {
      timechar = ch;
      send_to_char( "Setting up to record next update loop.\n\r", ch );
    }
    return;
  }
  set_char_color(AT_PLAIN, ch);
  send_to_char( "Starting timer.\n\r", ch );
  timing = TRUE;
  gettimeofday(&stime, NULL);
  interpret(ch, argument);
  gettimeofday(&etime, NULL);
  timing = FALSE;
  set_char_color(AT_PLAIN, ch);
  send_to_char( "Timing complete.\n\r", ch );
  subtract_times(&etime, &stime);
  ch_printf( ch, "Timing took %d.%06d seconds.\n\r",
      etime.tv_sec, etime.tv_usec );
  return;
}

void start_timer(struct timeval *stime)
{
  if ( !stime )
  {
    bug( "Start_timer: NULL stime.", 0 );
    return;
  }
  gettimeofday(stime, NULL);
  return;
}

time_t end_timer(struct timeval *stime)
{
  struct timeval etime;
  
  /* Mark etime before checking stime, so that we get a better reading.. */
  gettimeofday(&etime, NULL);
  if ( !stime || (!stime->tv_sec && !stime->tv_usec) )
  {
    bug( "End_timer: bad stime.", 0 );
    return 0;
  }
  subtract_times(&etime, stime);
  /* stime becomes time used */
  *stime = etime;
  return (etime.tv_sec*1000000)+etime.tv_usec;
}

void send_timer(struct timerset *vtime, CHAR_DATA *ch)
{
  struct timeval ntime;
  int carry;
  
  if ( vtime->num_uses == 0 )
    return;
  ntime.tv_sec  = vtime->total_time.tv_sec / vtime->num_uses;
  carry = (vtime->total_time.tv_sec % vtime->num_uses) * 1000000;
  ntime.tv_usec = (vtime->total_time.tv_usec + carry) / vtime->num_uses;
  ch_printf(ch, "Has been used %d times this boot.\n\r", vtime->num_uses);
  ch_printf(ch, "Time (in secs): min %d.%0.6d; avg: %d.%0.6d; max %d.%0.6d"
      "\n\r", vtime->min_time.tv_sec, vtime->min_time.tv_usec, ntime.tv_sec,
      ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec);
  return;
}

void update_userec(struct timeval *time_used, struct timerset *userec)
{
  userec->num_uses++;
  if ( !timerisset(&userec->min_time)
  ||    timercmp(time_used, &userec->min_time, <) )
  {
    userec->min_time.tv_sec  = time_used->tv_sec;
    userec->min_time.tv_usec = time_used->tv_usec;
  }
  if ( !timerisset(&userec->max_time)
  ||    timercmp(time_used, &userec->max_time, >) )
  {
    userec->max_time.tv_sec  = time_used->tv_sec;
    userec->max_time.tv_usec = time_used->tv_usec;
  }
  userec->total_time.tv_sec  += time_used->tv_sec;
  userec->total_time.tv_usec += time_used->tv_usec;
  while ( userec->total_time.tv_usec >= 1000000 )
  {
    userec->total_time.tv_sec++;
    userec->total_time.tv_usec -= 1000000;
  }
  return;
}
