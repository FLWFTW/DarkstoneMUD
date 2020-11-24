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
 *			   Player communication module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "connection.h"
#include "connection_manager.h"
#include "World.h"
#include "paths.const.h"

/*
 *  Externals
 */
void send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page);
void send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page);
void send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void send_control_page_to_char(CHAR_DATA * ch, char page);

/* Text scrambler -- Altrag */
string scramble( const string & argument, int modifier )
{
    static char arg[MAX_INPUT_LENGTH];
    sh_int position;
    sh_int conversion = 0;

	modifier %= number_range( 80, 300 ); /* Bitvectors get way too large #s */
    for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
    {
    	if ( argument[position] == '\0' )
    	{
    		arg[position] = '\0';
    		return arg;
    	}
    	else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'A';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'A';
	    }
	    else if ( argument[position] >= 'a' && argument[position] <= 'z' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - 'a';
	    	conversion = number_range( conversion - 5, conversion + 5 );
	    	while ( conversion > 25 )
	    		conversion -= 26;
	    	while ( conversion < 0 )
	    		conversion += 26;
	    	arg[position] = conversion + 'a';
	    }
	    else if ( argument[position] >= '0' && argument[position] <= '9' )
	    {
	    	conversion = -conversion + position - modifier + argument[position] - '0';
	    	conversion = number_range( conversion - 2, conversion + 2 );
	    	while ( conversion > 9 )
	    		conversion -= 10;
	    	while ( conversion < 0 )
	    		conversion += 10;
	    	arg[position] = conversion + '0';
	    }
	    else
	    	arg[position] = argument[position];
	}
	arg[position] = '\0';
	return arg;
}


/* I'll rewrite this later if its still needed.. -- Altrag */
const char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
	return "";
}

string drunk_speech( const string & argument, CHAR_DATA *ch )
{
  const char *arg = argument.c_str();
  static char buf[MAX_INPUT_LENGTH*2];
  char buf1[MAX_INPUT_LENGTH*2];
  sh_int drunk;
  char *txt;
  char *txt1;

  if ( IS_NPC( ch ) || !ch->pcdata )
	  return argument;

  drunk = ch->pcdata->condition[COND_DRUNK];

  if ( drunk <= 0 )
    return argument;

  buf[0] = '\0';
  buf1[0] = '\0';

  /*
  if ( *arg == '\0' )
    return (char *) argument;
  */

  txt = buf;
  txt1 = buf1;

  while ( *arg != '\0' )
  {
    if ( toupper(*arg) == 'S' )
    {
	if ( number_percent() < ( drunk * 2 ) )		/* add 'h' after an 's' */
	{
	   *txt++ = *arg;
	   *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( toupper(*arg) == 'X' )
    {
	if ( number_percent() < ( drunk * 2 / 2 ) )
	{
	  *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
	}
       else
	*txt++ = *arg;
    }
   else if ( number_percent() < ( drunk * 2 / 5 ) )  /* slurred letters */
    {
      sh_int slurn = number_range( 1, 2 );
      sh_int currslur = 0;

      while ( currslur < slurn )
	*txt++ = *arg, currslur++;
    }
   else
    *txt++ = *arg;

    arg++;
  };

  *txt = '\0';

  txt = buf;

  while ( *txt != '\0' )   /* Let's mess with the string's caps */
  {
    if ( number_percent() < ( 2 * drunk / 2.5 ) )
    {
      if ( isupper(*txt) )
        *txt1 = tolower( *txt );
      else
      if ( islower(*txt) )
        *txt1 = toupper( *txt );
      else
        *txt1 = *txt;
    }
    else
      *txt1 = *txt;

    txt1++, txt++;
  };

  *txt1 = '\0';
  txt1 = buf1;
  txt = buf;

  while ( *txt1 != '\0' )   /* Let's make them stutter */
  {
    if ( *txt1 == ' ' )  /* If there's a space, then there's gotta be a */
    {			 /* along there somewhere soon */

      while ( *txt1 == ' ' )  /* Don't stutter on spaces */
        *txt++ = *txt1++;

      if ( ( number_percent() < ( 2 * drunk / 4 ) ) && *txt1 != '\0' )
      {
	sh_int offset = number_range( 0, 2 );
	sh_int pos = 0;

	while ( *txt1 != '\0' && pos < offset )
	  *txt++ = *txt1++, pos++;

	if ( *txt1 == ' ' )  /* Make sure not to stutter a space after */
	{		     /* the initial offset into the word */
	  *txt++ = *txt1++;
	  continue;
	}

	pos = 0;
	offset = number_range( 2, 4 );
	while (	*txt1 != '\0' && pos < offset )
	{
	  *txt++ = *txt1;
	  pos++;
	  if ( *txt1 == ' ' || pos == offset )  /* Make sure we don't stick */
	  {		               /* A hyphen right before a space	*/
	    txt1--;
	    break;
	  }
	  *txt++ = '-';
	}
	if ( *txt1 != '\0' )
	  txt1++;
      }
    }
   else
    *txt++ = *txt1++;
  }

  *txt = '\0';

  return buf;
}

static bool avopened[4]={0,0,0,0}; /* initialize all channels closed */
static const char *avname[4]=
{
   "None"
  ,"Hero"
  ,"Villain"
  ,"Sage"
};

static int avcolor[4]=
{
   0,
   AT_AVTALK,
   AT_EVTALK,
   AT_SGTALK
};

void do_avstat(CHAR_DATA *ch, const char* argument)
{
int i;

  (void) argument;
  for (i=0;i<3;i++)
  {
    ch_printf(ch,"%s channel is %s.\r\n",
               avname[i],
         (avopened[i] ? "open" : "closed")
             );
  }
}

void do_avopen(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	int ethos;

	if(IS_NPC(ch))
	{
		ch_printf(ch,"Mobs can't do that!\r\n");
		return;
	}

	argument = one_argument( argument, arg );

	if(arg[0])
	{
		if(!str_cmp(arg,"avtalk"))
			ethos=ETHOS_HERO;
		else if(!str_cmp(arg,"evtalk"))
			ethos=ETHOS_VILLAIN;
		else if(!str_cmp(arg,"sgtalk"))
			ethos=ETHOS_SAGE;
		else
		{
			ch_printf(ch,"Unknown avchannel name.\r\n");
			return;
		}
	}
	else
		ethos=ch->pcdata->ethos;

	if(ethos != ch->pcdata->ethos && get_trust(ch)<LEVEL_IMMORTAL)
	{
		ch_printf(ch,"You cannot affect that channel!\r\n");
		return;
	}

	if(avopened[ethos])
	{
		ch_printf(ch,"That channel is already open.\r\n");
		return;
	}

	avopened[ethos]=1; /* finally open the channel */

	sprintf( buf, "You open the %s channel.\r\n",avname[ethos]);
	act( avcolor[ethos], buf, ch, NULL, NULL, TO_CHAR );

	sprintf(buf,"$n opens the %s channel.\r\n",avname[ethos]);

	itorSocketId itor;

	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if ( !d )
		{
			gTheWorld->LogBugString("do_avopen: An invalid connection reference was found in the world list!");
			continue;
		}

		CHAR_DATA * och;
		CHAR_DATA * vch;

		och = d->GetOriginalCharacter();
		vch = d->GetCharacter();

		if ( get_trust(och) < LEVEL_HERO_MIN
			|| d->ConnectedState != CON_PLAYING
			)
			continue;

		if (vch != ch)
		{
			act( avcolor[ethos], buf, ch, NULL, vch, TO_VICT );
		}
	}
}


void do_avclose(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	int ethos;

	if(IS_NPC(ch))
	{
		ch_printf(ch,"Mobs can't do that!\r\n");
		return;
	}

	argument = one_argument( argument, arg );
	if(arg[0])
	{
		if(!str_cmp(arg,"avtalk"))
			ethos=ETHOS_HERO;
		else if(!str_cmp(arg,"evtalk"))
			ethos=ETHOS_VILLAIN;
		else if(!str_cmp(arg,"sgtalk"))
			ethos=ETHOS_SAGE;
		else
		{
			ch_printf(ch,"Unknown avchannel name.\r\n");
			return;
		}
	}
	else
		ethos=ch->pcdata->ethos;

	if(ethos != ch->pcdata->ethos && get_trust(ch)<LEVEL_IMMORTAL)
	{
		ch_printf(ch,"You cannot affect that channel!\r\n");
		return;
	}

	if(!avopened[ethos])
	{
		ch_printf(ch,"That channel is already closed.\r\n");
		return;
	}
	avopened[ethos]=0; /* finally close the channel */

	sprintf( buf, "You close the %s channel.\r\n",avname[ethos]);
	act( avcolor[ethos], buf, ch, NULL, NULL, TO_CHAR );

	sprintf(buf,"$n closes the %s channel.\r\n",avname[ethos]);

	itorSocketId itor;

	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		if ( !d )
		{
			gTheWorld->LogBugString("do_avopen: An invalid connection reference was found in the world list!");
			continue;
		}

		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->GetOriginalCharacter();
		vch = d->GetCharacter();

		if ( get_trust(och)<LEVEL_HERO_MIN
			|| d->ConnectedState != CON_PLAYING
			)
			continue;
		if (vch != ch)
		{
			act( avcolor[ethos], buf, ch, NULL, vch, TO_VICT );
		}
	}
}

/* Ksilyan:
 * Can ch communicate globally?
 */

bool CanGlobalCommunicate(CHAR_DATA * ch)
{
	if (!ch)
		return FALSE;

	// For now, NPCs can always communicate.
	if (IS_NPC(ch))
		return TRUE;

	// If you have a stone you're on by default.
	if (IsOnNetwork(ch))
		return TRUE;

	// Otherwise... get outta here.
	return FALSE;
}


