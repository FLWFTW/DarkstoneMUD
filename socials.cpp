
#include "mud.h"
#include "connection.h"
#include "World.h"

#include "paths.const.h"

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

bool check_social     ( CHAR_DATA *ch, const char *command,
			     char *argument );
char *act_string(const char *format, CHAR_DATA*to, CHAR_DATA*ch,
        const void *arg1, const void *arg2);
void global_social  (const char* str, CHAR_DATA*ch, CHAR_DATA*vict);
SOCIALTYPE* random_social(CHAR_DATA*ch, int flags);
void color_social(char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type);
    
const char * const social_flags[] = {
   "aggressive", "sex",      "shocking", "friendly",    "maleonly",
   "femaleonly", "neuteronly", "goodonly", "evilonly", "neutralonly",
   "revulsion",  "npconly",    "pconly",   "sleepable", "immortal", "global",
   "log", "globalecho", "communicable"
};

int get_socialflag(char * flag) {
   int x;
   
   for ( x = 0; x < MAX_SOCIAL_TYPES; x++ ) {
      if ( !str_cmp( flag, social_flags[x] ) ) {
	 return x;
      }
   }
   return -1;
}

SOCIALTYPE *social_index[27];   /* hash table for socials   */

SOCIALTYPE *find_social( const char *command )
{
   SOCIALTYPE *social;
   int hash;

   if ( command[0] < 'a' || command[0] > 'z' )
     {
	hash = 0;
     } else {
	hash = (command[0] - 'a') + 1;
     }
   
   for ( social = social_index[hash]; social; social = social->next ) {
      if ( !str_prefix( command, social->name ) ) {
	 return social;
      }
   }
   
   return NULL;
}

bool can_do_social(CHAR_DATA* ch, SOCIALTYPE* social) {
    if ( !IS_IMMORTAL(ch) &&
         (ch->GetInRoom() == get_room_index(358) || ch->GetInRoom() == get_room_index(8))) {
        if ( social->flags & SOCIAL_GLOBAL || social->flags & SOCIAL_GLOBALECHO ) {
            return FALSE;
        }
    }
    if ( social->min_level > ch->level )
        return FALSE;
    if ( (ch->sex == SEX_MALE)   && (social->flags & SOCIAL_FEMALE_ONLY) )
        return FALSE;
    if ( (ch->sex != SEX_NEUTRAL) && (social->flags & SOCIAL_NEUTER_ONLY) )
        return FALSE;
    if ( (ch->sex == SEX_FEMALE) && (social->flags & SOCIAL_MALE_ONLY) )
        return FALSE;
    if ( IS_EVIL(ch) && (social->flags & SOCIAL_EVIL_ONLY) )
        return FALSE;
    if ( IS_GOOD(ch) && (social->flags & SOCIAL_GOOD_ONLY) )
        return FALSE;
    if ( IS_NPC(ch)  && (social->flags & SOCIAL_PC_ONLY) )
        return FALSE;
    if ( !IS_NPC(ch) && (social->flags & SOCIAL_MOB_ONLY) )
        return FALSE;
    if ( !IS_IMMORTAL(ch) && (social->flags & SOCIAL_IMMORTAL) )
        return FALSE;

   return TRUE;
}
   
