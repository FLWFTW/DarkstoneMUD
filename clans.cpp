/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 *			     Special clan module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"

#include "paths.const.h"

#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

ClanData * first_clan;
ClanData * last_clan;
CouncilData * first_council;
CouncilData * last_council;



ClanData::ClanData()
{
	next = prev = NULL;
	pkills = pdeaths = mkills = mdeaths = illegal_pk = score = color = clan_type = 0;
	favour = strikes = members = alignment = board = clanobj1 = clanobj2
		= clanobj3 = recall = storeroom = guard1 = guard2 = Class = 0;
}

CouncilData::CouncilData()
{
	next = prev = NULL;

	filename = description = powers = NULL;

	members = 0;

	board = meeting = 0;
}



/* local routines */
void	fread_clan	 ( ClanData *clan, FILE *fp ) ;
bool	load_clan_file	 ( const char *clanfile ) ;
void	write_clan_list	 ( void ) ;

void	fread_council	 ( CouncilData *council, FILE *fp ) ;
bool	load_council_file	 ( const char *councilfile ) ;
void	write_council_list	 ( void ) ;

/*
 * Get pointer to clan structure from clan name.
 */
ClanData *get_clan( const char *name )
{
	ClanData *clan;

	for ( clan = first_clan; clan; clan = clan->next )
		if ( !str_cmp( name, clan->name_.c_str() ) )
			return clan;
	return NULL;
}

CouncilData *get_council( const char *name )
{
    CouncilData *council;
    
    for ( council = first_council; council; council = council->next )
       if ( !str_cmp( name, council->name_.c_str() ) )
         return council;
    return NULL;
}

void write_clan_list( )
{
    ClanData *tclan;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open clan.lst for writing!\n\r", 0 );
 	return;
    }	  
    for ( tclan = first_clan; tclan; tclan = tclan->next )
	fprintf( fpout, "%s\n", tclan->filename_.c_str() );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