/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, const char *argument, int channel, const char *verb )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int position;

	if ( !ch )
		return; // abort


	Room * inRoom = ch->GetInRoom();

	if ( IS_NPC( ch ) && channel == CHANNEL_CLAN )
	{
		send_to_char( "Mobs can't be in clans.\r\n", ch );
		return;
	}

	if ( IS_NPC( ch ) && channel == CHANNEL_ORDER )
	{
		send_to_char( "Mobs can't be in orders.\r\n", ch );
		return;
	}

	if ( IS_NPC( ch ) && channel == CHANNEL_COUNCIL )
	{
		send_to_char( "Mobs can't be in councils.\r\n", ch);
		return;
	}

	if ( IS_NPC( ch ) && channel == CHANNEL_GUILD )
	{
		send_to_char( "Mobs can't be in guilds.\r\n", ch );
		return;
	}

	if ( channel == CHANNEL_AVTALK
		|| channel == CHANNEL_EVTALK
		|| channel == CHANNEL_SGTALK
		)
	{
		if( IS_NPC(ch))
		{
			send_to_char("Mobs can't do that!\r\n", ch );
			return;
		}
		if(get_trust(ch) < LEVEL_IMMORTAL)
		{
			if( (channel==CHANNEL_AVTALK && ch->pcdata->ethos != ETHOS_HERO)
				|| (channel==CHANNEL_EVTALK && ch->pcdata->ethos != ETHOS_VILLAIN)
				|| (channel==CHANNEL_SGTALK && ch->pcdata->ethos != ETHOS_SAGE))
			{
				send_to_char("You can't use that channel!\r\n", ch);
				return;
			}
		}
	}

	if ( IS_SET( inRoom->room_flags, ROOM_SILENCE )
		&& channel != CHANNEL_PRAY)
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
		if ( ch->MasterId != 0 )
			send_to_char( "I don't think so...\r\n", CharacterMap[ch->MasterId] );
		return;
	}

	if ( argument[0] == '\0' )
	{
		sprintf( buf, "%s what?\r\n", verb );
		buf[0] = UPPER(buf[0]);
		send_to_char( buf, ch );	/* where'd this line go? */
		return;
	}

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
	{
		ch_printf( ch, "You can't %s.\r\n", verb );
		return;
	}

	REMOVE_BIT(ch->deaf, channel);

	switch ( channel )
	{
		default:
			set_char_color( AT_GOSSIP, ch );
			ch_printf( ch, "You %s '%s'\r\n", verb, argument );
			sprintf( buf, "$n %ss '$t'",	 verb );
			break;
		case CHANNEL_OOC:
			set_char_color( AT_OOC, ch );
			ch_printf( ch, "You %s '%s'\r\n", verb, argument );
			sprintf( buf, "$n %ss '$t'",	 verb );
			break;
		case CHANNEL_PRAY:
			set_char_color( AT_PRAY, ch);
			ch_printf(ch, "You %s '%s'\r\n", verb, argument);
			sprintf(buf, "$n %ss '$t'", verb);
			break;
		case CHANNEL_YELL:
			set_char_color(AT_YELL, ch);
			ch_printf(ch, "You %s '%s'\r\n", verb, argument);
			sprintf(buf, "$n %ss '$t'", verb);
			break;
		case CHANNEL_WARTALK:
			set_char_color( AT_WARTALK, ch );
			ch_printf( ch, "You %s '%s'\r\n", verb, argument );
			sprintf( buf, "$n %ss '$t'", verb );
			break;
		case CHANNEL_BELLOW:
			set_char_color( AT_IMMORT, ch );
			ch_printf( ch, "You %s '%s'\r\n", verb, argument );
			sprintf( buf, "$n %ss '$t'", verb );
			break;
		case CHANNEL_IMMTALK:
			sprintf( buf, "$n: $t" );
			position		= ch->position;
			ch->position	= POS_STANDING;
			act( AT_IMMORT, buf, ch, argument, NULL, TO_CHAR );
			ch->position	 = position;
			break;
		case CHANNEL_AVTALK:
			sprintf( buf, "$n> $t" );
			position	= ch->position;
			ch->position	= POS_STANDING;
			act( AT_AVTALK, buf, ch, argument, NULL, TO_CHAR );
			ch->position	= position;
			break;
		case CHANNEL_EVTALK:
			sprintf( buf, "$n< $t" );
			position  = ch->position;
			ch->position	  = POS_STANDING;
			act( AT_EVTALK, buf, ch, argument, NULL, TO_CHAR );
			ch->position	  = position;
			break;
		case CHANNEL_SGTALK:
			sprintf( buf, "$n= $t" );
			position  = ch->position;
			ch->position	  = POS_STANDING;
			act( AT_SGTALK, buf, ch, argument, NULL, TO_CHAR );
			ch->position	  = position;
			break;
		case CHANNEL_HIGHGOD:
			set_char_color( AT_MUSE, ch);
			ch_printf(ch, "You %s '%s'\r\n", verb, argument);
			sprintf(buf, "$n %ss '$t'", verb);
			break;
		case CHANNEL_CLAN:
			set_char_color(AT_CLAN, ch);
			ch_printf(ch, "You %s '%s'\r\n", verb, argument);
			sprintf(buf, "$n %ss '$t'", verb);
			break;
		case CHANNEL_COUNCIL:
			set_char_color(AT_COUNCIL, ch);
			ch_printf(ch, "You tell the council '%s'\r\n", argument);
			sprintf(buf, "$n tells the council '$t'");
			break;
		case CHANNEL_ORDER:
			set_char_color(AT_ORDER, ch);
			ch_printf(ch, "You %s '%s'\r\n", verb, argument);
			sprintf(buf, "$n %ss '$t'", verb);
			break;
		case CHANNEL_ARENA:
		{
			set_char_color(AT_ARENA, ch);
			ch_printf(ch, "::ARENA:: %s\r\n", argument);
			sprintf(buf, "::ARENA:: %s", argument);
		}
	}

	if ( IS_SET( inRoom->room_flags, ROOM_LOGSPEECH ) && channel != CHANNEL_ARENA )
	{
		sprintf( buf2, "%s: %s (%s)", ch->getShort(false).c_str(),
			argument, verb );
		append_to_file( LOG_FILE, buf2 );
	}

	if ( channel == CHANNEL_YELL )
	{
		mprog_yell_trigger(argument, ch);
	}

	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->GetOriginalCharacter();
		vch = d->GetCharacter();

		if ( d->ConnectedState == CON_PLAYING && vch != ch && !IS_SET(och->deaf, channel) )
		{
			string sbuf;
			Room * vchInRoom = RoomMap[vch->InRoomId];

			if ( channel == CHANNEL_PRAY && !IS_IMMORTAL(och) )
				continue;
			if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL(och) )
				continue;
			if ( channel == CHANNEL_WARTALK && NOT_AUTHED( och ) )
				continue;
			if ( channel == CHANNEL_AVTALK
				&& (get_trust(och) < LEVEL_HERO_MIN
				|| (get_trust(och) < LEVEL_IMMORTAL
				&& (och->pcdata->ethos != ETHOS_HERO
				&& !avopened[ETHOS_HERO]
				)
				)
				)
				)
				continue;
			if ( channel == CHANNEL_EVTALK
				&& (get_trust(och) < LEVEL_HERO_MIN
				|| (get_trust(och) < LEVEL_IMMORTAL
				&& (och->pcdata->ethos != ETHOS_VILLAIN
				&& !avopened[ETHOS_VILLAIN]
				)
				)
				)
				)
				continue;
			if ( channel == CHANNEL_SGTALK
				&& (get_trust(och) < LEVEL_HERO_MIN
				|| (get_trust(och) < LEVEL_IMMORTAL
				&& (och->pcdata->ethos != ETHOS_SAGE
				&& !avopened[ETHOS_SAGE]
				)
				)
				)
				)
				continue;

			if ( channel == CHANNEL_HIGHGOD && get_trust( och ) < sysdata.muse_level )
				continue;
			if ( channel == CHANNEL_HIGH	&& get_trust( och ) < sysdata.think_level )
				continue;

			/* Don't have channel restrictions on newbiechat
			if ( channel == CHANNEL_NEWBIE && get_trust(och) > 10
				&& get_trust(och) < LEVEL_HERO_MIN-1 )
				continue;
			*/

			if ( IS_SET( vchInRoom->room_flags, ROOM_SILENCE ) )
				continue;
			if ( channel == CHANNEL_YELL
				&&	 vchInRoom->area != inRoom->area )
				continue;

			if ( channel == CHANNEL_CLAN || channel == CHANNEL_ORDER
				||	 channel == CHANNEL_GUILD )
			{
				if ( IS_NPC( vch ) )
					continue;
				if ( vch->pcdata->clan != ch->pcdata->clan )
					continue;
			}

			if ( channel == CHANNEL_COUNCIL ) {
				if ( IS_NPC( vch ) )
					continue;
				if ( vch->pcdata->council != ch->pcdata->council )
					continue;
			}

			position		= vch->position;

			if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
				vch->position	= POS_STANDING;

			if ( !knows_language( vch, ch->speaking, ch ) && (!IS_NPC(ch) || ch->speaking != 0) )
				sbuf = scramble(argument, ch->speaking);
			else
				sbuf = argument;

			MOBtrigger = FALSE;

			if ( channel == CHANNEL_IMMTALK )
				act( AT_IMMORT, buf, ch, sbuf.c_str(), vch, TO_VICT );
			else if ( channel == CHANNEL_PRAY )
				act(AT_PRAY, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_BELLOW )
				act(AT_IMMORT, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_AVTALK )
				act( AT_AVTALK, buf, ch, sbuf.c_str(), vch, TO_VICT );
			else if ( channel == CHANNEL_EVTALK )
				act( AT_EVTALK, buf, ch, sbuf.c_str(), vch, TO_VICT );
			else if(channel == CHANNEL_SGTALK )
				act( AT_SGTALK, buf, ch, sbuf.c_str(), vch, TO_VICT );
			else if (channel == CHANNEL_WARTALK)
				act( AT_WARTALK, buf, ch, sbuf.c_str(), vch, TO_VICT );
			else if (channel == CHANNEL_HIGHGOD)
				act(AT_MUSE, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_YELL )
				act(AT_YELL, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_OOC )
			{
				if ( CanGlobalCommunicate(vch) )
					act(AT_OOC, buf, ch, sbuf.c_str(), vch, TO_VICT);
			}
			else if ( channel == CHANNEL_ARENA )
				act(AT_ARENA, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_CLAN )
				act(AT_CLAN, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_ORDER )
				act(AT_ORDER, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_COUNCIL)
				act(AT_COUNCIL, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( channel == CHANNEL_QUEST)
				act( AT_GOSSIP, buf, ch, sbuf.c_str(), vch, TO_VICT);
			else if ( CanGlobalCommunicate(vch) )
				act( AT_GOSSIP, buf, ch, sbuf.c_str(), vch, TO_VICT );

			vch->position	= position;
		}
	}

	return;
}

void to_channel( const char *argument, int channel, const char *verb, sh_int level )
{
	char buf[MAX_STRING_LENGTH];

	if ( !gTheWorld || gTheWorld->ConnectionCount() == 0 || argument[0] == '\0' )
		return;

	if ( !gGameRunning )
		return; // die out if we're not running -
	//which means no one is here to hear it anyways

	sprintf(buf, "%s: %s\r\n", verb, argument );
	//sprintf(buf, "%s: %s", verb, argument );

	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->GetOriginalCharacter();
		vch = d->GetCharacter();


		if ( !och || !vch )
			continue;

		/* Testaur:  What a mess later on!	Just handle monitor channel seperately */
		if (channel == CHANNEL_MONITOR)
		{
			if(  IS_IMMORTAL(och)
				|| (  !IS_NPC(och)
				&& och->pcdata->council
				&& is_name("monitor",och->pcdata->council->powers)
				)
				)
			{
				if(  get_trust(och) >= level
					&& d->ConnectedState == CON_PLAYING
					&& !IS_SET(och->deaf,channel)
					)
				{
					set_char_color( AT_LOG, vch );
					send_to_char( buf, vch );
				}
			}
			continue;
		}
		/* end of monitor channel... original mess follows..
		note imbalanced parenthesis inside comment...
		who knows what they were thinking? */

		/*
		 * Codetalk added by Ksilyan - moved from a comm channel
		 * to a "log" channel. Meant to be used for debugging and
		 * stuff.
		 */
		if (channel == CHANNEL_CODETALK)
		{
			if(  IS_IMMORTAL(och) )
			{
				if(  get_trust(och) >= level
					&& d->ConnectedState == CON_PLAYING
					&& !IS_SET(och->deaf,channel)
					)
				{
					set_char_color( AT_LOG, vch );
					send_to_char( buf, vch );
				}
			}
			continue;
		}

		if ( !IS_IMMORTAL(vch)
			|| ( get_trust(vch) < sysdata.build_level && channel == CHANNEL_BUILD )
			|| ( get_trust(vch) < level
			/*	|| ( get_trust(vch) < sysdata.log_level
			&& ( channel == CHANNEL_LOG || channel == CHANNEL_HIGH || channel == CHANNEL_COMM)*/ ) )
			continue;

		if ( d->ConnectedState == CON_PLAYING
			&&	!IS_SET(och->deaf, channel)
			&&	 get_trust( vch ) >= level )
		{
			set_char_color( AT_LOG, vch );
			send_to_char( buf, vch );
		}
	}

	return;
}