bool check_social( CHAR_DATA *ch, const char *command, const char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	SOCIALTYPE *social;

	if ( (social=find_social(command)) == NULL ) {
		return FALSE;
	}
	if ( !can_do_social(ch, social) ) {
		return FALSE;
	}

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
	{
		send_to_char( "You are anti-social!\n\r", ch );
		return TRUE;
	}

	switch ( ch->position )
	{
		case POS_DEAD:
			send_to_char( "Lie still; you are DEAD.\n\r", ch );
			return TRUE;

		case POS_INCAP:
		case POS_MORTAL:
			send_to_char( "You are hurt far too bad for that.\n\r", ch );
			return TRUE;

		case POS_STUNNED:
			send_to_char( "You are too stunned to do that.\n\r", ch );
			return TRUE;

		case POS_SLEEPING:
			/* Matthew, Check for SOCIAL_SLEEP */
			if ( social->flags & SOCIAL_SLEEPABLE )
				break;

			send_to_char( "In your dreams, or what?\n\r", ch );
			return TRUE;
	}

	one_argument( (char *) argument, arg );
	victim = NULL;
	if ( arg[0] == '\0' )
	{
		char buf[MAX_STRING_LENGTH];
		color_social(social->char_no_arg,   ch, NULL, victim, TO_CHAR);

		if ( social->flags & SOCIAL_GLOBALECHO ) {
			global_social(social->others_no_arg, ch, NULL);
		} else {
			color_social(social->others_no_arg, ch, NULL, victim, TO_ROOM);
		}

		if ( social->flags & SOCIAL_LOG ) {
			sprintf(buf, "[%s] %s: Social %s",
					vnum_to_dotted(ch->GetInRoom()->vnum), ch->getName().c_str(), social->name);
			log_string_plus(buf, LOG_NORMAL, get_trust(ch));
		}
	}
	else if ((social->flags&SOCIAL_GLOBAL)&&!(victim = get_char_world(ch, arg)))
	{
		send_to_char("Nobody like that exists in this world.\r\n", ch);
	}
	else if ( !victim && ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
	}
	else if ( victim == ch )
	{
		char buf[MAX_STRING_LENGTH];
		color_social(social->char_auto,     ch, NULL, victim, TO_CHAR);
		if ( social->flags & SOCIAL_GLOBALECHO ) {
			color_social(social->others_auto, ch, NULL, victim, TO_WORLD);
		} else {            
			color_social(social->others_auto,   ch, NULL, victim, TO_ROOM);
		}

		if ( social->flags & SOCIAL_LOG ) {
			sprintf(buf, "[%s] %s: Social %s to self",
					vnum_to_dotted(ch->GetInRoom()->vnum), ch->getName().c_str(), social->name);
			log_string_plus(buf, LOG_NORMAL, get_trust(ch));       
		}
	}
	else
	{ /* victim != ch */
		char buf[MAX_STRING_LENGTH];
		color_social(social->char_found,    ch, NULL, victim, TO_CHAR    );
		color_social(social->vict_found,    ch, NULL, victim, TO_VICT    );

		if (social->flags & SOCIAL_COMMUNICABLE
				&& ch->GetInRoom() == victim->GetInRoom())
		{
			spread_contagion(victim,ch); /* victim's revenge for annoying ch */
			spread_contagion(ch,victim); /* ch is typhoid mary */
		}

		if ( social->flags & SOCIAL_GLOBALECHO ) {
			/*           global_social(social->others_found, ch, victim); */
			color_social(social->others_found, ch, NULL, victim, TO_WORLD);
		} else {
			ROOM_INDEX_DATA *pOld;
			color_social(social->others_found,  ch, NULL, victim, TO_NOTVICT);
			if ( social->flags & SOCIAL_GLOBAL && ch->GetInRoom() != victim->GetInRoom()) {
				pOld = ch->GetInRoom();
				char_from_room(ch);
				char_to_room(ch, victim->GetInRoom());
				color_social(social->others_found, ch, NULL, victim, TO_NOTVICT);
				char_from_room(ch);
				char_to_room(ch, pOld);
			}
		}

		if ( !IS_NPC(ch) && IS_NPC(victim)
				&&   victim->GetConnection() == NULL // don't want a switched mob to do auto-socials
				&&   !IS_AFFECTED(victim, AFF_CHARM)
				&&   IS_AWAKE(victim) 
				&&   !IS_SET(victim->act, ACT_INANIMATE) // inanimates don't do socials!
				&&   !IS_SET( victim->pIndexData->progtypes, ACT_PROG ) )
		{
			SOCIALTYPE* ressoc = NULL;

			if ( social->resp_same_sex ) {
				ressoc = random_social(victim, social->resp_same_sex);
			} else if ( victim->sex == SEX_FEMALE && social->resp_vict_male ) {
				ressoc = random_social(victim, social->resp_vict_female);
			} else if ( victim->sex == SEX_MALE && social->resp_vict_male ) { 
				ressoc = random_social(victim, social->resp_vict_male);
			} else {
				ressoc = random_social(victim, social->flags);
			}

			if ( !ressoc ) {
				ressoc = social;
			}

			if ( can_do_social(victim, ressoc) ) {
				color_social(ressoc->others_found, victim, NULL, ch, TO_NOTVICT );
				color_social(ressoc->char_found,   victim, NULL, ch, TO_CHAR    );
				color_social(ressoc->vict_found,   victim, NULL, ch, TO_VICT    );
			}
		}

		if ( social->flags & SOCIAL_LOG )  {
			sprintf(buf, "[%s] %s: Social %s to %s",
					vnum_to_dotted(ch->GetInRoom()->vnum),
					ch->getName().c_str(),
					social->name,
					victim->getName().c_str()
				   );
			log_string_plus(buf, LOG_NORMAL, get_trust(ch));       
		}
	}


	return TRUE;
}