void write_council_list( )
{
	CouncilData *tcouncil;
	FILE *fpout;
	char filename[256];

	sprintf( filename, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
	fpout = fopen( filename, "w" );
	if ( !fpout )
	{
		bug( "FATAL: cannot open council.lst for writing!\n\r", 0 );
		return;
	}	  
	for ( tcouncil = first_council; tcouncil; tcouncil = tcouncil->next )
		fprintf( fpout, "%s\n", tcouncil->filename );
	fprintf( fpout, "$\n" );
	fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( ClanData *clan )
{
	FILE *fp;
	char filename[256];
	char buf[MAX_STRING_LENGTH];

	if ( !clan )
	{
		bug( "save_clan: null clan pointer!", 0 );
		return;
	}

	if ( clan->filename_.length() == 0 )
	{
		sprintf( buf, "save_clan: %s has no filename", clan->name_.c_str() );
		bug( buf, 0 );
		return;
	}

	sprintf( filename, "%s%s", CLAN_DIR, clan->filename_.c_str() );

	fclose( fpReserve );
	if ( ( fp = fopen( filename, "w" ) ) == NULL )
	{
		bug( "save_clan: fopen", 0 );
		perror( filename );
	}
	else
	{
		fprintf( fp, "#CLAN\n" );
		fprintf( fp, "Name         %s~\n",	clan->name_.c_str()		);
		fprintf( fp, "Filename     %s~\n",	clan->filename_.c_str()		);
		fprintf( fp, "Motto        %s~\n",	clan->motto_.c_str()		);
		fprintf( fp, "Description  %s~\n",	clan->description_.c_str()	);
		fprintf( fp, "Deity        %s~\n",	clan->deity_.c_str()		);
		fprintf( fp, "Leader       %s~\n",	clan->leader_.c_str()		);
		fprintf( fp, "NumberOne    %s~\n",	clan->number1_.c_str()		);
		fprintf( fp, "NumberTwo    %s~\n",	clan->number2_.c_str()		);
		fprintf( fp, "PKills       %d\n",	clan->pkills		);
		fprintf( fp, "PDeaths      %d\n",	clan->pdeaths		);
		fprintf( fp, "MKills       %d\n",	clan->mkills		);
		fprintf( fp, "MDeaths      %d\n",	clan->mdeaths		);
		fprintf( fp, "IllegalPK    %d\n",	clan->illegal_pk	);
		fprintf( fp, "Score        %d\n",	clan->score		);
		fprintf( fp, "Type         %d\n",	clan->clan_type		);
		fprintf( fp, "Color        %d\n",	clan->color			);
		fprintf( fp, "Class        %d\n",	clan->Class		);
		fprintf( fp, "Favour       %d\n",	clan->favour		);
		fprintf( fp, "Strikes      %d\n",	clan->strikes		);
		fprintf( fp, "Members      %d\n",	clan->members		);
		fprintf( fp, "Alignment    %d\n",	clan->alignment		);
		fprintf( fp, "Board        %d\n",	clan->board		);
		fprintf( fp, "ClanObjOne   %d\n",	clan->clanobj1		);
		fprintf( fp, "ClanObjTwo   %d\n",	clan->clanobj2		);
		fprintf( fp, "ClanObjThree %d\n",	clan->clanobj3		);
		fprintf( fp, "Recall       %d\n",	clan->recall		);
		fprintf( fp, "Storeroom    %d\n",	clan->storeroom		);
		fprintf( fp, "GuardOne     %d\n",	clan->guard1		);
		fprintf( fp, "GuardTwo     %d\n",	clan->guard2		);
		fprintf( fp, "End\n\n"						);
		fprintf( fp, "#END\n"						);
	}
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
}

/*
 * Save a council's data to its data file
 */
void save_council( CouncilData *council )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !council )
    {
	bug( "save_council: null council pointer!", 0 );
	return;
    }
        
    if ( !council->filename || council->filename[0] == '\0' )
    {
	sprintf( buf, "save_council: %s has no filename", council->name_.c_str() );
	bug( buf, 0 );
	return;
    }
 
    sprintf( filename, "%s%s", COUNCIL_DIR, council->filename );
    
    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_council: fopen", 0 );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#COUNCIL\n" );
	fprintf( fp, "Name         %s~\n",	council->name_.c_str()		);
	fprintf( fp, "Filename     %s~\n",	council->filename	);
	fprintf( fp, "Description  %s~\n",	council->description	);
	fprintf( fp, "Head         %s~\n",	council->head_.c_str()		);
	fprintf( fp, "Members      %d\n",	council->members	);
	fprintf( fp, "Board        %d\n",	council->board		);
	fprintf( fp, "Meeting      %d\n",	council->meeting	);
	fprintf( fp, "Powers       %s~\n",	council->powers		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual clan data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_clan( ClanData *clan, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    const char *word;
    bool fMatch;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
		case '*':
			fMatch = TRUE;
			fread_to_eol( fp );
			break;

		case 'A':
			KEY( "Alignment",	clan->alignment,	fread_number( fp ) );
			break;

		case 'B':
			KEY( "Board",	clan->board,		fread_number( fp ) );
			break;

		case 'C':
			KEY( "ClanObjOne",	clan->clanobj1,		fread_number( fp ) );
			KEY( "ClanObjTwo",	clan->clanobj2,		fread_number( fp ) );
			KEY( "ClanObjThree",clan->clanobj3,		fread_number( fp ) );
			KEY( "Class",	clan->Class,		fread_number( fp ) );
			KEY( "Color",		clan->color,		fread_number( fp ) );
			break;

		case 'D':
			KEY( "Deity",	clan->deity_,		fread_string_noheap( fp ) );
			KEY( "Description",	clan->description_,	fread_string_noheap( fp ) );
			break;

		case 'E':
			if ( !str_cmp( word, "End" ) )
				return;

			break;

		case 'F':
			KEY( "Favour",	clan->favour,		fread_number( fp ) );
			KEY( "Filename",	clan->filename_,		fread_string_nohash( fp ) );

		case 'G':
			KEY( "GuardOne",	clan->guard1,		fread_number( fp ) );
			KEY( "GuardTwo",	clan->guard2,		fread_number( fp ) );
			break;

		case 'I':
			KEY( "IllegalPK",	clan->illegal_pk,	fread_number( fp ) );
			break;

		case 'L':
			KEY( "Leader",	clan->leader_,		fread_string_nohash( fp ) );
			break;

		case 'M':
			KEY( "MDeaths",	clan->mdeaths,		fread_number( fp ) );
			KEY( "Members",	clan->members,		fread_number( fp ) );
			KEY( "MKills",	clan->mkills,		fread_number( fp ) );
			KEY( "Motto",	clan->motto_,		fread_string_nohash( fp ) );
			break;

		case 'N':
			KEY( "Name",	clan->name_,		fread_string_nohash( fp ) );
			KEY( "NumberOne",	clan->number1_,		fread_string_nohash( fp ) );
			KEY( "NumberTwo",	clan->number2_,		fread_string_nohash( fp ) );
			break;

		case 'P':
			KEY( "PDeaths",	clan->pdeaths,		fread_number( fp ) );
			KEY( "PKills",	clan->pkills,		fread_number( fp ) );
			break;

		case 'R':
			KEY( "Recall",	clan->recall,		fread_number( fp ) );
			break;

		case 'S':
			KEY( "Score",	clan->score,		fread_number( fp ) );
			KEY( "Strikes",	clan->strikes,		fread_number( fp ) );
			KEY( "Storeroom",	clan->storeroom,	fread_number( fp ) );
			break;

		case 'T':
			KEY( "Type",	clan->clan_type,	fread_number( fp ) );
			break;
	}
	
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_clan: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/*
 * Read in actual council data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_council( CouncilData *council, FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	const char *word;
	bool fMatch;

	for ( ; ; )
	{
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
			case '*':
				fMatch = TRUE;
				fread_to_eol( fp );
				break;

			case 'B':
				KEY( "Board",	council->board,		fread_number( fp ) );
				break;

			case 'D':
				KEY( "Description",	council->description,	fread_string( fp ) );
				break;

			case 'E':
				if ( !str_cmp( word, "End" ) )
				{
					if (!council->description)
						council->description 	= STRALLOC( "" );
					if (!council->powers)
						council->powers	= STRALLOC( "" );
					return;
				}
				break;

			case 'F':
				KEY( "Filename",	council->filename,	fread_string_nohash( fp ) );
				break;

			case 'H':
				KEY( "Head", 	council->head_, 		fread_string_nohash( fp ) );
				break;

			case 'M':
				KEY( "Members",	council->members,	fread_number( fp ) );
				KEY( "Meeting",   	council->meeting, 	fread_number( fp ) );
				break;

			case 'N':
				KEY( "Name",	council->name_,		fread_string_nohash( fp ) );
				break;

			case 'P':
				KEY( "Powers",	council->powers,	fread_string( fp ) );
				break;
		}

		if ( !fMatch )
		{
			sprintf( buf, "Fread_council: no match: %s", word );
			bug( buf, 0 );
		}
	}
}


/*
 * Load a clan file
 */

bool load_clan_file( const char *clanfile )
{
    char filename[256];
    ClanData *clan;
    FILE *fp;
    bool found;

    clan = new ClanData();

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, clanfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    const char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_clan_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "CLAN"	) )
	    {
	    	fread_clan( clan, fp );
	    	if (clan->color == 0)
			{
				/* quickie check by Ksilyan to make sure
				that we don't have black clan colors */
				clan->color = AT_BLUE;
			}
			break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_clan_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( found )
    {
	ROOM_INDEX_DATA *storeroom;

	LINK( clan, first_clan, last_clan, next, prev );

	if ( clan->storeroom == 0
	|| (storeroom = get_room_index( clan->storeroom )) == NULL )
	{
	    log_string( "Storeroom not found" );
	    return found;
	}
	
	sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename_.c_str() );
	if ( ( fp = fopen( filename, "r" ) ) != NULL )
	{
	    int iNest;
	    bool found;
	    OBJ_DATA *tobj, *tobj_next;

	    log_string( "Loading clan storage room" );
	    rset_supermob(storeroom);
	    for ( iNest = 0; iNest < MAX_NEST; iNest++ )
		rgObjNest[iNest] = NULL;

	    found = TRUE;
	    for ( ; ; )
	    {
		char letter;
		const char *word;

		letter = fread_letter( fp );
		if ( letter == '*' )
		{
		    fread_to_eol( fp );
		    continue;
		}

		if ( letter != '#' )
		{
		    bug( "Load_clan_vault: # not found.", 0 );
		    bug( clan->name_.c_str(), 0 );
		    break;
		}

		word = fread_word( fp );
		if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
		  fread_obj  ( supermob, fp, OS_CARRY );
		else
		if ( !str_cmp( word, "END"    ) )	/* Done		*/
		  break;
		else
		{
		    bug( "Load_clan_vault: bad section.", 0 );
		    bug( clan->name_.c_str(), 0 );
		    break;
		}
	    }
	    fclose( fp );
	    for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
	    {
		tobj_next = tobj->next_content;
		obj_from_char( tobj );
		obj_to_room( tobj, storeroom );
	    }
	    release_supermob();
	}
	else
	    log_string( "Cannot open clan vault" );
    }
    else
      delete clan;

    return found;
}