/*
void do_auction(CHAR_DATA *ch, const char* argument)
{
    talk_channel( ch, argument, CHANNEL_AUCTION, "auction" );
    return;
}
*/

void do_ooc(CHAR_DATA *ch, const char* argument)
{
    if ( NOT_AUTHED(ch) )
    {
        send_to_char("Huh?\r\n", ch);
        return;
    }

	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}

    talk_channel(ch, argument, CHANNEL_OOC, "ramble");
}

void do_arenachat(CHAR_DATA *ch, const char* argument) {
    talk_channel(ch, argument, CHANNEL_ARENA, "$n: ");
}

void do_chat(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}

    talk_channel( ch, argument, CHANNEL_CHAT, "proclaim" );
    return;
}

void do_bellow(CHAR_DATA *ch, const char* argument)
{
	talk_channel(ch, argument, CHANNEL_BELLOW, "bellow");
}

void do_clantalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan
    ||   ch->pcdata->clan->clan_type == CLAN_ORDER
    ||   ch->pcdata->clan->clan_type == CLAN_GUILD )
    {
	send_to_char( "Huh?\r\n", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_CLAN, "clantalk" );
    return;
}

void do_newbiechat(CHAR_DATA *ch, const char* argument)
{
	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}

    talk_channel( ch, argument, CHANNEL_NEWBIE, "newbiechat" );

    return;
}

void do_ordertalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan
         || ch->pcdata->clan->clan_type != CLAN_ORDER )
    {
	send_to_char( "Huh?\r\n", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_ORDER, "ordertalk" );
    return;
}

void do_ot(CHAR_DATA *ch, const char* argument)
{
  do_ordertalk( ch, argument );
}

void do_counciltalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\r\n", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_COUNCIL, "counciltalk" );
    return;
}

void do_guildtalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan || ch->pcdata->clan->clan_type != CLAN_GUILD )
    {
	send_to_char( "Huh?\r\n", ch );
	return;
    }
    talk_channel( ch, argument, CHANNEL_GUILD, "guildtalk" );
    return;
}

void do_music(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_MUSIC, "music" );
    return;
}


void do_quest(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}
    talk_channel( ch, argument, CHANNEL_QUEST, "quest" );
    return;
}

void do_ask(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

	if (!CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}
    talk_channel( ch, argument, CHANNEL_ASK, "ask" );
    return;
}



void do_answer(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}

    talk_channel( ch, argument, CHANNEL_ASK, "answer" );
    return;
}



void do_shout(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }

	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}
  talk_channel( ch, drunk_speech( argument, ch ).c_str(), CHANNEL_SHOUT, "shout" );
  ch->AddWait(16);
  return;
}



void do_yell(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
  talk_channel( ch, drunk_speech( argument, ch ).c_str(), CHANNEL_YELL, "yell" );
  return;
}



void do_immtalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk" );
    return;
}


void do_muse(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_HIGHGOD, "muse" );
    return;
}


void do_think(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA * vch;
	char buf[MAX_STRING_LENGTH];

    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
	/* KSILYAN
		Alright. I'm not so sure why it is think is a global channel for imms,
		since we already have a ton of those.

	talk_channel( ch, argument, CHANNEL_HIGH, "think" );
	*/
	/* begin new think code here - all imms in the room. */

	Room * inRoom = ch->GetInRoom();

	for (vch = inRoom->first_person; vch; vch = vch->next_in_room)
	{
		if (IS_IMMORTAL(vch))
		{
			sprintf(buf, "%s thinks '%s'\r\n", ch->getName().c_str(), argument);
			set_pager_color(AT_THINK, vch);
			send_to_char(buf, vch);
		}
	}

	return;
}


void do_avtalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_AVTALK, "avtalk" );
    return;
}

void do_evtalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_EVTALK, "evtalk" );
    return;
}

void do_sgtalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_SGTALK, "sgtalk" );
    return;
}


void say( Character * ch, const std::string & what, Character * target = NULL )
{
	Room * inRoom = ch->GetInRoom();

	if ( IS_SET( inRoom->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	int actflags = ch->act;

	if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );

	for ( Character * vch = inRoom->first_person; vch; vch = vch->next_in_room )
	{
		char buffer[MAX_STRING_LENGTH];
		string understand;
		string sbuf;
		bool language = false;
		bool can_understand;


		if ( vch == ch )
			continue;

		can_understand = TRUE;

		if ( !knows_language(vch, ch->speaking, ch) &&
			(!IS_NPC(ch) || ch->speaking != 0) )
		{
			sbuf = scramble(what, ch->speaking);
			can_understand = FALSE;
		}
		else
			sbuf = what;

		if ( ch->speaking != 0 )
		{
			language = TRUE;
		}

		sbuf = drunk_speech( sbuf, ch ).c_str();

		MOBtrigger = FALSE;

		if (language)
		{
			int i, count, lang;
			count = lang = 0;
			for (i = LANG_COMMON; lang_array[i] != LANG_UNKNOWN; i++)
			{
				if (ch->speaking & lang_array[i])
				{
					count++;
					lang = i;
				}
			}
			if (count == 1)
			{
				if (can_understand)
					understand = string("in ") + lang_names[lang];
				else
					understand = "in some language";

				if ( target )
					sprintf(buffer, "$n says to %s, %s, '$t'", target->getShort().c_str(), understand.c_str());
				else
					sprintf(buffer, "$n says, %s, '$t'", understand.c_str());
			}
			else
			{
				if ( target )
					sprintf(buffer, "$n says to %s '$t'", target->getShort().c_str() );
				else
					sprintf(buffer, "$n says '$t'");
			}
		}
		else
		{
			if ( target )
				sprintf(buffer, "$n says to %s '$t'", target->getShort().c_str() );
			else
				sprintf(buffer, "$n says '$t'");
		}

		act( AT_SAY, buffer, ch, sbuf.c_str(), vch, TO_VICT );
	}
	/*	  MOBtrigger = FALSE;
	act( AT_SAY, "$n says '$T'", ch, NULL, argument, TO_ROOM );*/

	/* KSILYAN- television handling */
	if (inRoom->TelevisionObjectId != 0)
	{
		Object * television = ObjectMap[inRoom->TelevisionObjectId];

		ostringstream os;

		os << ch->getShort().str() << " says ";

		if ( target )
			os << " to " << target->getShort().str() << " ";

		os << "'" << what << "'";
		television_talk( television, os.str().c_str() );
	}


	ch->act = actflags;
	MOBtrigger = FALSE;

	{
		int i, count, lang;
		char buffer[MAX_STRING_LENGTH];
		count = lang = 0;
		for (i = LANG_COMMON; lang_array[i] != LANG_UNKNOWN; i++)
		{
			if (ch->speaking & lang_array[i])
			{
				count++;
				lang = i;
			}
		}
		if (count == 1 && lang_array[lang] != LANG_COMMON)
		{
			if ( target )
				sprintf(buffer, "You say to %s, in %s, '$T'", target->getShort().c_str(), lang_names[lang]);
			else
				sprintf(buffer, "You say, in %s, '$T'", lang_names[lang]);
		}
		else
		{
			if ( target )
				sprintf(buffer, "You say to %s '$T'", target->getShort().c_str() );
			else
				sprintf(buffer, "You say '$T'");
		}
		act( AT_SAY, buffer, ch, NULL, drunk_speech( what, ch ).c_str(), TO_CHAR );
	}

	if ( IS_SET( inRoom->room_flags, ROOM_LOGSPEECH ) )
	{
		ostringstream os;

		os << ch->getShort(false).str();

		if ( target )
			os << " to " << target->getShort(false).str();

		os << ": " << what;

		append_to_file( LOG_FILE, os.str().c_str() );
	}

	mprog_speech_trigger( what.c_str(), ch );
	if ( char_died(ch) )
		return;

	oprog_speech_trigger( what.c_str(), ch );
	if ( char_died(ch) )
		return;

	rprog_speech_trigger( what.c_str(), ch );
	return;
}

// Sayto command ...
//    - Ksilyan sep-13-2004
void do_sayto(Character * ch, const char* argument)
{
	if ( string(argument) == "" )
	{
		ch->sendText("Say what to whom?\n\r");
		return;
	}

	// Look up the target
	char target[MAX_INPUT_LENGTH];
	argument = one_argument(argument, target);

	Character * victim = get_char_room( ch, target );

	if ( !victim )
	{
		ch->sendText( string("I couldn't find '") + target + "' here.\n\r");
		return;
	}

	say ( ch, argument, victim );
}

void do_say(CHAR_DATA *ch, const char* argument)
{
	if ( argument[0] == '\0' )
	{
		send_to_char( "Say what?\r\n", ch );
		return;
	}

	say( ch, argument );
}

void do_osay(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;
    int actflags;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Say what?\r\n", ch );
		return;
	}

	Room * inRoom = ch->GetInRoom();

	if ( IS_SET( inRoom->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

    actflags = ch->act;

    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );

	if ( IS_SET( ch->deaf , CHANNEL_OSAY ) )
		REMOVE_BIT( ch->deaf , CHANNEL_OSAY );

	for ( vch = inRoom->first_person; vch; vch = vch->next_in_room )
	{
		const char *sbuf = argument;

		if ( vch == ch )
			continue;

		MOBtrigger = FALSE;

		if (!IS_SET(vch->deaf, CHANNEL_OSAY))
			act( AT_SAY, "$n says (ooc) '$t'", ch, sbuf, vch, TO_VICT );
	}

    /* KSILYAN- television handling */
    if (inRoom->TelevisionObjectId != 0)
    {
		Object * television = ObjectMap[inRoom->TelevisionObjectId];

        sprintf( buf, "%s says (ooc) '%s'", ch->getShort().c_str(), argument);
        television_talk(television, buf);
    }

    ch->act = actflags;
    MOBtrigger = FALSE;

	act( AT_SAY, "You say (ooc) '$T'", ch, NULL, argument, TO_CHAR );

	if ( IS_SET( inRoom->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "%s (ooc): %s", ch->getShort(false).c_str(), argument );
		append_to_file( LOG_FILE, buf );
	}

    return;
}
void do_pray(CHAR_DATA *ch, const char* argument)
{
    if ( !argument || argument[0] == '\0' ) {
        if ( check_skill(ch, "pray", "") )
            return;
    }

    talk_channel(ch, argument, CHANNEL_PRAY, "pray");
}

void do_respond(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	Character *victim;
	int position;
	CHAR_DATA *switched_victim;

	switched_victim = NULL;

	if (!IS_NPC(ch)
		&& ( IS_SET(ch->act, PLR_SILENCE)
		||	 IS_SET(ch->act, PLR_NO_TELL) ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Respond to whom?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == NULL
		|| ( IS_NPC(victim) && victim->InRoomId != ch->InRoomId )
		|| (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_char( "You have a nice little chat with yourself.\r\n", ch );
		return;
	}

	if ( !IS_NPC( victim ) && ( victim->SwitchedCharId != 0 )
		&& ( get_trust( ch ) > LEVEL_HERO_MAX )
		&& !IS_SET( CharacterMap[victim->SwitchedCharId]->act, ACT_POLYMORPHED)
		&& !IS_AFFECTED(CharacterMap[victim->SwitchedCharId], AFF_POSSESS) )
	{
		send_to_char( "That player is switched.\r\n", ch );
		return;
	}

	else if ( !IS_NPC( victim ) && ( victim->SwitchedCharId != 0 )
		&& (IS_SET(CharacterMap[victim->SwitchedCharId]->act, ACT_POLYMORPHED)
		||	IS_AFFECTED(CharacterMap[victim->SwitchedCharId], AFF_POSSESS) ) )
		switched_victim = CharacterMap[victim->SwitchedCharId];

	else if ( !IS_NPC( victim ) && ( !victim->GetConnection() ) )
	{
		send_to_char( "That player is link-dead.\r\n", ch );
		return;
	}

	if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_AFK ) ) )
	{
		send_to_char( "That player is afk, and may not hear you.\r\n", ch );
	}

	if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_SILENCE ) ) )
	{
		send_to_char( "That player is silenced.  They will receive your message but can not respond.\r\n", ch );
	}

	if ( victim->GetConnection() != 0
		&&	 victim->GetConnection()->ConnectedState == CON_EDITING
		&&	 get_trust(ch) < sysdata.level_interrupt_buffer )
	{
		act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}


	if (switched_victim)
		victim = switched_victim;

	act( AT_PRAY, "You respond to $N '$t'", ch, argument, victim, TO_CHAR );
	position		= victim->position;
	victim->position	= POS_STANDING;
	act( AT_PRAY, "$n responds '$t'", ch, argument, victim, TO_VICT );
	victim->position	= position;

	/* can't use talk_channel for this */
	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->GetOriginalCharacter();
		vch = d->GetCharacter();

		if ( d->ConnectedState == CON_PLAYING && vch != ch && victim != vch )
		{
			if ( IS_IMMORTAL(vch) )
			{
				char buf[MAX_INPUT_LENGTH];

				sprintf(buf, "%s responds to %s '%s'\r\n",
					can_see(vch,	 ch) ? NAME(ch) 	: "Someone",
					can_see(vch, victim) ? NAME(victim) : "someone",
					argument);
				set_char_color(AT_PRAY, vch);
				send_to_char(buf, vch);
			}
		}
	}
	return;
}