/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE *social )
{
	if ( social->name )
		DISPOSE( social->name );
	if ( social->char_no_arg )
		DISPOSE( social->char_no_arg );
	if ( social->others_no_arg )
		DISPOSE( social->others_no_arg );
	if ( social->char_found )
		DISPOSE( social->char_found );
	if ( social->others_found )
		DISPOSE( social->others_found );
	if ( social->vict_found )
		DISPOSE( social->vict_found );
	if ( social->char_auto )
		DISPOSE( social->char_auto );
	if ( social->others_auto )
		DISPOSE( social->others_auto );
	DISPOSE( social );
}

/*
 * Remove a social from it's hash index				-Thoric
*/
void unlink_social( SOCIALTYPE *social )
{
    SOCIALTYPE *tmp, *tmp_next;
    int hash;

    if ( !social )
    {
	bug( "Unlink_social: NULL social", 0 );
	return;
    }

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
	hash = 0;
    else
	hash = (social->name[0] - 'a') + 1;

    if ( social == (tmp=social_index[hash]) )
    {
	social_index[hash] = tmp->next;
	return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
	tmp_next = tmp->next;
	if ( social == tmp_next )
	{
	    tmp->next = tmp_next->next;
	    return;
	}
    }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE *social )
{
    int hash, x;
    SOCIALTYPE *tmp, *prev;

    if ( !social )
    {
	bug( "Add_social: NULL social", 0 );
	return;
    }

    if ( !social->name )
    {
	bug( "Add_social: NULL social->name", 0 );
	return;
    }

    if ( !social->char_no_arg )
    {
	bug( "Add_social: NULL social->char_no_arg", 0 );
	return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; social->name[x] != '\0'; x++ )
	social->name[x] = LOWER(social->name[x]);

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
	hash = 0;
    else
	hash = (social->name[0] - 'a') + 1;

    if ( (prev = tmp = social_index[hash]) == NULL )
    {
	social->next = social_index[hash];
	social_index[hash] = social;
	return;
    }

    for ( ; tmp; tmp = tmp->next )
    {
	if ( (x=strcmp(social->name, tmp->name)) == 0 )
	{
	    bug( "Add_social: trying to add duplicate name to bucket %d", hash );
	    free_social( social );
	    return;
	}
	else
	if ( x < 0 )
	{
	    if ( tmp == social_index[hash] )
	    {
		social->next = social_index[hash];
		social_index[hash] = social;
		return;
	    }
	    prev->next = social;
	    social->next = tmp;
	    return;
	}
	prev = tmp;
    }

    /* add to end */
    prev->next = social;
    social->next = NULL;
    return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit(CHAR_DATA *ch, const char* argument)
{
	SOCIALTYPE *social;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = smash_tilde_static( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	set_char_color( AT_SOCIAL, ch );

	if ( arg1[0] == '\0' ) {
		int i;

		send_to_char( "Syntax: sedit <social> [field]\n\r", ch );
		send_to_char( "Syntax: sedit <social> create\n\r", ch );

		send_to_char( "Syntax: sedit <social> delete\n\r", ch );
		send_to_char( "Syntax: sedit <save>\n\r", ch );
		send_to_char("Syntax: sedit <social> level <level>\r\n", ch);
		send_to_char( "Syntax: sedit <social> flags [flags]\r\n", ch);

		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\n\r", ch );

		ch_printf(ch, "\n\rFlags being on of:\r\n");
		ch_printf(ch, " ");
		for ( i = 0; i < MAX_SOCIAL_TYPES; ++i ) {
			if ( i % 4 == 0 && i != 0 ) { ch_printf(ch, "\r\n"); }
			ch_printf(ch, " %12s", social_flags[i]);
		}
		ch_printf(ch, "\r\n");

		return;
	}

	if ( !str_cmp( arg1, "save" ) ) {
		save_socials();
		send_to_char( "Saved.\n\r", ch );
		return;
	}

	social = find_social( arg1 );

	if ( !str_cmp( arg2, "create" ) ) {
		if ( social ) {
			send_to_char( "That social already exists!\n\r", ch );
			return;
		}
		CREATE( social, SOCIALTYPE, 1 );
		social->name = str_dup( arg1 );
		sprintf( arg2, "You %s.", arg1 );
		social->char_no_arg = str_dup( arg2 );
		add_social( social );
		send_to_char( "Social added.\n\r", ch );
		return;
	}

	if ( ! social ) {
		send_to_char("That social was not found!\r\n", ch);
		return;
	}

	if ( social->min_level > ch->level ) {
		send_to_char("You cannot modify this social!\r\n", ch);
		return;
	}

	if ( !str_cmp( arg2, "create" ) ) {
		if ( social ) {
			send_to_char( "That social already exists!\n\r", ch );
			return;
		}
		CREATE( social, SOCIALTYPE, 1 );
		social->name = str_dup( arg1 );
		sprintf( arg2, "You %s.", arg1 );
		social->char_no_arg = str_dup( arg2 );
		add_social( social );
		send_to_char( "Social added.\n\r", ch );
		return;
	}

	if ( !social ) {
		send_to_char( "Social not found.\n\r", ch );
		return;
	}

	if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) ) {
		ch_printf( ch, "Social: %s\n\r\n\rCNoArg: %s\n\r",
				social->name,	social->char_no_arg );
		ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
				social->others_no_arg	? social->others_no_arg	: "(not set)",
				social->char_found		? social->char_found	: "(not set)",
				social->others_found	? social->others_found	: "(not set)" );
		ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
				social->vict_found	? social->vict_found	: "(not set)",
				social->char_auto	? social->char_auto	: "(not set)",
				social->others_auto	? social->others_auto	: "(not set)" );
		ch_printf( ch, "Flags : %s\n\r",
				flag_string( social->flags, social_flags ) );
		ch_printf( ch, "RespSS: %s\r\n",
				flag_string( social->resp_same_sex, social_flags ) );
		ch_printf( ch, "RespVF: %s\r\n",
				flag_string( social->resp_vict_female, social_flags ) );
		ch_printf( ch, "RespVM: %s\r\n",
				flag_string( social->resp_vict_male, social_flags ) );
		ch_printf( ch, "Level : %d\r\n",
				social->min_level);   

		return;
	}

	if ( !str_cmp( arg2, "delete" ) ) {
		unlink_social( social );
		free_social( social );
		send_to_char( "Deleted.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "level" ) || !str_cmp(arg2, "minlevel") )
	{
		if ( argument[0] == '\0' ) {
			send_to_char("Min Level?\r\n", ch);
			return;
		}

		social->min_level = atoi(argument);

		if ( social->min_level < 0 || social->min_level > ch->level ) {
			send_to_char("Invalid min_level, set to 0\r\n", ch);
			social->min_level = 0;
		}

		return;
	}

	if ( !str_cmp( arg2, "cnoarg" ) ) {
		if ( argument[0] == '\0' || !str_cmp( argument, "clear" ) ) {
			send_to_char( "You cannot clear this field.  It must have a message.\n\r", ch );
			return;
		}

		if ( social->char_no_arg ) {
			DISPOSE( social->char_no_arg );
		}

		social->char_no_arg = str_dup( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "onoarg" ) ) {
		if ( social->others_no_arg ) {
			DISPOSE( social->others_no_arg );
		}

		if ( argument[0] != '\0' && str_cmp( argument, "clear" ) ) {
			social->others_no_arg = str_dup( argument );
		}

		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "cfound" ) ) {
		if ( social->char_found ) {
			DISPOSE( social->char_found );
		}

		if ( argument[0] != '\0' && str_cmp( argument, "clear" ) ) {
			social->char_found = str_dup( argument );
		}

		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "ofound" ) ) {
		if ( social->others_found ) {
			DISPOSE( social->others_found );
		}

		if ( argument[0] != '\0' && str_cmp( argument, "clear" ) ) {
			social->others_found = str_dup( argument );
		}

		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "vfound" ) ) {
		if ( social->vict_found ) {
			DISPOSE( social->vict_found );
		}

		if ( argument[0] != '\0' && str_cmp( argument, "clear" ) ) {
			social->vict_found = str_dup( argument );
		}

		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "flags" ) ) {
		bool pcflag;
		int  fSet;
		int  value;
		char arg3[MAX_INPUT_LENGTH] = "\0";

		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Usage: sedit <social> flags <flag> [flag]...\r\n", ch);
			return;
		}

		while ( argument[0] != '\0' ) {
			pcflag = FALSE;
			fSet = -1;

			if ( argument[0] == '+' ) {
				argument++;
				fSet = 1;
			} else if ( argument[0] == '-' ) {
				argument++;
				fSet = 0;
			}

			argument = one_argument(argument, arg3);

			value = get_socialflag(arg3);

			if ( value < 0 || value > MAX_SOCIAL_TYPES ) {
				ch_printf( ch, "Unknown flag: %s\r\n", arg3);
			} else {
				if ( fSet == -1 ) {
					TOGGLE_BIT( social->flags, 1 << value );
				} else if ( fSet == 1 ) {
					SET_BIT(social->flags, 1 << value);
				} else {
					REMOVE_BIT ( social->flags, 1 << value);
				}
			}
		}

		ch_printf(ch, "Done.\r\n");
		return;
	}

	if ( !str_cmp( arg2, "respss" ) ) {
		bool pcflag;
		int  fSet;
		int  value;
		char arg3[MAX_INPUT_LENGTH] = "\0";

		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Usage: sedit <social> flags <flag> [flag]...\r\n", ch);
			return;
		}

		while ( argument[0] != '\0' ) {
			pcflag = FALSE;
			fSet = -1;

			if ( argument[0] == '+' ) {
				argument++;
				fSet = 1;
			} else if ( argument[0] == '-' ) {
				argument++;
				fSet = 0;
			}

			argument = one_argument(argument, arg3);

			value = get_socialflag(arg3);

			if ( value < 0 || value > MAX_SOCIAL_TYPES ) {
				ch_printf( ch, "Unknown flag: %s\r\n", arg3);
			} else {
				if ( fSet == -1 ) {
					TOGGLE_BIT( social->resp_same_sex, 1 << value );
				} else if ( fSet == 1 ) {
					SET_BIT(social->resp_same_sex, 1 << value);
				} else {
					REMOVE_BIT ( social->resp_same_sex, 1 << value);
				}
			}
		}

		ch_printf(ch, "Done.\r\n");
		return;
	}

	if ( !str_cmp( arg2, "respvf" ) ) {
		bool pcflag;
		int  fSet;
		int  value;
		char arg3[MAX_INPUT_LENGTH] = "\0";

		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Usage: sedit <social> flags <flag> [flag]...\r\n", ch);
			return;
		}

		while ( argument[0] != '\0' ) {
			pcflag = FALSE;
			fSet = -1;

			if ( argument[0] == '+' ) {
				argument++;
				fSet = 1;
			} else if ( argument[0] == '-' ) {
				argument++;
				fSet = 0;
			}

			argument = one_argument(argument, arg3);

			value = get_socialflag(arg3);

			if ( value < 0 || value > MAX_SOCIAL_TYPES ) {
				ch_printf( ch, "Unknown flag: %s\r\n", arg3);
			} else {
				if ( fSet == -1 ) {
					TOGGLE_BIT( social->resp_vict_female, 1 << value );
				} else if ( fSet == 1 ) {
					SET_BIT(social->resp_vict_female, 1 << value);
				} else {
					REMOVE_BIT ( social->resp_vict_female, 1 << value);
				}
			}
		}

		ch_printf(ch, "Done.\r\n");
		return;
	}

	if ( !str_cmp( arg2, "respvm" ) ) {
		bool pcflag;
		int  fSet;
		int  value;
		char arg3[MAX_INPUT_LENGTH] = "\0";

		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Usage: sedit <social> flags <flag> [flag]...\r\n", ch);
			return;
		}

		while ( argument[0] != '\0' ) {
			pcflag = FALSE;
			fSet = -1;

			if ( argument[0] == '+' ) {
				argument++;
				fSet = 1;
			} else if ( argument[0] == '-' ) {
				argument++;
				fSet = 0;
			}

			argument = one_argument(argument, arg3);

			value = get_socialflag(arg3);

			if ( value < 0 || value > MAX_SOCIAL_TYPES ) {
				ch_printf( ch, "Unknown flag: %s\r\n", arg3);
			} else {
				if ( fSet == -1 ) {
					TOGGLE_BIT( social->resp_vict_male, 1 << value );
				} else if ( fSet == 1 ) {
					SET_BIT(social->resp_vict_male, 1 << value);
				} else {
					REMOVE_BIT ( social->resp_vict_male, 1 << value);
				}
			}
		}

		ch_printf(ch, "Done.\r\n");
		return;
	}


	if ( !str_cmp( arg2, "cauto" ) ) {
		if ( social->char_auto ) {
			DISPOSE( social->char_auto );
		}

		if ( argument[0] != '\0' && str_cmp( argument, "clear" ) ) {
			social->char_auto = str_dup( argument );
		}

		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "oauto" ) ) {
		if ( social->others_auto ) {
			DISPOSE( social->others_auto );
		}

		if ( argument[0] != '\0' && str_cmp( argument, "clear" ) ) {
			social->others_auto = str_dup( argument );
		}

		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "name" ) ) {
		bool relocate;

		one_argument( argument, arg1 );

		if ( arg1[0] == '\0' ) {
			send_to_char( "Cannot clear name field!\n\r", ch );
			return;
		}

		if ( arg1[0] != social->name[0] ) {
			unlink_social( social );
			relocate = TRUE;
		} else {
			relocate = FALSE;
		}

		if ( social->name ) {
			DISPOSE( social->name );
		}

		social->name = str_dup( arg1 );

		if ( relocate ) {
			add_social( social );
		}

		send_to_char( "Done.\n\r", ch );
		return;
	}

	/* display usage message */
	send_to_char("Unknown option\r\n", ch);
	do_sedit( ch, "" );
}