/*
 * Load a council file
 */

bool load_council_file( const char *councilfile )
{
    char filename[256];
    CouncilData *council;
    FILE *fp;
    bool found;

    council = new CouncilData;

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, councilfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    const char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_council_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "COUNCIL"	) )
	    {
	    	fread_council( council, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_council_file: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( found )
      LINK( council, first_council, last_council, next, prev );

    else
      delete council;

    return found;
}

/*
 * Load in all the clan files.
 */
void load_clans( )
{
    FILE *fpList;
    const char *filename;
    char clanlist[256];
    char buf[MAX_STRING_LENGTH];
    
    
    first_clan	= NULL;
    last_clan	= NULL;

    log_string( "Loading clans..." );

    sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
	perror( clanlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_clan_file( filename ) )
	{
	  sprintf( buf, "Cannot load clan file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done clans " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Load in all the council files.
 */
void load_councils( )
{
    FILE *fpList;
    const char *filename;
    char councillist[256];
    char buf[MAX_STRING_LENGTH];
    
    
    first_council	= NULL;
    last_council	= NULL;

    log_string( "Loading councils..." );

    sprintf( councillist, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( councillist, "r" ) ) == NULL )
    {
	perror( councillist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_council_file( filename ) )
	{
	  sprintf( buf, "Cannot load council file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done councils " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_make(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	ClanData *clan;
	int level;

	if ( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if ( ch->getName().ciEqual(clan->leader_) == false
			&&   ch->getName().ciEqual(clan->deity_) == false
			&&  (clan->clan_type != CLAN_GUILD
				||   ch->getName().ciEqual(clan->number1_) == false
				)
			)
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Make what?\n\r", ch );
		return;
	}

	pObjIndex = get_obj_index( clan->clanobj1 );
	level = 40;

	if ( !pObjIndex || !is_name( arg, pObjIndex->name_.c_str() ) )
	{
		pObjIndex = get_obj_index( clan->clanobj2 );
		level = 45;
	}
	if ( !pObjIndex || !is_name( arg, pObjIndex->name_.c_str() ) )
	{
		pObjIndex = get_obj_index( clan->clanobj3 );
		level = 50;
	}

	if ( !pObjIndex || !is_name( arg, pObjIndex->name_.c_str() ) )
	{
		send_to_char( "You don't know how to make that.\n\r", ch );
		return;
	}

	obj = create_object( pObjIndex, level );
	act( AT_MAGIC, "$n makes $p!", ch, obj, NULL, TO_ROOM );
	act( AT_MAGIC, "You make $p!", ch, obj, NULL, TO_CHAR );
	SET_BIT( obj->extra_flags, ITEM_CLANOBJECT );
	if ( CAN_WEAR(obj, ITEM_TAKE) )
		obj = obj_to_char( obj, ch );
	else
		obj = obj_to_room( obj, ch->GetInRoom() );

	return;
}

void do_induct(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ClanData *clan;

	if ( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if ( (ch->pcdata && ch->pcdata->bestowments_.c_str()
				&&    is_name("induct", ch->pcdata->bestowments_.c_str()))
			||   ch->getName().ciEqual( clan->deity_   )
			||   ch->getName().ciEqual( clan->leader_  )
			||   ch->getName().ciEqual( clan->number1_ )
			||   ch->getName().ciEqual( clan->number2_ ) )
		;
	else
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Induct whom?\n\r", ch );
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

	if ( clan->clan_type == CLAN_GUILD )
	{
#if 0 /* Testaur - no one wants single-class guilds */
		if ( victim->Class != clan->Class)
		{
			send_to_char( "This player's will is not in accordance with your guild.\n\r", ch);
			return;
		}
#endif
	}
	else
	{
		if ( victim->level < 2 )
		{
			send_to_char( "This player is not worthy of joining yet.\n\r", ch );
			return;
		}

		if ( victim->level > ch->level )
		{
			send_to_char( "This player is too powerful for you to induct.\n\r", ch );
			return;
		}
	}

	if ( victim->pcdata->clan )
	{
		if ( victim->pcdata->clan->clan_type == CLAN_ORDER )
		{
			if ( victim->pcdata->clan == clan )
				send_to_char( "This player already belongs to your order!\n\r", ch );
			else
				send_to_char( "This player already belongs to an order!\n\r", ch );
			return;
		}
		else
			if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
			{
				if ( victim->pcdata->clan == clan )
					send_to_char( "This player already belongs to your guild!\n\r", ch );
				else
					send_to_char( "This player already belongs to an guild!\n\r", ch );
				return;
			}
			else
			{
				if ( victim->pcdata->clan == clan )
					send_to_char( "This player already belongs to your clan!\n\r", ch );
				else
					send_to_char( "This player already belongs to a clan!\n\r", ch );
				return;
			}
	}
	clan->members++;
	if ( clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
		SET_BIT(victim->speaks, LANG_CLAN);

	if ( clan->clan_type == CLAN_GUILD )
	{
		int sn;

		for ( sn = 0; sn < top_sn; sn++ )
		{
			if (skill_table[sn]->guild == clan->Class &&
					skill_table[sn]->name_.c_str() != NULL )
			{
				victim->pcdata->learned[sn] = GET_ADEPT(victim, sn);
				ch_printf( victim, "%s instructs you in the ways of %s.\n\r",
					ch->getName().c_str(), skill_table[sn]->name_.c_str());
			}
		}
	}

	victim->pcdata->clan = clan;
	victim->pcdata->clanName_ = clan->name_;
	act( AT_MAGIC, "You induct $N into $t", ch, clan->name_.c_str(), victim, TO_CHAR );
	act( AT_MAGIC, "$n inducts $N into $t", ch, clan->name_.c_str(), victim, TO_NOTVICT );
	act( AT_MAGIC, "$n inducts you into $t", ch, clan->name_.c_str(), victim, TO_VICT );
	save_char_obj( victim );
	return;
}

void do_council_induct(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CouncilData *council;

	if ( IS_NPC( ch ) || !ch->pcdata->council )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	council = ch->pcdata->council;

	if ( !ch->getName().ciEqual(council->head_) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Induct whom into your council?\n\r", ch );
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

	/*    if ( victim->level < 51 )
		  {
		  send_to_char( "This player is not worthy of joining any council yet.\n\r", ch );
		  return;
		  }
	 */
	if ( victim->pcdata->council )
	{
		send_to_char( "This player already belongs to a council!\n\r", ch );
		return;
	}

	council->members++;
	victim->pcdata->council = council;
	victim->pcdata->councilName_ = council->name_;
	act( AT_MAGIC, "You induct $N into $t", ch, council->name_.c_str(), victim, TO_CHAR );
	act( AT_MAGIC, "$n inducts $N into $t", ch, council->name_.c_str(), victim, TO_ROOM );
	act( AT_MAGIC, "$n inducts you into $t", ch, council->name_.c_str(), victim, TO_VICT );
	save_char_obj( victim );
	save_council( council );
	return;
}

void do_outcast(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ClanData *clan;
	char buf[MAX_STRING_LENGTH];

	if ( IS_NPC( ch ) || !ch->pcdata->clan )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	clan = ch->pcdata->clan;

	if ( (ch->pcdata && ch->pcdata->bestowments_.c_str()
				&&    is_name("outcast", ch->pcdata->bestowments_.c_str()))
			||   ch->getName().ciEqual( clan->deity_   )
			||   ch->getName().ciEqual( clan->leader_  )
			||   ch->getName().ciEqual( clan->number1_ )
			||   ch->getName().ciEqual( clan->number2_ ) )
		;
	else
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}


	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Outcast whom?\n\r", ch );
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

	if ( victim == ch )
	{
		if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
		{
			send_to_char( "Kick yourself out of your own order?\n\r", ch );
			return;
		}
		else
			if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
			{
				send_to_char( "Kick yourself out of your own guild?\n\r", ch );
				return;
			}
			else
			{
				send_to_char( "Kick yourself out of your own clan?\n\r", ch );
				return;
			}
	}

	if ( victim->level > ch->level )
	{
		send_to_char( "This player is too powerful for you to outcast.\n\r", ch );
		return;
	}

	if ( victim->pcdata->clan != ch->pcdata->clan )
	{
		if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
		{
			send_to_char( "This player does not belong to your order!\n\r", ch );
			return;
		}
		else
			if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
			{
				send_to_char( "This player does not belong to your guild!\n\r", ch );
				return;
			}
			else
			{
				send_to_char( "This player does not belong to your clan!\n\r", ch );
				return;
			}
	}

	if ( clan->clan_type == CLAN_GUILD )
	{
		int sn;

		for ( sn = 0; sn < top_sn; sn++ )
			if ( skill_table[sn]->guild == victim->pcdata->clan->Class
					&&   skill_table[sn]->name_.c_str() != NULL )
			{
				victim->pcdata->learned[sn] = 0;
				ch_printf( victim, "You forget the ways of %s.\n\r", skill_table[sn]->name_.c_str());
			}
	}

	if ( victim->speaking & LANG_CLAN )
		victim->speaking = LANG_COMMON;
	REMOVE_BIT( victim->speaks, LANG_CLAN );
	--clan->members;
	if ( victim->getName().ciEqual(ch->pcdata->clan->number1_) )
	{
		ch->pcdata->clan->number1_ = "";
	}
	if ( victim->getName().ciEqual(ch->pcdata->clan->number2_) )
	{
		ch->pcdata->clan->number2_ = "";
	}
	victim->pcdata->clan = NULL;
	victim->pcdata->clanName_ = "";
	act( AT_MAGIC, "You outcast $N from $t", ch, clan->name_.c_str(), victim, TO_CHAR );
	act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name_.c_str(), victim, TO_ROOM );
	act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name_.c_str(), victim, TO_VICT );
	if ( clan->clan_type != CLAN_GUILD )
	{
		sprintf(buf, "%s has been outcast from %s!",
			victim->getShort().c_str(), clan->name_.c_str());
		echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);
	}

	/* Outcast flag setting removed by Narn.  It's useless now that deadlies
	   remain deadly even on being cast out of a clan.
	 */ 
	/*    if ( clan->clan_type != CLAN_GUILD )
		  SET_BIT(victim->act, PLR_OUTCAST);
	 */
	save_char_obj( victim );	/* clan gets saved when pfile is saved */
	return;
}

void do_council_outcast(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CouncilData *council;

	if ( IS_NPC( ch ) || !ch->pcdata->council )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	council = ch->pcdata->council;

	if ( ! ch->getName().ciEqual( council->head_ ) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Outcast whom from your council?\n\r", ch );
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

	if ( victim == ch )
	{
		send_to_char( "Kick yourself out of your own council?\n\r", ch );
		return;
	}

	if ( victim->pcdata->council != ch->pcdata->council )
	{
		send_to_char( "This player does not belong to your council!\n\r", ch );
		return;
	}

	--council->members;
	victim->pcdata->council = NULL;
	victim->pcdata->councilName_ = "";
	act( AT_MAGIC, "You outcast $N from $t", ch, council->name_.c_str(), victim, TO_CHAR );
	act( AT_MAGIC, "$n outcasts $N from $t", ch, council->name_.c_str(), victim, TO_ROOM );
	act( AT_MAGIC, "$n outcasts you from $t", ch, council->name_.c_str(), victim, TO_VICT );
	save_char_obj( victim );
	save_council( council );
	return;
}

void do_setclan(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ClanData *clan;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Usage: setclan <clan> <field> <deity|leader|number1|number2> <player>\n\r", ch );
		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( " deity leader number1 number2\n\r", ch ); 
		send_to_char( " members board recall storage\n\r", ch );
		send_to_char( " align color\n\r", ch );	
		send_to_char( " obj1 obj2 obj3 guard1 guard2\n\r", ch );
		send_to_char( " name filename motto desc\n\r", ch );
		send_to_char( " favour strikes type\n\r", ch );
		return;
	}

	clan = get_clan( arg1 );
	if ( !clan )
	{
		send_to_char( "No such clan.\n\r", ch );
		return;
	}

	if ( !strcmp( arg2, "deity" ) )
	{
		clan->deity_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "leader" ) )
	{
		clan->leader_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "number1" ) )
	{
		clan->number1_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "number2" ) )
	{
		clan->number2_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "board" ) )
	{
		clan->board = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "members" ) )
	{
		clan->members = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "recall" ) )
	{
		clan->recall = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "storage" ) )
	{
		clan->storeroom = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "obj1" ) )
	{
		clan->clanobj1 = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "obj2" ) )
	{
		clan->clanobj2 = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "obj3" ) )
	{
		clan->clanobj3 = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "guard1" ) )
	{
		clan->guard1 = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "guard2" ) )
	{
		clan->guard2 = dotted_to_vnum( ch->GetInRoom()->vnum, argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "align" ) )
	{
		clan->alignment = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	/* KSILYAN
	   Added this small feature to allow for different
	   clan colors (to make the who list more interesting :P)
	 */
	if ( !strcmp( arg2, "color" ))
	{
		int color = atoi( argument );
		if ((color == 0) || (color > 15))
		{
			send_to_char( "Invalid color!\n\r", ch);
			return;
		}
		clan->color = color;
		send_to_char( "Done.\n\r", ch);
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "type" ) )
	{
		if ( !str_cmp( argument, "order" ) )
			clan->clan_type = CLAN_ORDER;
		else
			if ( !str_cmp( argument, "guild" ) )
				clan->clan_type = CLAN_GUILD;
			else
				clan->clan_type = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "class" ) )
	{
		clan->Class = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "name" ) )
	{
		clan->name_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "filename" ) )
	{
		clan->filename_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		write_clan_list( );
		return;
	}

	if ( !strcmp( arg2, "motto" ) )
	{
		clan->motto_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	if ( !strcmp( arg2, "desc" ) )
	{
		clan->description_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_clan( clan );
		return;
	}

	return;
}