/* Ksilyan: whisper
 * Allow characters to send private messages
 * inside a room.
 */

void do_whisper(CHAR_DATA * ch, const char* argument)
{
	char ToWhom[MAX_INPUT_LENGTH];
	char WhisperText[MAX_INPUT_LENGTH];
	CHAR_DATA * victim;

	argument = one_argument(argument, ToWhom);
	if (strlen(ToWhom) == 0)
	{
		set_char_color(AT_PLAIN, ch);
		send_to_char("Whisper to whom?", ch);
		return;
	}

	//argument = one_argument(argument, WhisperText);
	strcpy(WhisperText, argument);
	if (strlen(WhisperText) == 0)
	{
		set_char_color(AT_PLAIN, ch);
		send_to_char("Whisper what?", ch);
		return;
	}

	victim = get_char_room(ch, ToWhom);
	if (!victim)
	{
		set_char_color(AT_PLAIN, ch);
		send_to_char("They aren't here.", ch);
		return;
	}

	act( AT_TELL, "You whisper to $N '$t'", ch, WhisperText, victim, TO_CHAR );
	mprog_speech_trigger( WhisperText, ch );

	if ( knows_language( victim, ch->speaking, ch )
			||  (IS_NPC(ch) && !ch->speaking) )
		act( AT_TELL, "$n whispers to you '$t'", ch, WhisperText, victim, TO_VICT );
	else
		act( AT_TELL, "$n whispers to you '$t'", ch, scramble(WhisperText, ch->speaking).c_str(), victim, TO_VICT );

	act(AT_ACTION, "$n whispers something to $N.", ch, NULL, victim, TO_NOTVICT);
}