void fread_social( FILE *fp )
{
   char buf[MAX_STRING_LENGTH];
   const char *word;
   bool fMatch;
   SOCIALTYPE *social;
   
   CREATE( social, SOCIALTYPE, 1 );
   
   for ( ;; )
     {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;
	
	switch ( UPPER(word[0]) )
	  {
	   case '*':
	     fMatch = TRUE;
	     fread_to_eol( fp );
	     break;
	     
	   case 'C':
	     KEY( "CharNoArg",	social->char_no_arg,	fread_string_nohash(fp) );
	     KEY( "CharFound",	social->char_found,	fread_string_nohash(fp) );
	     KEY( "CharAuto",	social->char_auto,	fread_string_nohash(fp) );
	     break;

	   case 'F':
	     KEY( "Flags",      social->flags,          fread_number(fp));
	     break;

     	   case 'R':
	     KEY( "RespSS", social->resp_same_sex,    fread_number(fp));
	     KEY( "RespVF", social->resp_vict_female, fread_number(fp));
	     KEY( "RespVM", social->resp_vict_male,   fread_number(fp));
	     break;
	       
	   case 'E':
	     if ( !str_cmp( word, "End" ) )
	       {
		  if ( !social->name )
		    {
		       bug( "Fread_social: Name not found", 0 );
		       free_social( social );
		       return;
		    }
		  if ( !social->char_no_arg )
		    {
		       bug( "Fread_social: CharNoArg not found", 0 );
		       free_social( social );
		       return;
		    }
		  add_social( social );
		  return;
	       }
	    break;
	    
        case 'M':
        KEY("MinLevel", social->min_level, fread_number(fp) );
        break;
        
	   case 'N':
	     KEY( "Name",	social->name,		fread_string_nohash(fp) );
	     break;
	     
	   case 'O':
	     KEY( "OthersNoArg",	social->others_no_arg,	fread_string_nohash(fp) );
	     KEY( "OthersFound",	social->others_found,	fread_string_nohash(fp) );
	     KEY( "OthersAuto",	social->others_auto,	fread_string_nohash(fp) );
	     break;
	     
	   case 'V':
	     KEY( "VictFound",	social->vict_found,	fread_string_nohash(fp) );
	     break;
	  }
	
	if ( !fMatch )
	  {
	     sprintf( buf, "Fread_social: no match: %s", word );
	     bug( buf, 0 );
	  }
     }
}
   