void do_setcouncil(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CouncilData *council;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Usage: setcouncil <council> <field> <deity|leader|number1|number2> <player>\n\r", ch );
		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( " head members board meeting\n\r", ch ); 
		send_to_char( " name filename desc\n\r", ch );
		send_to_char( " powers\n\r", ch);
		return;
	}

	council = get_council( arg1 );
	if ( !council )
	{
		send_to_char( "No such council.\n\r", ch );
		return;
	}

	if ( !strcmp( arg2, "head" ) )
	{
		council->head_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		return;
	}

	if ( !strcmp( arg2, "board" ) )
	{
		council->board = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		return;
	}

	if ( !strcmp( arg2, "members" ) )
	{
		council->members = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		return;
	}

	if ( !strcmp( arg2, "meeting" ) )
	{
		council->meeting = atoi( argument );
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		return;
	}

	if ( !strcmp( arg2, "name" ) )
	{
		council->name_ = argument;
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		return;
	}


	if ( !strcmp( arg2, "filename" ) )
	{
		DISPOSE( council->filename );
		council->filename = str_dup( argument );
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		write_council_list( );
		return;
	}

	if ( !strcmp( arg2, "desc" ) )
	{
		STRFREE( council->description );
		council->description = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		return;
	}

	if ( !strcmp( arg2, "powers" ) )
	{
		STRFREE( council->powers );
		council->powers = STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		save_council( council );
		return;
	}

	do_setcouncil( ch, "" );
	return;
}