void do_tell(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int position;
	CHAR_DATA *switched_victim;

	switched_victim = NULL;

	REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
	Room * inRoom = ch->GetInRoom();

	if ( IS_SET( inRoom->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if (!IS_NPC(ch)
		&& ( IS_SET(ch->act, PLR_SILENCE)
		||	 IS_SET(ch->act, PLR_NO_TELL) ) )
	{
		send_to_char( "You can't do that.\r\n", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Tell whom what?\r\n", ch );
		return;
	}

	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}

	if ( (( victim = get_char_world( ch, arg ) ) == NULL )
		|| ( IS_NPC(victim) && victim->InRoomId != ch->InRoomId )
		|| (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_char( "You have a nice little chat with yourself.\r\n", ch );
		return;
	}

	if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim) )
	{
		send_to_char( "They can't hear you because you are not authorized.\r\n", ch);
		return;
	}

	if ( !IS_NPC( victim ) && ( victim->SwitchedCharId != 0 )
		&& ( get_trust( ch ) > LEVEL_HERO_MAX )
		&& !IS_SET( CharacterMap[victim->SwitchedCharId]->act, ACT_POLYMORPHED)
		&& !IS_AFFECTED(CharacterMap[victim->SwitchedCharId], AFF_POSSESS) )
	{
		send_to_char( "That player is switched.\r\n", ch );
		return;
	}

	else if ( !IS_NPC( victim ) && ( victim->SwitchedCharId != 0 )
		&& (IS_SET(CharacterMap[victim->SwitchedCharId]->act, ACT_POLYMORPHED)
		||	IS_AFFECTED(CharacterMap[victim->SwitchedCharId], AFF_POSSESS) ) )
		switched_victim = CharacterMap[victim->SwitchedCharId];

	else if ( !IS_NPC( victim ) && ( !victim->GetConnection() ) )
	{
		send_to_char( "That player is link-dead.\r\n", ch );
		return;
	}

	if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_AFK ) ) )
	{
		send_to_char( "That player is afk, and may not hear you.\r\n", ch );
	}

	if ( IS_SET( victim->deaf, CHANNEL_TELLS )
		&& ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
	{
		act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
			TO_CHAR );
		return;
	}

	if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_SILENCE ) ) )
	{
		send_to_char( "That player is silenced.  They will receive your message but can not respond.\r\n", ch );
	}

	if ( (!IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
		|| ( !IS_NPC(victim) && IS_SET(RoomMap[victim->InRoomId]->room_flags, ROOM_SILENCE) ) )
	{
		act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if ( victim->GetConnection()
		&&	 victim->GetConnection()->ConnectedState == CON_EDITING
		&&	 get_trust(ch) < sysdata.level_interrupt_buffer )
	{
		act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}


	if(switched_victim)
		victim = switched_victim;

	act( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );
	position		= victim->position;
	/* Bug fix by guppy@wavecomputers.net */
	MOBtrigger = FALSE;
	victim->position	= POS_STANDING;
	if ( knows_language( victim, ch->speaking, ch )
		||	(IS_NPC(ch) && !ch->speaking) )
		act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
	else
		act( AT_TELL, "$n tells you '$t'", ch, scramble(argument, ch->speaking).c_str(), victim, TO_VICT );
	MOBtrigger = TRUE;
	victim->position	= position;
	victim->ReplyToCharId = ch->GetId();
	ch->RetellToCharId = victim->GetId();
	if ( IS_SET( inRoom->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "%s: %s (tell to) %s.",
			ch->getShort(false).c_str(),
			argument,
			victim->getShort(false).c_str() );
		append_to_file( LOG_FILE, buf );
	}
	mprog_speech_trigger( argument, ch );
	return;
}



void reply( CHAR_DATA *ch, const char *argument, bool retell )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int position;
	idCharacter ReplyToCharId;

	if ( !CanGlobalCommunicate(ch) )
	{
		send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
		return;
	}

	Room * inRoom = ch->GetInRoom();

	REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
	if ( IS_SET( inRoom->room_flags, ROOM_SILENCE ) )
	{
		send_to_char( "You can't do that here.\r\n", ch );
		return;
	}

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
	{
		send_to_char( "You are silenced!\r\n", ch );
		return;
	}

	if ( retell == true )
	{
		if ( ( ReplyToCharId = ch->RetellToCharId ) == 0 )
		{
			ch->sendText("You have nobody to retell to.\r\n");
			return;
		}
	}
	else
	{
		if ( ( ReplyToCharId = ch->ReplyToCharId ) == 0 )
		{
			ch->sendText("You have nobody to reply to.\r\n");
			return;
		}
	}

	if ( ( victim = CharacterMap[ReplyToCharId] ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
                if (retell) {
                    ch->RetellToCharId = 0;
                }
                else {
		    ch->ReplyToCharId = 0;
                }
		return;
	}

	if ( !IS_NPC( victim ) && ( victim->SwitchedCharId != 0 )
		&& can_see( ch, victim ) && ( get_trust( ch ) > LEVEL_HERO_MAX ) )
	{
		send_to_char( "That player is switched.\r\n", ch );
		return;
	}
	else if ( !IS_NPC( victim ) && ( !victim->GetConnection() ) )
	{
		send_to_char( "That player is link-dead.\r\n", ch );
		return;
	}

	if ( !IS_NPC (victim) && ( IS_SET (victim->act, PLR_AFK ) ) )
	{
		send_to_char( "That player is afk, and may not hear your message.\r\n", ch );
	}

	if ( IS_SET( victim->deaf, CHANNEL_TELLS )
		&& ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
	{
		act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
			TO_CHAR );
		return;
	}

	if ( ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
		|| ( !IS_NPC(victim) && IS_SET( RoomMap[victim->InRoomId]->room_flags, ROOM_SILENCE ) ) )
	{
		act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}
	/* Bug fix by guppy@wavecomputers.net */
	MOBtrigger = FALSE;
	act( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );
	position		= victim->position;
	victim->position	= POS_STANDING;
	if ( knows_language( victim, ch->speaking, ch ) ||
		(IS_NPC(ch) && !ch->speaking) )
		act( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
	else
		act( AT_TELL, "$n tells you '$t'", ch, scramble(argument, ch->speaking).c_str(), victim, TO_VICT );
	MOBtrigger = TRUE;
	victim->position = position;
	victim->ReplyToCharId = ch->GetId();
	ch->RetellToCharId = victim->GetId();
	if ( IS_SET( inRoom->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "%s: %s (reply to) %s.",
			ch->getShort(false).c_str(), argument,
			victim->getShort(false).c_str() );
		append_to_file( LOG_FILE, buf );
	}

	return;
}

void do_reply(CHAR_DATA *ch, const char* argument)
{
    reply(ch, argument, false);
}

/* Retell: Sends a reply to the last person a tell was sent to.
 * - Sap
 */
void do_retell(CHAR_DATA *ch, const char* argument)
{
    reply(ch, argument, true);
}

void do_emote(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	const char *plast;
	CHAR_DATA *vch;
	int actflags;

	Room * inRoom = ch->GetInRoom();

	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
	{
		send_to_char( "You can't show your emotions.\r\n", ch );
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "Emote what?\r\n", ch );
		return;
	}

	actflags = ch->act;

	if ( IS_NPC( ch ) )
		REMOVE_BIT( ch->act, ACT_SECRETIVE );

	for ( plast = argument; *plast != '\0'; plast++ )
		;

	strcpy( buf, argument );
	if	( isalpha(plast[-1]) )
		strcat( buf, "." );
	for ( vch = inRoom->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = buf;
		/*
		if	( !knows_language( vch, ch->speaking, ch ) &&
			(!IS_NPC(ch) && ch->speaking != 0) )
			sbuf = scramble(buf, ch->speaking);
		*/
		MOBtrigger = FALSE; /* Testaur: emotes don't trigger progs */
		act( AT_ACTION, "$n $t", ch, sbuf, vch, (vch == ch ? TO_CHAR : TO_VICT) );
	}
	/*
	MOBtrigger = FALSE;
	act( AT_ACTION, "$n $T", ch, NULL, buf, TO_ROOM );
	MOBtrigger = FALSE;
	act( AT_ACTION, "$n $T", ch, NULL, buf, TO_CHAR );
	*/
	ch->act = actflags;
	if ( IS_SET( inRoom->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf( buf, "%s %s (emote)", ch->getShort(false).c_str(),
			argument );
		append_to_file( LOG_FILE, buf );
	}
	return;
}

/* New bug, idea and typo commands, allow you to view the files from in-game -Zoie */
void do_bug(CHAR_DATA * ch, const char *argument)
{
   	char buf[MAX_STRING_LENGTH];
   	char arg[MAX_INPUT_LENGTH];
   	const char *argument1;
//   	struct tm *t = localtime( &current_time );
   	struct tm *t = localtime( &secCurrentTime );

   	argument1 = one_argument(argument, arg);

   	if (arg[0] == '\0')
   	{
   		set_char_color( AT_PLAIN, ch );
      	send_to_char("Usage: bug <message>\n\r", ch);
      	send_to_char("Usage: bug <# of entries>\n\r", ch);
      	return;
   	}

   	if( isdigit( arg[0] ) || !str_cmp( arg, "today" ) ) /* View list instead of adding to it */
   	{
   		if( isdigit( arg[0] ) && ( atoi(arg) < 1 || atoi(arg) > 300 ) ) /* View list instead of adding to it */
      	{
      		send_to_char("Invalid range, must be between 1 and 300.\n\r", ch);
      		return;
		}
   		else
   		{
   			send_to_char("&w&RVnum       Date    Name       Bug Message\n\r&c&w---------------------------------------------------------------------------\n\r", ch);
      		if (!str_cmp(arg, "today"))
        	 	read_last_file(ch, -2, NULL, 1);
      		else
        	 	read_last_file(ch, atoi(arg), NULL, 1);
      		return;
		}
   	}

    sprintf( buf, "[%8s] %-2.2d/%-2.2d %-12s %s", ch->GetInRoom() ? vnum_to_dotted( ch->GetInRoom()->vnum ) : 0, t->tm_mon + 1, t->tm_mday, ch->getName().c_str(), argument );
    write_last_file( buf, 1 );
   	send_to_char( "Thanks! your bug notice has been recorded.\r\n", ch );
   	return;
}

void do_idea(CHAR_DATA * ch, const char *argument)
{
   	char buf[MAX_STRING_LENGTH];
   	char arg[MAX_INPUT_LENGTH];
   	const char *argument1;
//   	struct tm *t = localtime( &current_time );
   	struct tm *t = localtime( &secCurrentTime );

   	argument1 = one_argument(argument, arg);

   	if (arg[0] == '\0')
   	{
   		set_char_color( AT_PLAIN, ch );
      	send_to_char("Usage: idea <message>\n\r", ch);
      	send_to_char("Usage: idea <# of entries OR \'-1\' for all entries>\n\r", ch);
      	return;
   	}

   	if( isdigit( arg[0] ) || !str_cmp( arg, "today" ) ) /* View list instead of adding to it */
   	{
   		if( isdigit( arg[0] ) && ( atoi(arg) < 1 || atoi(arg) > 300 ) ) /* View list instead of adding to it */
      	{
      		send_to_char("Invalid range, must be between 1 and 300.\n\r", ch);
      		return;
		}
   		else
   		{
   			send_to_char("&w&RDate    Name         Idea Message\n\r&c&w---------------------------------------------------------------------------\n\r", ch);
      		if (!str_cmp(arg, "today"))
        	 	read_last_file(ch, -2, NULL, 2);
      		else
        	 	read_last_file(ch, atoi(arg), NULL, 2);
      		return;
		}
   	}

    sprintf( buf, "%-2.2d/%-2.2d   %-12s %s", t->tm_mon + 1, t->tm_mday, ch->getName().c_str(), argument );
    write_last_file( buf, 2 );
   	send_to_char( "Thanks for your idea contribution!\r\n", ch );
   	return;
}

void do_typo(CHAR_DATA * ch, const char *argument)
{
   	char buf[MAX_STRING_LENGTH];
   	char arg[MAX_INPUT_LENGTH];
   	const char *argument1;
//   	struct tm *t = localtime( &current_time );
   	struct tm *t = localtime( &secCurrentTime );

   	argument1 = one_argument(argument, arg);

   	if (arg[0] == '\0')
   	{
   		set_char_color( AT_PLAIN, ch );
      	send_to_char("Usage: typo <message>\n\r", ch);
      	send_to_char("Usage: typo <# of entries OR \'-1\' for all entries>\n\r", ch);
      	return;
   	}

   	if( isdigit( arg[0] ) || !str_cmp( arg, "today" ) ) /* View list instead of adding to it */
   	{
   		if( isdigit( arg[0] ) && ( atoi(arg) < 1 || atoi(arg) > 300 ) ) /* View list instead of adding to it */
      	{
      		send_to_char("Invalid range, must be between 1 and 300.\n\r", ch);
      		return;
		}
   		else
		{
	   		send_to_char("&w&RVnum       Date    Name       Typo Message\n\r&c&w---------------------------------------------------------------------------\n\r", ch);
	      	if (!str_cmp(arg, "today"))
	         	read_last_file(ch, -2, NULL, 3);
	      	else
	         	read_last_file(ch, atoi(arg), NULL, 3);
	      	return;
		}
   	}

    sprintf( buf, "[%8s] %-2.2d/%-2.2d %-12s %s", ch->GetInRoom() ? vnum_to_dotted( ch->GetInRoom()->vnum ) : 0, t->tm_mon + 1, t->tm_mday, ch->getName().c_str(), argument );
    write_last_file( buf, 3 );
   	send_to_char( "Thanks! Your typo notice has been recorded.\r\n", ch );
   	return;
}


void do_ide(CHAR_DATA *ch, const char* argument)
{
    send_to_char("If you want to send an idea, type 'idea <message>'.\r\n", ch);
    send_to_char("If you want to identify an object and have the identify spell,\r\n", ch);
    send_to_char("Type 'cast identify <object>'.\r\n", ch);
    return;
}

/* Old bug/idea/typo commands -Zoie
void do_bug(CHAR_DATA *ch, const char* argument)
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Ok.  Thanks.\r\n", ch );
    return;
}

void do_idea(CHAR_DATA *ch, const char* argument)
{
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Thanks for the idea, we'll contact you if we have any questions.\r\n", ch );
    return;
}

void do_typo(CHAR_DATA *ch, const char* argument)
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Thanks!\r\n", ch );
    return;
}
*/


/* Ksilyan
 * Command that allows a player to knock on a "door",
 * with a knock message appearing on the other side.
 */
void do_knock(CHAR_DATA * ch, const char* argument)
{
	ExitData * exit;
	ROOM_INDEX_DATA * toRoom;
	ROOM_INDEX_DATA * fromRoom;

	exit = find_door(ch, argument, TRUE);

	if ( !exit || !IS_SET(exit->exit_info, EX_ISDOOR) )
	{
		send_to_char( "There is no door to knock on.\r\n", ch);
		return;
	}
	if ( IS_SET(exit->exit_info, EX_SECRET) )
	{
		send_to_char( "There is no door to knock on.\r\n", ch);
		return;
	}

	act( AT_ACTION, "You knock on the door.", ch, NULL, NULL, TO_CHAR);
	act( AT_ACTION, "$n knocks on the door.", ch, NULL, NULL, TO_ROOM);

	fromRoom = ch->GetInRoom();
	toRoom = exit->to_room;
	char_from_room(ch);
	char_to_room(ch, toRoom);
	act( AT_ACTION, "Someone knocks on the door.", ch, NULL, NULL, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, fromRoom);
}



void do_rent(CHAR_DATA *ch, const char* argument)
{
    set_char_color( AT_WHITE, ch );
    send_to_char( "There is no rent here.  Just save and quit.\r\n", ch );
    return;
}



void do_qui(CHAR_DATA *ch, const char* argument)
{
    set_char_color( AT_RED, ch );
    send_to_char( "If you want to QUIT, you have to spell it out.\r\n", ch );
    return;
}

void do_quit(Character *ch, const char* argument)
{
	int x, y;
	int level;

	if ( IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
	{
		send_to_char("You can't quit while polymorphed.\r\n", ch);
		return;
	}

	if ( IS_NPC(ch) )
		return;

	if ( ch->position == POS_FIGHTING )
	{
		set_char_color( AT_RED, ch );
		send_to_char( "No way! You are fighting.\r\n", ch );
		return;
	}

	if ( ch->position  < POS_STUNNED  )
	{
		set_char_color( AT_BLOOD, ch );
		send_to_char( "You're not DEAD yet.\r\n", ch );
		return;
	}

	if ( get_timer(ch, TIMER_RECENTFIGHT) > 0
		&&	!IS_IMMORTAL(ch)
		&&	strcmp(argument, "forcemetoquit") )
	{
		set_char_color( AT_RED, ch );
		send_to_char( "Your adrenaline is pumping too hard to quit now!\r\n", ch );
		return;
	}
/**********  logon wait time *****************
	if ( get_timer(ch, TIMER_LOGON) > 0 &&
	     !IS_IMMORTAL(ch)               &&
	     strcmp( argument, "forcemetoquit" ) )
	{
		set_char_color( AT_YELLOW, ch );
		send_to_char( "Fast character switching has been disabled to prevent multiplaying. Please wait 60 seconds.\r\n", ch );
		return;
	}
********************************************/

	if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller) ) )
	{
		send_to_char("Wait until you have bought/sold the item on auction.\r\n", ch);
		return;
	}
	//set_char_color( AT_WHITE, ch );

	/* changes to save mount to pfile - fiz */

	if	( ch->GetMount() )
	{
		send_to_char("Dismount first.\n", ch);
		return; /* Yd */
	}

	act( AT_BYE, "Your surroundings begin to fade as a mystical swirling vortex of colors envelops your body...\r\n", ch, NULL, NULL, TO_CHAR );
	act( AT_BYE, "When you come to, things are not as they were.\r\n", ch , NULL, NULL, TO_CHAR );
	act( AT_SAY, "A strange voice says, 'We await your return, $n...'\r\n", ch, NULL, NULL, TO_CHAR );
	act( AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_ROOM );
	set_char_color( AT_GREY, ch);

	if ( ch->GetConnection() )
	{
		if ( ch->GetConnection()->Account )
			sprintf( log_buf, "%s has quit. (account: %s)", ch->getName().c_str(),
					ch->GetConnection()->Account->email );
	}
	else
	{
		sprintf( log_buf, "%s has quit (was link-dead).", ch->getName().c_str() );
	}
	quitting_char = ch;

	// Get rid of any spells on the character.
	// extract_char takes care of the spells
	// that character has cast.
	list<SpellMemory*>::iterator itor;

	// Create a local copy of the list to loop through.
	list<SpellMemory*> localList = ch->spellMemory_;
	for (itor = localList.begin(); itor != localList.end(); itor++)
	{
		SpellMemory * spell = *itor;

		// If the spell is on me, but I'm not the caster,
		// then get rid of it.
		if ( spell->Target == ch && ch != spell->Caster && !IS_IMMORTAL(spell->Caster) )
		{
			if ( ch->IsAffected(spell->Spell) )
			{
				affect_remove( ch, spell->Spell );
			}
			spell->Target->spellMemory_.remove(spell);
			spell->Caster->spellMemory_.remove(spell);
		}
	}

	save_char_obj( ch );

	if ( ch->GetConnection() )
	{
		ch->GetConnection()->WriteMainMenu();

		ch->GetConnection()->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
		ch->GetConnection()->SetInputReceiver(gConnectionManager);
		ch->GetConnection()->CurrentCharId = 0;
		ch->SetConnection(NULL);
	}

	/* get rid of the mount mob - fiz */
	if	( ch->GetMount() )
		extract_char( ch->GetMount(), TRUE );

	saving_char = NULL;

	level = get_trust(ch);
	/*
	* After extract_char the ch is no longer valid!
	*/
	extract_char( ch, TRUE );
	for ( x = 0; x < MAX_WEAR; x++ )
	{
		for ( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;
	}

	/* don't show who's logging off to leaving player */
	/*
	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", level );
	*/
	log_string_plus( log_buf, LOG_COMM, level );
	return;
}


void send_rip_screen( Character *ch )
{
	FILE *rpfile;
	int num=0;
	char BUFF[MAX_STRING_LENGTH*2];

	if ((rpfile = fopen(RIPSCREEN_FILE,"r")) !=NULL) {
		while ((BUFF[num]=fgetc(rpfile)) != EOF)
			num++;
		fclose(rpfile);
		BUFF[num] = 0;
		ch->GetConnection()->SendText(BUFF, num);
	}
}

void send_rip_title( Character *ch )
{
	FILE *rpfile;
	int num=0;
	char BUFF[MAX_STRING_LENGTH*2];

	if ((rpfile = fopen(RIPTITLE_FILE,"r")) !=NULL) {
		while ((BUFF[num]=fgetc(rpfile)) != EOF)
			num++;
		fclose(rpfile);
		BUFF[num] = 0;
		ch->GetConnection()->SendText(BUFF, num);
	}
}

void send_ansi_title( CHAR_DATA *ch )
{
	FILE *rpfile;
	int num=0;
	char BUFF[MAX_STRING_LENGTH*2];

	if ((rpfile = fopen(ANSITITLE_FILE,"r")) !=NULL) {
		while ((BUFF[num]=fgetc(rpfile)) != EOF)
			num++;
		fclose(rpfile);
		BUFF[num] = 0;
		ch->GetConnection()->SendText(BUFF, num);
	}
}

void send_ascii_title( CHAR_DATA *ch )
{
	FILE *rpfile;
	int num=0;
	char BUFF[MAX_STRING_LENGTH];

	if ((rpfile = fopen(ASCTITLE_FILE,"r")) !=NULL) {
		while ((BUFF[num]=fgetc(rpfile)) != EOF)
			num++;
		fclose(rpfile);
		BUFF[num] = 0;
		ch->GetConnection()->SendText(BUFF, num);
	}
}

void do_omenu(CHAR_DATA *ch, const char* argument)
{
	send_to_char("Deprecated.\r\n", ch); // Ksilyan
#if 0
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: omenu <object> [page]  \r\n",     ch );
        send_to_char( "      Where:    <object> is a prototype object  \r\n",     ch );
        send_to_char( "            and  <page>  is an optional letter to select menu-pages\r\n",     ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    /* can redit or something */

    ch->inter_type = OBJ_TYPE;
    ch->inter_substate = SUB_NORTH;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(obj->pIndexData->name);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  obj->pIndexData->vnum;
    send_obj_page_to_char(ch, obj->pIndexData, arg2[0]);
#endif
}


void do_rmenu(CHAR_DATA *ch, const char* argument)
{
	send_to_char("Deprecated.\r\n", ch); // Ksilyan
#if 0
    ROOM_INDEX_DATA *idx;
    char arg1[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

    idx = ch->in_room;
    /* can redit or something */

    ch->inter_type = ROOM_TYPE;
    ch->inter_substate = SUB_NORTH;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(idx->name);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  idx->vnum;
    send_room_page_to_char(ch, idx, arg1[0]);
#endif
}

void do_cmenu(CHAR_DATA *ch, const char* argument)
{
	send_to_char("Deprecated.\r\n", ch); // Ksilyan
#if 0
    char arg1[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

    ch->inter_type = CONTROL_TYPE;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup("Control Panel");
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    send_control_page_to_char(ch, arg1[0]);
#endif
}


void do_mmenu(CHAR_DATA *ch, const char* argument)
{
	send_to_char("Deprecated.\r\n", ch); // Ksilyan
#if 0
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: mmenu <victim> [page]  \r\n",     ch );
        send_to_char( "      Where:    <victim> is a prototype mob  \r\n",     ch );
        send_to_char( "            and  <page>  is an optional letter to select menu-pages\r\n",     ch );
        return;
    }


    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\r\n", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	send_to_char( "Not on players.\r\n", ch );
	return;
    }

    if ( get_trust( ch ) < victim->level )
    {
	set_char_color( AT_IMMORT, ch );
	send_to_char( "Their godly glow prevents you from getting a good look .\r\n", ch );
	return;
    }
    ch->inter_type = MOB_TYPE;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(arg1);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  victim->pIndexData->vnum;
    send_page_to_char(ch, victim->pIndexData, arg2[0]);
#endif
}


void do_rip(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Rip ON or OFF?\r\n", ch );
	return;
    }
    if ( (strcmp(arg,"on")==0) || (strcmp(arg,"ON") == 0) ) {
	send_rip_screen(ch);
	SET_BIT(ch->act,PLR_RIP);
	SET_BIT(ch->act,PLR_ANSI);
	return;
    }

    if ( (strcmp(arg,"off")==0) || (strcmp(arg,"OFF") == 0) ) {
	REMOVE_BIT(ch->act,PLR_RIP);
	send_to_char( "!|*\r\nRIP now off...\r\n", ch );
	return;
    }
}

void do_ansi(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "ANSI ON or OFF?\r\n", ch );
	return;
    }
    if ( (strcmp(arg,"on")==0) || (strcmp(arg,"ON") == 0) ) {
	SET_BIT(ch->act,PLR_ANSI);
	set_char_color( AT_WHITE + AT_BLINK, ch);
	send_to_char( "ANSI ON!!!\r\n", ch);
	return;
    }

    if ( (strcmp(arg,"off")==0) || (strcmp(arg,"OFF") == 0) ) {
	REMOVE_BIT(ch->act,PLR_ANSI);
	send_to_char( "Okay... ANSI support is now off\r\n", ch );
	return;
    }
}