void load_socials()
{
   FILE *fp;
   
   if ( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
     {
	top_sn = 0;
	for ( ;; )
	  {
	     char letter;
	     char *word;
	     
	     letter = fread_letter( fp );
	     if ( letter == '*' )
	       {
		  fread_to_eol( fp );
		  continue;
	       }
	     
	     if ( letter != '#' )
	       {
		  bug( "Load_socials: # not found.", 0 );
		  break;
	       }
	     
	     word = fread_word( fp );
	     if ( !str_cmp( word, "SOCIAL"      ) )
	       {
		  fread_social( fp );
		  continue;
	       }
	     else
	       if ( !str_cmp( word, "END"	) )
		 break;
	     else
	       {
		  bug( "Load_socials: bad section.", 0 );
		  continue;
	       }
	  }
	fclose( fp );
     }
   else
     {
	bug( "Cannot open socials.dat", 0 );
 	exit(1);
     }
}

/*
 * Save the socials to disk
 */
void save_socials()
{
   FILE *fpout;
   SOCIALTYPE *social;
   int x;
   
   if ( (fpout=fopen( SOCIAL_FILE, "w" )) == NULL )
     {
	bug( "Cannot open socials.dat for writting", 0 );
	perror( SOCIAL_FILE );
	return;
     }
   
   for ( x = 0; x < 27; x++ )
     {
	for ( social = social_index[x]; social; social = social->next )
	  {
	     if ( !social->name || social->name[0] == '\0' )
	       {
		  bug( "Save_socials: blank social in hash bucket %d", x );
		  continue;
	       }
	     fprintf( fpout, "#SOCIAL\n" );
	     fprintf( fpout, "Name        %s~\n",	social->name );
	     if ( social->char_no_arg )
	       fprintf( fpout, "CharNoArg   %s~\n",	social->char_no_arg );
	     else
	       bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
	     if ( social->others_no_arg )
	       fprintf( fpout, "OthersNoArg %s~\n",	social->others_no_arg );
	     if ( social->char_found )
	       fprintf( fpout, "CharFound   %s~\n",	social->char_found );
	     if ( social->others_found )
	       fprintf( fpout, "OthersFound %s~\n",	social->others_found );
	     if ( social->vict_found )
	       fprintf( fpout, "VictFound   %s~\n",	social->vict_found );
	     if ( social->char_auto )
	       fprintf( fpout, "CharAuto    %s~\n",	social->char_auto );
	     if ( social->others_auto )
	       fprintf( fpout, "OthersAuto  %s~\n",	social->others_auto );
	     fprintf(fpout,    "Flags       %d\n",      social->flags );
	     fprintf(fpout,    "RespSS      %d\n", social->resp_same_sex );
	     fprintf(fpout,    "RespVF      %d\n", social->resp_vict_female );
	     fprintf(fpout,    "RespVM      %d\n", social->resp_vict_male );
         fprintf(fpout,    "MinLevel    %d\n", social->min_level);
	     fprintf( fpout, "End\n\n" );
	  }
     }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

void do_socials(CHAR_DATA *ch, const char* argument)
{
   int iHash;
   int col = 0;
   SOCIALTYPE *social;
   FILE *socialout;
   
   if (!ch)
     {
	socialout = fopen( argument, "w" );
	fprintf( socialout, "<CENTER><TABLE>\n\r<TR>\n\r");
	
     }
   else
     set_pager_color( AT_PLAIN, ch );
   
   for ( iHash = 0; iHash < 27; iHash++ )
     for ( social = social_index[iHash]; social; social = social->next )
     {
	if ( !can_do_social(ch, social) ) {
	   continue;
	}
	
	if (!ch)
	  {
	     fprintf( socialout, "<TD>%s</TD>",social->name);
	     if ( ++col % 8 == 0 )
	       fprintf( socialout, "\n\r</TR>\n\r<TR>\n\r");
	  }
	else{
	   pager_printf( ch, "%-12s", social->name );
	   if ( ++col % 6 == 0 )
	     send_to_pager( "\n\r", ch );
	}
     }
   
   if (!ch)
     {
	
	fprintf( socialout, "</TR>\n\r");
	fprintf( socialout, "</TABLE></CENTER>\n\r");
	fclose( socialout );
     }
   else
     {
	if ( col % 6 != 0 )
	  send_to_pager( "\n\r", ch );
     }

   return;
}

SOCIALTYPE * random_social(CHAR_DATA *ch, int flags) {
   int count = 0;
   int hash;
   SOCIALTYPE *soc = NULL;
   SOCIALTYPE *keeper = NULL;
   char letter;

   for ( letter = 'a'; letter <= 'z'; letter++ ) {
      hash = letter - 'a';
      
      for ( soc = social_index[hash]; soc; soc = soc->next ) {
	 if ((soc->flags & flags || flags == 0) && can_do_social(ch, soc)) {
	    ++count;
	    if ( number_range(0, count) < 1 ) {
	       keeper = soc;
	    }
	 }
      }
   }
   
   return keeper;
}

void global_social(const char* str, CHAR_DATA* ch, CHAR_DATA* vict) {
	CHAR_DATA *vch;
	
	sh_int color;
	char * msg;
	
	if ( !str || str[0] == '\0' )
		return;
	
	msg = act_string(str, vict, ch, NULL, vict);
	
	if ( (color = get_color(msg)) ) {
		msg += 4;
		while ( *msg == ' ' )
			++msg;
	} else {
		color = AT_SOCIAL;
	}
	
	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if ( !(vch = d->GetCharacter()) )
			continue;
		if ( vch == vict || vch == ch )
			continue;
		
		set_char_color(color, vch);
		send_to_char(msg, vch);
	}
}

/* social -- almost identical to act, except it allows _col ors on the
   beginning
 */   
void color_social(char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
    sh_int color;

    if ( !format || format[0] == '\0' )
        return;
    
    if ( (color = get_color(format)) ) {
        format += 4;
        while ( *format == ' ' )
            format++;
    } else {
        color = AT_SOCIAL;
    }
    
    act(color, format, ch, arg1, arg2, type);

}