void do_showclan(CHAR_DATA *ch, const char* argument)
{   
	ClanData *clan;

	if ( IS_NPC( ch ) )
	{
		send_to_char( "Huh?\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "Usage: showclan <clan>\n\r", ch );
		return;
	}

	clan = get_clan( argument );
	if ( !clan )
	{
		send_to_char( "No such clan.\n\r", ch );
		return;
	}

	set_pager_color(AT_PLAIN, ch);
	ch_printf( ch, "%s      : %s\n\rFilename: %s\n\rMotto   : %s\n\r",
			clan->clan_type == CLAN_ORDER ? "Order" :
			(clan->clan_type == CLAN_GUILD ? "Guild" : "Clan "),
			clan->name_.c_str(),
			clan->filename_.c_str(),
			clan->motto_.c_str() );
	send_to_char( "Clan color: ", ch);
	set_pager_color( clan->color, ch);
	send_to_char( "(color)\n\r", ch);
	set_pager_color( AT_PLAIN, ch);
	ch_printf( ch, "Description: %s\n\rDeity: %s\n\rLeader: %s\n\r",
			clan->description_.c_str(),
			clan->deity_.c_str(),
			clan->leader_.c_str() );
	ch_printf( ch, "Number1: %s\n\rNumber2: %s\n\rPKills: %6d    PDeaths: %6d\n\r",
			clan->number1_.c_str(),
			clan->number2_.c_str(),
			clan->pkills,
			clan->pdeaths );
	ch_printf( ch, "MKills: %6d    MDeaths: %6d\n\r",
			clan->mkills,
			clan->mdeaths );
	ch_printf( ch, "IllegalPK: %-6d Score: %d\n\r",
			clan->illegal_pk,
			clan->score );
	ch_printf( ch, "Type: %d    Favour: %6d  Strikes: %d\n\r",
			clan->clan_type,
			clan->favour,
			clan->strikes );
	ch_printf( ch, "Members: %3d   Alignment: %d  Class: %d\n\r",
			clan->members,
			clan->alignment,
			clan->Class );
	ch_printf( ch, "Board: %5d     Object1: %5d Object2: %5d Object3: %5d\n\r",
			clan->board,
			clan->clanobj1,
			clan->clanobj2,
			clan->clanobj3 );
	ch_printf( ch, "Recall: %5d  Storeroom: %5d  Guard1: %5d  Guard2: %5d\n\r",
			clan->recall,
			clan->storeroom,
			clan->guard1,
			clan->guard2 );
	return;
}

void do_showcouncil(CHAR_DATA *ch, const char* argument)
{
    CouncilData *council;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showcouncil <council>\n\r", ch );
	return;
    }

    council = get_council( argument );
    if ( !council )
    {
	send_to_char( "No such council.\n\r", ch );
	return;
    }

    ch_printf( ch, "Council    : %s\n\rFilename: %s\n\r",
    			council->name_.c_str(),
    			council->filename );
    ch_printf( ch, "Description: %s\n\rHead: %s\n\rMembers: %3d\n\r",
    			council->description,
    			council->head_.c_str(),
    			council->members );
    ch_printf( ch, "Board: %5d\n\rMeeting: %5d\n\rPowers: %s\n\r",
    			council->board,
    			council->meeting,
			council->powers );
    return;
}