void do_save(CHAR_DATA *ch, const char* argument)
{
    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_POLYMORPHED))
    {
      send_to_char("You can't save while polymorphed.\r\n", ch);
      return;
    }

    if ( IS_NPC(ch) )
	return;


    if ( !IS_SET( ch->affected_by, race_table[ch->race].affected ) )
	SET_BIT( ch->affected_by, race_table[ch->race].affected );
    if ( !IS_SET( ch->resistant, race_table[ch->race].resist ) )
	SET_BIT( ch->resistant, race_table[ch->race].resist );
    if ( !IS_SET( ch->susceptible, race_table[ch->race].suscept ) )
	SET_BIT( ch->susceptible, race_table[ch->race].suscept );

    if ( ch->pcdata->deity )
    {
	if ( !IS_SET( ch->affected_by, ch->pcdata->deity->affected ) )
	   SET_BIT( ch->affected_by, ch->pcdata->deity->affected );
	if ( !IS_SET( ch->resistant, ch->pcdata->deity->element ) )
	   SET_BIT( ch->resistant, ch->pcdata->deity->element );
	if ( !IS_SET( ch->susceptible, ch->pcdata->deity->suscept ) )
	   SET_BIT( ch->susceptible, ch->pcdata->deity->suscept );
    }
    save_char_obj( ch );
    saving_char = NULL;
    send_to_char( "Ok.\r\n", ch );
    return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim )
{
	Character * tmp;

	for ( tmp = victim; tmp; tmp = CharacterMap[tmp->MasterId] )
		if ( tmp == ch )
			return true;

	return false;
}


void do_follow(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Follow whom?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

#if 0
	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master )
	{
		act( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
		return;
	}
#endif

	if ( victim == ch )
	{
		if ( ch->GetMaster() == NULL )
		{
			send_to_char( "You already follow yourself.\r\n", ch );
			return;
		}
		stop_follower( ch );
		return;
	}

#if 0
	if ( ( ch->level - victim->level < -10 || ch->level - victim->level >  10 )
		&&	 !IS_HERO(ch) )
	{
		send_to_char( "You are not of the right caliber to follow.\r\n", ch );
		return;
	}
#endif

	if ( circle_follow( ch, victim ) )
	{
		send_to_char( "Following in loops is not allowed... sorry.\r\n", ch );
		return;
	}

	if ( ch->MasterId != 0 )
	{
		stop_follower( ch );
	}

	add_follower( ch, victim );
	return;
}



void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
	if ( ch->MasterId != 0 )
	{
		bug( "Add_follower: non-null master.", 0 );
		return;
	}

	ch->MasterId = master->GetId();
	ch->LeaderId = 0;

	if ( can_see( master, ch ) )
		act( AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT );

	act( AT_ACTION, "You now follow $N.",  ch, NULL, master, TO_CHAR );

	return;
}



void stop_follower( CHAR_DATA *ch )
{
	if ( !ch )
	{
		bug( "Stop_follower: null ch.", 0 );
		return;
	}

	Character * master;
	master = CharacterMap[ch->MasterId];

	if ( !master )
	{
		bug( "Stop_follower: null master.", 0 );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_CHARM) )
	{
		REMOVE_BIT( ch->affected_by, AFF_CHARM );
		affect_strip( ch, gsn_charm_person );
	}

	if ( can_see( master, ch ) )
		act( AT_ACTION, "$n stops following you.",	   ch, NULL, master, TO_VICT	);

	act( AT_ACTION, "You stop following $N.",	   ch, NULL, master, TO_CHAR	);

	ch->MasterId = 0;
	ch->LeaderId = 0;
	return;
}



void die_follower( CHAR_DATA *ch )
{
	CHAR_DATA *fch;
	ch->LeaderId = 0;

	for ( fch = first_char; fch; fch = fch->next )
	{
		if ( ch->MasterId == fch->GetId() )
		{
			stop_follower( ch );
			ch->MasterId = 0;
		}

		if ( fch->MasterId == ch->GetId() )
			stop_follower( fch );
		if ( fch->LeaderId == ch->GetId() )
			fch->LeaderId = fch->GetId();
	}
	return;
}



void do_order(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char argbuf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found;
	bool fAll;

	strcpy( argbuf, argument );
	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Order whom to do what?\r\n", ch );
		return;
	}

	if ( IS_AFFECTED( ch, AFF_CHARM ) )
	{
		send_to_char( "You feel like taking, not giving, orders.\r\n", ch );
		return;
	}

	if ( !str_cmp( arg, "all" ) )
	{
		fAll   = TRUE;
		victim = NULL;
	}
	else
	{
		fAll   = FALSE;
		if ( ( victim = get_char_room( ch, arg ) ) == NULL )
		{
			send_to_char( "They aren't here.\r\n", ch );
			return;
		}

		if ( victim == ch )
		{
			send_to_char( "Aye aye, right away!\r\n", ch );
			return;
		}

		if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->MasterId != ch->GetId() )
		{
			send_to_char( "Do it yourself!\r\n", ch );
			return;
		}
	}

	found = FALSE;
	Room * inRoom;
	inRoom = RoomMap[ch->InRoomId];
	if ( !inRoom )
	{
		gTheWorld->LogBugString(
				string("do_order: ch's InRoom reference was invalid! Char: ") + ch->getName().str() );
		return;
	}

	for ( och = inRoom->first_person; och; och = och_next )
	{
		och_next = och->next_in_room;

		if ( IS_AFFECTED(och, AFF_CHARM)
			&&	 och->MasterId == ch->GetId()
			&& ( fAll || och == victim ) )
		{
			found = TRUE;
			act( AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT );
			if ( !(argument[0] == 'm' && argument[1] == 'p') )
				interpret( och, argument );
		}
	}

	if ( found )
	{
		sprintf( log_buf, "%s: order %s.", ch->getName().c_str(), argbuf );
		log_string_plus( log_buf, LOG_NORMAL, ch->level );

		send_to_char( "Ok.\r\n", ch );
		ch->AddWait(12);
	}
	else
		send_to_char( "You have no followers here.\r\n", ch );
	return;
}

/*
char *itoa(int foo)
{
  static char bar[256];

  sprintf(bar,"%d",foo);
  return(bar);

}
*/