void do_makeclan(CHAR_DATA *ch, const char* argument)
{
	char filename[256];
	ClanData *clan;
	bool found;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makeclan <clan name>\n\r", ch );
		return;
	}

	found = FALSE;
	sprintf( filename, "%s%s", CLAN_DIR, strlower(argument) );

	clan = new ClanData;
	LINK( clan, first_clan, last_clan, next, prev );
}

void do_makecouncil(CHAR_DATA *ch, const char* argument)
{
    char filename[256];
    CouncilData *council;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makecouncil <council name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, strlower(argument) );

	council = new CouncilData;
    LINK( council, first_council, last_council, next, prev );
    council->name_		= argument;
    council->head_		= "";
    council->powers		= STRALLOC( "" );
}

void do_clans(CHAR_DATA *ch, const char* argument)
{
    ClanData *clan;
    int count = 0;

    /* Switched deadly clan mobkills/mobdeaths to pkills -- Blodkai */
    set_char_color( AT_BLOOD, ch );
    send_to_char( "\n\rClan           Deity          Leader            Pkills\n\r", ch );
    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD )
          continue;
        set_char_color( AT_NOTE, ch );
        ch_printf( ch, "%-14s %-14s %-14s", clan->name_.c_str(), clan->deity_.c_str(), clan->leader_.c_str() );
        set_char_color( AT_BLOOD, ch );
        ch_printf( ch, "   %7d\n\r", clan->pkills );
        count++;
    }

    if ( !count )
    {
	set_char_color( AT_BLOOD, ch);
        send_to_char( "There are no clans currently formed.\n\r", ch );
	return;
    }
}

void do_orders(CHAR_DATA *ch, const char* argument)
{
    ClanData *order;
    int count = 0;

   /* Added displaying of mkills and mdeaths	- Brittany */
    set_char_color( AT_NOTE, ch );
    send_to_char( "Order            Deity          Leader        Mkills     Mdeaths\n\r", ch );
    for ( order = first_clan; order; order = order->next )
        if ( order->clan_type == CLAN_ORDER )
	{
	    ch_printf( ch, "%-16s %-14s %-14s %5d       %5d\n\r", order->name_.c_str(),
		order->deity_.c_str(), order->leader_.c_str(), order->mkills, order->mdeaths );
	    count++;
	}

    if ( !count )
    {
	send_to_char( "There are no Orders currently formed.\n\r", ch );
	return;
    }
}

void do_councils(CHAR_DATA *ch, const char* argument)
{
	CouncilData *council;

	if ( !first_council )
	{
		send_to_char( "There are no councils currently formed.\n\r", ch );
		return;
	}

	set_char_color( AT_NOTE, ch );
	send_to_char( "Name                  Head\n\r", ch);
	for ( council = first_council; council; council = council->next )
		ch_printf( ch, "%-21s %-14s\n\r", council->name_.c_str(), council->head_.c_str());
}                                                                           

void do_guilds(CHAR_DATA *ch, const char* argument)
{
    ClanData *guild;
    int count = 0;

    /* Added guild mobkills/mobdeaths -- Blodkai */
    set_char_color( AT_NOTE, ch );
    send_to_char( "\n\rGuild                  Leader             Mkills      Mdeaths\n\r", ch);
    for ( guild = first_clan; guild; guild = guild->next )
        if ( guild->clan_type == CLAN_GUILD )
	{
	    ++count;
	    set_char_color( AT_YELLOW, ch );
	    ch_printf( ch, "%-20s   %-14s     %-6d       %6d\n\r",
			guild->name_.c_str(), guild->leader_.c_str(), guild->mkills, guild->mdeaths );
	}

    set_char_color( AT_NOTE, ch );
    if ( !count )
	send_to_char( "There are no Guilds currently formed.\n\r", ch );
    else
	ch_printf( ch, "%d guilds found.\n\r", count );
}                                                                           