void do_group(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim = NULL;

	one_argument( argument, arg );

	Room * inRoom = ch->GetInRoom();

	if ( arg[0] == '\0' )
	{
		CHAR_DATA * gch;
		CHAR_DATA * leader;

		leader = ch->LeaderId != 0 ? CharacterMap[ch->LeaderId] : ch;
		if ( !leader )
		{
			gTheWorld->LogBugString(string("do_group: character had invalid LeaderId! Ch: ") + ch->getName().c_str() );
			ch->sendText("A critical error has occured. Please quit and reconnect.\r\n");
			return;
		}

		set_char_color( AT_GREEN, ch );
		ch_printf( ch, "%s's group:\r\n", PERS(leader, ch) );

		/* Changed so that no info revealed on possess */
		for ( gch = first_char; gch; gch = gch->next )
		{
			if ( is_same_group( gch, ch ) )
			{
				set_char_color( AT_DGREEN, ch );
				if (IS_AFFECTED(gch, AFF_POSSESS))
					ch_printf( ch,
					"[%2d %s] %-16s %4s/%4s hp %4s/%4s %s %4s/%4s mv %5s xp\r\n",
					gch->level,
					IS_NPC(gch) ? "Mob" : class_table[gch->Class]->whoName_.c_str(),
					capitalize( PERS(gch, ch) ),
					"????",
					"????",
					"????",
					"????",
					IS_VAMPIRE(gch) ? "bp" : "mana",
					"????",
					"????",
					"?????"    );

				else
					ch_printf( ch,
					"[%2d %s] %-16s %4d/%4d hp %4d/%4d %s %4d/%4d mv %5d xp\r\n",
					gch->level,
					IS_NPC(gch) ? "Mob" : class_table[gch->Class]->whoName_.c_str(),
					capitalize( PERS(gch, ch) ),
					gch->hit,
					gch->max_hit,
					IS_VAMPIRE(gch) ? gch->pcdata->condition[COND_BLOODTHIRST]
					: gch->mana,
					IS_VAMPIRE(gch) ? 10 + gch->level : gch->max_mana,
					IS_VAMPIRE(gch) ? "bp" : "mana",
					gch->move,
					gch->max_move,
					gch->exp	);
			}
		}
		return;
	}

	if ( !strcmp( arg, "disband" ))
	{
		CHAR_DATA *gch;
		int count = 0;

		if ( ch->LeaderId != 0 || ch->MasterId != 0 )
		{
			send_to_char( "You cannot disband a group if you're following someone.\r\n", ch );
			return;
		}

		for ( gch = first_char; gch; gch = gch->next )
		{
			if ( is_same_group( ch, gch )
				&& ( ch != gch ) )
			{
				gch->LeaderId = 0;
				gch->MasterId = 0;
				count++;
				send_to_char( "Your group is disbanded.\r\n", gch );
			}
		}

		if ( count == 0 )
			send_to_char( "You have no group members to disband.\r\n", ch );
		else
			send_to_char( "You disband your group.\r\n", ch );

		return;
	}

	if ( !strcmp( arg, "all" ) )
	{
		CHAR_DATA *rch;
		int count = 0;

		for ( rch = inRoom->first_person; rch; rch = rch->next_in_room )
		{
			if ( ch != rch
				&&	 !IS_NPC( rch )
				&&	 can_see( ch, rch )
				&&	 rch->MasterId == ch->GetId()
				&&	 ch->MasterId == 0
				&&	 ch->LeaderId == 0
				/*		   &&	abs( ch->level - rch->level ) < 8 */
				&&	 !is_same_group( rch, ch )
				)
			{
				rch->LeaderId = ch->GetId();
				count++;
			}
		}

		if ( count == 0 )
			send_to_char( "You have no eligible group members.\r\n", ch );
		else
		{
			act( AT_ACTION, "$n groups $s followers.", ch, NULL, victim, TO_ROOM );
			send_to_char( "You group your followers.\r\n", ch );
		}
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\r\n", ch );
		return;
	}

	if ( ch->MasterId != 0 || ( ch->LeaderId != 0 && ch->LeaderId != ch->GetId() ) )
	{
		send_to_char( "But you are following someone else!\r\n", ch );
		return;
	}

	if ( victim->MasterId != ch->GetId() && ch != victim )
	{
		act( AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( is_same_group( victim, ch ) && ch != victim )
	{
		victim->LeaderId = 0;
		act( AT_ACTION, "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
		act( AT_ACTION, "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
		act( AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
		return;
	}

	/*
	if ( ch->level - victim->level < -7
	||	 ch->level - victim->level >  8)
	{
	act( AT_PLAIN, "$N cannot join $n's group.",	 ch, NULL, victim, TO_NOTVICT );
	act( AT_PLAIN, "You cannot join $n's group.",	 ch, NULL, victim, TO_VICT	  );
	act( AT_PLAIN, "$N cannot join your group.",	 ch, NULL, victim, TO_CHAR	  );
	return;
	}
	*/
	victim->LeaderId = ch->GetId();
	act( AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
	act( AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT	 );
	act( AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR	 );
	return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
 void do_split(CHAR_DATA *ch, const char* argument)
 {
	 char buf[MAX_STRING_LENGTH];
	 char arg[MAX_INPUT_LENGTH];
	 CHAR_DATA *gch;
	 int members;
	 int amount;
	 int share;
	 int extra;

	 Room * inRoom = ch->GetInRoom();

	 one_argument( argument, arg );

	 if ( arg[0] == '\0' )
	 {
		 send_to_char( "Split how much?\r\n", ch );
		 return;
	 }

	 amount = atoi( arg );

	 if ( amount < 0 )
	 {
		 send_to_char( "Your group wouldn't like that.\r\n", ch );
		 return;
	 }

	 if ( amount == 0 )
	 {
		 send_to_char( "You hand out zero coins, but no one notices.\r\n", ch );
		 return;
	 }

	 if ( ch->gold < amount )
	 {
		 send_to_char( "You don't have that much gold.\r\n", ch );
		 return;
	 }

	 members = 0;
	 for ( gch = inRoom->first_person; gch; gch = gch->next_in_room )
	 {
		 if ( is_same_group( gch, ch ) )
			 members++;
	 }


	 if ( ( IS_SET(ch->act, PLR_AUTOGOLD) ) && (members < 2))
		 return; // don't display a message if it's auto-split

	 if ( members < 2 )
	 {
		 send_to_char( "Just keep it all.\r\n", ch );
		 return;
	 }

	 share = amount / members;
	 extra = amount % members;

	 if ( share == 0 )
	 {
		 send_to_char( "Don't even bother, cheapskate.\r\n", ch );
		 return;
	 }

	 ch->gold -= amount;
	 ch->gold += share + extra;

	 set_char_color( AT_GOLD, ch );
	 ch_printf( ch,
		 "You split %d gold coins.  Your share is %d gold coins.\r\n",
		 amount, share + extra );

	 sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
		 amount, share );

	 for ( gch = inRoom->first_person; gch; gch = gch->next_in_room )
	 {
		 if ( gch != ch && is_same_group( gch, ch ) )
		 {
			 act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
			 gch->gold += share;
		 }
	 }
	 return;
 }



void do_gtell(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *gch;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Tell your group what?\r\n", ch );
		return;
	}

	if ( IS_SET( ch->act, PLR_NO_TELL ) )
	{
		send_to_char( "Your message didn't get through!\r\n", ch );
		return;
	}

	/*
	 * Note use of send_to_char, so gtell works on sleepers.
	 */
	for ( gch = first_char; gch; gch = gch->next )
	{
		if ( is_same_group( gch, ch ) )
		{
			set_char_color( AT_GTELL, gch );
			/* Groups unscrambled regardless of clan language.  Other languages
			   still garble though. -- Altrag */
			if ( knows_language( gch, ch->speaking, gch )
					||  (IS_NPC(ch) && !ch->speaking) )
			{
				ch_printf( gch, "%s tells the group '%s'.\r\n",
						ch->getShort().c_str(), argument );
			}
			else
			{
				ch_printf( gch, "%s tells the group '%s'.\r\n",
						ch->getShort().c_str(),
						scramble(argument, ch->speaking).c_str() );
			}
		}
	}

	return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->LeaderId != 0 ) ach = CharacterMap[ach->LeaderId];
    if ( bch->LeaderId != 0) bch = CharacterMap[bch->LeaderId];
    return ach == bch;
}

/*
 * This func sends the specified text to all rooms flagged with ROOM_AUCTION
 * but only if there is an auction.
 * 08.08.2000 -- Warp
 */

void talk_auction (char *argument)
{
	ROOM_INDEX_DATA *oldRoom;
	AUCTION_ROOM *auc;
	CHAR_DATA *ch, *vch;
	int bSentToSeller = FALSE;

	ch = auction->seller;

	if(auction->item != NULL) {
		oldRoom = RoomMap[auction->seller->InRoomId];

		for(auc = first_auctionroom; auc; auc = auc->next)
		{
			for(vch = auc->room->first_person; vch; vch = vch->next_in_room)
			{
				if( (ch == vch && bSentToSeller) || IS_NPC(vch) )
					continue;
				else
					bSentToSeller = TRUE;

				if( vch->GetConnection() )
				{
					set_char_color(AT_PLAIN, vch);
					vch->sendText(argument, false);
				}
			}
		}
	}
	else
		bug("talk_auction: No ongoing auctions! buf: %s", argument);
}

/*
 * Language support functions. -- Altrag
 * 07/01/96
 */
bool knows_language( CHAR_DATA *ch, int language, CHAR_DATA *cch )
{
	sh_int sn;

	if ( !IS_NPC(ch) && IS_IMMORTAL(ch) )
		return TRUE;
	if ( IS_NPC(ch) && !ch->speaks ) /* No langs = knows all for npcs */
		return TRUE;
	if ( IS_NPC(ch) && IS_SET(ch->speaks, (language & ~LANG_CLAN)) )
		return TRUE;
	/* everyone KNOWS common tongue */
	if ( IS_SET(language, LANG_COMMON) )
		return TRUE;
	if ( language & LANG_CLAN )
	{
		/* Clan = common for mobs.. snicker.. -- Altrag */
		if ( IS_NPC(ch) || IS_NPC(cch) )
			return TRUE;
		if ( ch->pcdata->clan == cch->pcdata->clan &&
			 ch->pcdata->clan != NULL )
			return TRUE;
	}
	if ( !IS_NPC( ch ) )
	{
	    int lang;

		/* Racial languages for PCs */
	    if ( IS_SET(race_table[ch->race].language, language) )
	    	return TRUE;

	    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	      if ( IS_SET(language, lang_array[lang]) &&
	      	   IS_SET(ch->speaks, lang_array[lang]) )
	      {
		  if ( (sn = skill_lookup(lang_names[lang])) != -1
		  &&    ch->pcdata->learned[sn] >= 60 )
		    return TRUE;
	      }
	}
	return FALSE;
}

bool can_learn_lang( CHAR_DATA *ch, int language )
{
	if ( language & LANG_CLAN )
		return FALSE;
	if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
		return FALSE;
	if ( race_table[ch->race].language & language )
		return FALSE;
	if ( ch->speaks & language )
	{
		int lang;

		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
			if ( language & lang_array[lang] )
			{
				int sn;

				if ( !(VALID_LANGS & lang_array[lang]) )
					return FALSE;
				if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
				{
					bug( "Can_learn_lang: valid language without sn: %d", lang );
					continue;
				}
				if ( ch->pcdata->learned[sn] >= 99 )
					return FALSE;
			}
	}
	if ( VALID_LANGS & language )
		return TRUE;
	return FALSE;
}

int const lang_array[] = {
	LANG_COMMON,
	LANG_ELVEN,
	LANG_DWARVEN,
	LANG_PIXIE,
	LANG_OGRE,
	LANG_ORCISH,
	LANG_TROLLISH,
	LANG_RODENT,
	LANG_INSECTOID,
	LANG_MAMMAL,
	LANG_REPTILE,
	LANG_DRAGON,
	LANG_SPIRITUAL,
	LANG_MAGICAL,
	LANG_GOBLIN,
	LANG_GOD,
	LANG_ANCIENT,
	LANG_HALFLING,
	LANG_CLAN,
	LANG_GITH,
	LANG_KATRIN,
	LANG_UNKNOWN
};
/* Note: does not count racial language.  This is intentional (for now). */
int countlangs( int languages )
{
	int numlangs = 0;
	int looper;

	for ( looper = 0; lang_array[looper] != LANG_UNKNOWN; looper++ )
	{
		if ( lang_array[looper] == LANG_CLAN )
			continue;
		if ( languages & lang_array[looper] )
			numlangs++;
	}
	return numlangs;
}

const char * const lang_names[] = { "common", "elvish", "dwarven", "pixie", "ogre",
							 "orcish", "trollese", "rodent", "insectoid",
							 "mammal", "reptile", "dragon", "spiritual",
							 "magical", "goblin", "god", "ancient",
							 "halfling", "clan", "gith", "katrin", "" };
void do_speak(CHAR_DATA *ch, const char* argument)
{
	int langs;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg );

	if ( !str_cmp( arg, "all" ) && IS_IMMORTAL( ch ) )
	{
		set_char_color( AT_SAY, ch );
		ch->speaking = ~LANG_CLAN;
		send_to_char( "Now speaking all languages.\r\n", ch );
		return;
	}
	for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
    {
		if ( !str_prefix( arg, lang_names[langs] ) )
        {
			if ( knows_language( ch, lang_array[langs], ch ) )
			{
				if ( lang_array[langs] == LANG_CLAN &&
					(IS_NPC(ch) || !ch->pcdata->clan) )
					continue;
				ch->speaking = lang_array[langs];
				set_char_color( AT_SAY, ch );
				ch_printf( ch, "You now speak %s.\r\n", lang_names[langs] );
				return;
			}
        }
    }
	set_char_color( AT_SAY, ch );
	send_to_char( "You do not know that language.\r\n", ch );
}

void do_practice(CHAR_DATA *ch, const char* argument);
void do_languages(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	int lang;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' && !str_prefix( arg, "learn" ) &&
		!IS_IMMORTAL(ch) && !IS_NPC(ch) )
	{
		CHAR_DATA *sch;
		char arg2[MAX_INPUT_LENGTH];
		int sn;
		int prct;
		int prac;

		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Learn which language?\r\n", ch );
			return;
		}

        do_practice(ch, arg2);
        return;

		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		{
			if ( lang_array[lang] == LANG_CLAN )
				continue;
			if ( !str_prefix( arg2, lang_names[lang] ) )
				break;
		}
		if ( lang_array[lang] == LANG_UNKNOWN )
		{
			send_to_char( "That is not a language.\r\n", ch );
			return;
		}
		if ( !(VALID_LANGS & lang_array[lang]) )
		{
			send_to_char( "You may not learn that language.\r\n", ch );
			return;
		}
		if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
		{
			send_to_char( "That is not a language.\r\n", ch );
			return;
		}
		if ( race_table[ch->race].language & lang_array[lang] ||
			 lang_array[lang] == LANG_COMMON ||
			 ch->pcdata->learned[sn] >= 99 )
		{
			act( AT_PLAIN, "You are already fluent in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
			return;
		}
		for ( sch = ch->GetInRoom()->first_person; sch; sch = sch->next )
			if ( IS_NPC(sch) && IS_SET(sch->act, ACT_SCHOLAR) &&
					knows_language( sch, ch->speaking, ch ) &&
					knows_language( sch, lang_array[lang], sch ) &&
					(!sch->speaking || knows_language( ch, sch->speaking, sch )) )
				break;
		if ( !sch )
		{
			send_to_char( "There is no one who can teach that language here.\r\n", ch );
			return;
		}
		if ( countlangs(ch->speaks) >= (ch->level / 10) &&
			 ch->pcdata->learned[sn] <= 0 )
		{
			act( AT_TELL, "$n tells you 'You may not learn a new language yet.'",
				 sch, NULL, ch, TO_VICT );
			return;
		}
		/* 0..16 cha = 2 pracs, 17..25 = 1 prac. -- Altrag */
		prac = 2 - (ch->getCha() / 17);
		if ( ch->practice < prac )
		{
			act( AT_TELL, "$n tells you 'You do not have enough practices.'",
				 sch, NULL, ch, TO_VICT );
			return;
		}
		ch->practice -= prac;
		/* Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag */
		prct = 5 + (ch->getInt() / 6) + (ch->getWis() / 7);
		ch->pcdata->learned[sn] += prct;
		ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn], 99);
		SET_BIT( ch->speaks, lang_array[lang] );
		if ( ch->pcdata->learned[sn] == prct )
			act( AT_PLAIN, "You begin lessons in $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 )
			act( AT_PLAIN, "You continue lessons in $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 + prct )
			act( AT_PLAIN, "You feel you can start communicating in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 99 )
			act( AT_PLAIN, "You become more fluent in $t.", ch,
				 lang_names[lang], NULL, TO_CHAR );
		else
			act( AT_PLAIN, "You now speak perfect $t.", ch, lang_names[lang],
				 NULL, TO_CHAR );
		return;
	}
	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( knows_language( ch, lang_array[lang], ch ) )
		{
			if ( ch->speaking & lang_array[lang] ||
				(IS_NPC(ch) && !ch->speaking) )
				set_char_color( AT_RED, ch );
			else
				set_char_color( AT_SAY, ch );
			send_to_char( lang_names[lang], ch );
			send_to_char( " ", ch );
		}
	send_to_char( "\r\n", ch );
	return;
}

void do_wartalk(CHAR_DATA *ch, const char* argument)
{
    if (NOT_AUTHED(ch))
    {
      send_to_char("Huh?\r\n", ch);
      return;
    }
    talk_channel( ch, argument, CHANNEL_WARTALK, "war" );
    return;
}

void do_beep(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	if (IS_NPC(ch))
		return;
	argument = one_argument( argument, arg );
	if  ( arg[0] == '\0' )
	{
		send_to_char( "Beep who?\r\n", ch );
		return;
	}
	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
		send_to_char( "They are not here.\r\n", ch );
		return;
	}
	if ( IS_NPC(victim))
	{
		send_to_char( "They are not beepable.\r\n", ch );
		return;
	}
	sprintf( buf, "\aYou beep %s.\r\n", victim->getShort().c_str() );
	send_to_char( buf, ch );

	sprintf( buf, "\a%s has beeped you.\r\n", ch->getShort().c_str() );
	send_to_char( buf, victim );
	return;
}