void do_shove(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int exit_dir;
	ExitData *pexit;
	CHAR_DATA *victim;
	bool nogo;
	ROOM_INDEX_DATA *to_room;	 
	int chance;  
	int race_bonus;
	
	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	
	if ( IS_NPC(ch) )
	{
		send_to_char("Only deadly characters can shove.\n\r", ch);
		return;
	}
	
	if ( arg[0] == '\0' )
	{
		send_to_char( "Shove whom?\n\r", ch);
		return;
	}
	
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch);
		return;
	}
	
	if (victim == ch)
	{
		send_to_char("You shove yourself around, to no avail.\n\r", ch);
		return;
	}
	if ( IS_NPC(victim))
	{
		send_to_char("You can only shove deadly characters.\n\r", ch);
		return;
	}
	
	if ( ch->level - victim->level > 5 
		||	 victim->level - ch->level > 5 )
	{
		send_to_char("There is too great an experience difference for you to even bother.\n\r", ch);
		return;
	}
	
	if ( (victim->position) != POS_STANDING )
	{
		act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
		return;
	}
	
	if ( arg2[0] == '\0' )
	{
		send_to_char( "Shove them in which direction?\n\r", ch);
		return;
	}
	
	exit_dir = get_dir( arg2 );
	if ( IS_SET(victim->GetInRoom()->room_flags, ROOM_SAFE)
		&&	get_timer(victim, TIMER_SHOVEDRAG) <= 0)
	{
		send_to_char("That character cannot be shoved right now.\n\r", ch);
		return;
	}
	victim->position = POS_SHOVE;
	nogo = FALSE;
	if ((pexit = get_exit(ch->GetInRoom(), exit_dir)) == NULL )
		nogo = TRUE;
	else if ( IS_SET(pexit->exit_info, EX_CLOSED)
		&& (!IS_AFFECTED(victim, AFF_PASS_DOOR)
		||	 IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
		nogo = TRUE;
	if ( nogo )
	{
		send_to_char( "There's no exit in that direction.\n\r", ch );
		victim->position = POS_STANDING;
		return;
	}
	to_room = pexit->to_room;
	if (IS_SET(to_room->room_flags, ROOM_DEATH))
	{
		send_to_char("You cannot shove someone into a death trap.\n\r", ch);
		victim->position = POS_STANDING;
		return;
	}
	
	if (ch->GetInRoom()->area != to_room->area
		&&	!in_hard_range( victim, to_room->area ) )
	{
		send_to_char("That character cannot enter that area.\n\r", ch);
		victim->position = POS_STANDING;
		return;
	}
	
	/* Check for class, assign percentage based on that. */
	if (ch->Class == CLASS_WARRIOR)
		chance = 70;
	if (ch->Class == CLASS_VAMPIRE)
		chance = 65;
	if (ch->Class == CLASS_RANGER)
		chance = 60;
	if (ch->Class == CLASS_DRUID)
		chance = 45;
	if (ch->Class == CLASS_CLERIC)
		chance = 35;
	if (ch->Class == CLASS_THIEF)
		chance = 30;
	if (ch->Class == CLASS_MAGE)
		chance = 15;
	
		/* Add 3 points to chance for every str point above 15, subtract for 
	below 15 */
	
	chance += ((ch->getStr() - 15) * 3);
	
	chance += (ch->level - victim->level);
	
	if (ch->race == 1)
		race_bonus = -3;
	
	if (ch->race == 2)
		race_bonus = 3;
	
	if (ch->race == 3)
		race_bonus = -5;
	
	if (ch->race == 4)
		race_bonus = -7;
	
	if (ch->race == 6)
		race_bonus = 5;
	
	if (ch->race == 7)
		race_bonus = 7;
	
	if (ch->race == 8)
		race_bonus = 10;
	
	if (ch->race == 9)
		race_bonus = -2;
	
	chance += race_bonus;
	
	/* Debugging purposes - show percentage for testing */
	
	/* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
	*/
	
	if (chance < number_percent( ))
	{
		send_to_char("You failed.\n\r", ch);
		victim->position = POS_STANDING;
		return;
	}
	act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
	move_char( victim, get_exit(ch->GetInRoom(),exit_dir), 0);
	if ( !char_died(victim) )
		victim->position = POS_STANDING;
	ch->AddWait( 12);
	/* Remove protection from shove/drag if char shoves -- Blodkai */
	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_SAFE)   
		&&	 get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
		add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

void do_drag(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int exit_dir;
	CHAR_DATA *victim;
	ExitData *pexit;
	ROOM_INDEX_DATA *to_room;
	bool nogo;
	int chance;
	int race_bonus;

	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );

	if ( IS_NPC(ch))
	{
		send_to_char("Only deadly characters can drag.\n\r", ch);
		return;
	}

	if ( arg[0] == '\0' )
	{
		send_to_char( "Drag whom?\n\r", ch);
		return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch);
		return;
	}

	if ( victim == ch )
	{
		send_to_char("You take yourself by the scruff of your neck, but go nowhere.\n\r", ch);
		return; 
	}

	if ( IS_NPC(victim))
	{
		send_to_char("You can only drag deadly characters.\n\r", ch);
		return;
	}

	if ( victim->IsFighting() )
	{
		send_to_char( "You try, but can't get close enough.\n\r", ch);
		return;
	}

	if ( arg2[0] == '\0' )
	{
		send_to_char( "Drag them in which direction?\n\r", ch);
		return;
	}

	if ( ch->level - victim->level > 5
			||   victim->level - ch->level > 5 )
	{
		send_to_char("There is too great an experience difference for you to even bother.\n\r", ch);
		return;
	}

	exit_dir = get_dir( arg2 );

	if ( IS_SET(victim->GetInRoom()->room_flags, ROOM_SAFE)
			&&   get_timer( victim, TIMER_SHOVEDRAG ) <= 0)
	{
		send_to_char("That character cannot be dragged right now.\n\r", ch);
		return;
	}

	nogo = FALSE;
	if ((pexit = get_exit(ch->GetInRoom(), exit_dir)) == NULL )
		nogo = TRUE;
	else
		if ( IS_SET(pexit->exit_info, EX_CLOSED)
				&& (!IS_AFFECTED(victim, AFF_PASS_DOOR)
					||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
			nogo = TRUE;
	if ( nogo )
	{
		send_to_char( "There's no exit in that direction.\n\r", ch );
		return;
	}

	to_room = pexit->to_room;
	if (IS_SET(to_room->room_flags, ROOM_DEATH))
	{
		send_to_char("You cannot drag someone into a death trap.\n\r", ch);
		return;
	}

	if (ch->GetInRoom()->area != to_room->area
			&& !in_hard_range( victim, to_room->area ) )
	{
		send_to_char("That character cannot enter that area.\n\r", ch);
		victim->position = POS_STANDING;
		return;
	}

	/* Check for class, assign percentage based on that. */
	if (ch->Class == CLASS_WARRIOR)
		chance = 70;
	if (ch->Class == CLASS_VAMPIRE)
		chance = 65;
	if (ch->Class == CLASS_RANGER)
		chance = 60;
	if (ch->Class == CLASS_DRUID)
		chance = 45;
	if (ch->Class == CLASS_CLERIC)
		chance = 35;
	if (ch->Class == CLASS_THIEF)
		chance = 30;
	if (ch->Class == CLASS_MAGE)
		chance = 15;

	/* Add 3 points to chance for every str point above 15, subtract for 
	   below 15 */

	chance += ((ch->getStr() - 15) * 3);

	chance += (ch->level - victim->level);

	if (ch->race == 1)
		race_bonus = -3;

	if (ch->race == 2)
		race_bonus = 3;

	if (ch->race == 3)
		race_bonus = -5;

	if (ch->race == 4)
		race_bonus = -7;

	if (ch->race == 6)
		race_bonus = 5;

	if (ch->race == 7)
		race_bonus = 7;

	if (ch->race == 8)
		race_bonus = 10;

	if (ch->race == 9)
		race_bonus = -2;

	chance += race_bonus;
	/*
	   sprintf(buf, "Drag percentage of %s = %d", ch->name, chance);
	   act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
	 */
	if (chance < number_percent( ))
	{
		send_to_char("You failed.\n\r", ch);
		victim->position = POS_STANDING;
		return;
	}
	if ( victim->position < POS_STANDING )
	{
		sh_int temp;

		temp = victim->position;
		victim->position = POS_DRAG;
		act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR ); 
		act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT ); 
		move_char( victim, get_exit(ch->GetInRoom(),exit_dir), 0);
		if ( !char_died(victim) )
			victim->position = temp;
		/* Move ch to the room too.. they are doing dragging - Scryn */
		move_char( ch, get_exit(ch->GetInRoom(),exit_dir), 0);
		ch->AddWait( 12);
		return;
	}
	send_to_char("You cannot do that to someone who is standing.\n\r", ch);
	return;
}

