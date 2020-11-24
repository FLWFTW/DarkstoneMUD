/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 ****************************************************************************
 *  The MUDprograms are heavily based on the original MOBprogram code that  *
 *  was written by N'Atas-ha.						    *
 *  Much has been added, including the capability to put a "program" on     *
 *  rooms and objects, not to mention many more triggers and ifchecks, as   *
 *  well as "script" support.						    *
 *                                                                          *
 *  Error reporting has been changed to specify whether the offending       *
 *  program is on a mob, a room or and object, along with the vnum.         *
 *                                                                          *
 *  Mudprog parsing has been rewritten (in mprog_driver). Mprog_process_if  *
 *  and mprog_process_cmnd have been removed, mprog_do_command is new.      *
 *  Full support for nested ifs is in.                                      *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "mud.h"
#include "commands.h"

#include "World.h"

int fcontinue_after_command; /* for command triggers */
bool bMpwalkSuccess;

bool MOBtrigger; // moved here from mud.h by Ksilyan

/* Defines by Narn for new mudprog parsing, used as
   return values from mprog_do_command. */
#define COMMANDOK    1
#define IFTRUE       2
#define IFFALSE      3
#define ORTRUE       4
#define ORFALSE      5
#define FOUNDELSE    6
#define FOUNDENDIF   7
#define IFIGNORED    8
#define ORIGNORED    9
#define ANDTRUE      10
#define ANDFALSE     11

/* Ifstate defines, used to create and access ifstate array
   in mprog_driver. */
#define MAX_IFS     20		/* should always be generous */
#define IN_IF        0
#define IN_ELSE      1
#define DO_IF        2
#define DO_ELSE      3

#define MAX_PROG_NEST   20

int mprog_do_command( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor,
                      OBJ_DATA *obj, void *vo, CHAR_DATA *rndm,
                      bool ignore, bool ignore_ors );

/*
 *  Mudprogram additions
 */
CHAR_DATA *supermob;
struct act_prog_data *room_act_list;
struct act_prog_data *obj_act_list;
struct act_prog_data *mob_act_list;

/*
 * Local function prototypes
 */

char *	mprog_next_command	 ( char* clist ) ;
bool	mprog_seval		( const char* lhs, char* opr, char* rhs,
                                        CHAR_DATA *mob );
bool	mprog_veval		( int lhs, char* opr, int rhs,
                                        CHAR_DATA *mob );
int	mprog_do_ifcheck	( char* ifcheck, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm );
void	mprog_translate		( char ch, char* t, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, CHAR_DATA* rndm );
void	mprog_driver		( char* com_list, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       void* vo, bool single_step );

bool mprog_keyword_check	 ( const char *argu, const char *argl ) ;


void oprog_wordlist_check( const char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int64 type, OBJ_DATA *iobj );
void set_supermob(OBJ_DATA *obj);
bool oprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int64 type);
void rprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int64 type);
void rprog_wordlist_check( const char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, void *vo, int64 type, ROOM_INDEX_DATA *room );

/***************************************************************************
 * Local function code and brief comments.
 */

/* if you dont have these functions, you damn well should... */

#ifdef DUNNO_STRSTR
char * strstr(s1,s2) const char *s1; const char *s2;
{
  char *cp;
  int i,j=strlen(s1)-strlen(s2),k=strlen(s2);
  if(j<0)
    return NULL;
  for(i=0; i<=j && strncmp(s1++,s2, k)!=0; i++);
  return (i>j) ? NULL : (s1-1);
}
#endif

#define RID ROOM_INDEX_DATA

void init_supermob()
{
   RID *office;

   supermob = create_mobile(get_mob_index( 3 ));
   office = get_room_index ( 3 );
   char_to_room( supermob, office );
}


#undef RID


/* Used to get sequential lines of a multi line string (separated by "\n\r")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char *mprog_next_command( char *clist )
{

	char *pointer = clist;

	while ( *pointer != '\n' && *pointer != '\0' )
		pointer++;
	if ( *pointer == '\n' || *pointer == '\r' )
		*pointer++ = '\0';
	if ( *pointer == '\r' || *pointer == '\n' )
		*pointer++ = '\0';

	return ( pointer );

}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval( const char *lhs, char *opr, char *rhs, CHAR_DATA *mob )
{

  if ( !str_cmp( opr, "==" ) )
    return ( bool )( !str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "!=" ) )
    return ( bool )( str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "/" ) )
    return ( bool )( !str_infix( rhs, lhs ) );
  if ( !str_cmp( opr, "!/" ) )
    return ( bool )( str_infix( rhs, lhs ) );

  sprintf( log_buf, "Improper MOBprog operator '%s'", opr );
  progbug( log_buf, mob );
  return 0;

}

bool mprog_veval( int lhs, char *opr, int rhs, CHAR_DATA *mob )
{
  if ( !str_cmp( opr, "==" ) )
    return ( lhs == rhs );
  if ( !str_cmp( opr, "!=" ) )
    return ( lhs != rhs );
  if ( !str_cmp( opr, ">" ) )
    return ( lhs > rhs );
  if ( !str_cmp( opr, "<" ) )
    return ( lhs < rhs );
  if ( !str_cmp( opr, "<=" ) )
    return ( lhs <= rhs );
  if ( !str_cmp( opr, ">=" ) )
    return ( lhs >= rhs );
  if ( !str_cmp( opr, "&" ) )
    return ( lhs & rhs );
  if ( !str_cmp( opr, "|" ) )
    return ( lhs | rhs );

  sprintf( log_buf, "Improper MOBprog operator '%s'", opr );
  progbug( log_buf, mob );

  return false;

}

/*
 * Customizable Flag/Quest Code modified from GenmaC's code - Zoie
 * (This is probably the only thing simple enough to work with Darkstone's code)
 */

#define DT_FLAG_DIR "../flags/"

void AddFlag( const char *flag, CHAR_DATA *player );
bool CheckForFlag( const char *flag, CHAR_DATA *player );

/*
	AddFlag
	Appends the line passed to it to the player's flag file.
	Note that it could duplicate flags if marked more than once,
	so use if hasflag before calling this.
*/
void AddFlag( const char *flag, CHAR_DATA *player )
{
	char flag_file[100];
	FILE * fw;

	strcpy(flag_file, DT_FLAG_DIR);
	strcat(flag_file, player->getName().c_str());
	strcat(flag_file, ".flg");

	fw = NULL;
	fw = fopen(flag_file,"a");

	fprintf(fw, "%s\n",flag);
	printf("stuck %s in %s - correct?\n",flag,flag_file);
	fclose(fw);
}

/*
	CheckForFlag
	finds the flag entry in /flags/player_name.flg, and returns 1 if
	found.
*/
bool CheckForFlag( const char *flag, CHAR_DATA *player )
{
	char flag_file[100];
	char read_string[100];
	bool bFound;
	FILE * read;

	strcpy(flag_file, DT_FLAG_DIR);
	strcat(flag_file, player->getName().c_str());
	strcat(flag_file, ".flg");

	read = fopen(flag_file, "r");

	bFound = 0;

	if(read==NULL)
	{
		return FALSE;
	}
	while(fgets(read_string,99,read) != NULL)
	{
		if(read_string[strlen(read_string)-1] == '\n')
		{
			read_string[strlen(read_string)-1]='\0';
		}

		if( !str_cmp(flag,read_string) )
		{
			bFound = 1;
			break;
		}
	}
	return bFound;
}

/* This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifcheck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return BERR otherwise return boolean 1,0
 * Redone by Altrag.. kill all that big copy-code that performs the
 * same action on each variable..
 */
int mprog_do_ifcheck( char *ifcheck, CHAR_DATA *mob, CHAR_DATA *actor,
		      OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
	char cvar[MAX_INPUT_LENGTH] = "\0";
	char chck[MAX_INPUT_LENGTH] = "\0";
	char opr[MAX_INPUT_LENGTH]  = "\0";
	char rval[MAX_INPUT_LENGTH] = "\0";
	char *point = ifcheck;
	char *pchck = chck;
	CHAR_DATA *chkchar = NULL;
	OBJ_DATA *chkobj = NULL;
	int lhsvl, rhsvl;

	if ( !*point )
	{
		progbug( "Null ifcheck", mob );
		return BERR;
	}
	while ( *point == ' ' )
		point++;
	while ( *point != '(' )
	{
		if ( *point == '\0' )
		{
			progbug( "Ifcheck syntax error", mob );
			return BERR;
		}
		else if ( *point == ' ' )
			point++;
		else
			*pchck++ = *point++;
	}

	*pchck = '\0';
	point++;
	pchck = cvar;
	while ( *point != ')' )
	{
		if ( *point == '\0' )
		{
			progbug( "Ifcheck syntax error", mob );
			return BERR;
		}
		else if ( *point == ' ' )
			point++;
		else
			*pchck++ = *point++;
	}
	point++;

	while ( *point == ' ' )
		point++;
	if ( !*point )
	{
		opr[0] = '\0';
		rval[0] = '\0';
	}
	else
	{
		pchck = opr;
		while ( *point != ' ' && !isalnum(*point) )
			if ( *point == '\0' )
			{
				progbug( "Ifcheck operator without value", mob );
				return BERR;
			}
			else
				*pchck++ = *point++;
		*pchck = '\0';

		while ( *point == ' ' )
			point++;
		pchck = rval;
		while ( *point != '\0' && *point != '\0' )
			*pchck++ = *point++;
		*pchck = '\0';
	}

	/* chck contains check, cvar is the variable in the (), opr is the
	 * operator if there is one, and rval is the value if there was an
	 * operator.
	 */
	if ( cvar[0] == '$' )
	{
		switch(cvar[1])
		{
			case 'i':	chkchar = mob;			break;
			case 'n':	chkchar = actor;		break;
			case 't':	chkchar = (CHAR_DATA *)vo;	break;
			case 'r':	chkchar = rndm;			break;
			case 'o':	chkobj = obj;			break;
			case 'p':	chkobj = (OBJ_DATA *)vo;	break;
			default:
						sprintf(rval, "Bad argument '%c' to '%s'", cvar[0], chck);
						progbug(rval, mob);
						return BERR;
		}
		if ( !chkchar && !chkobj )
			return BERR;
	}

	/* KSILYAN
	 * this is to allow certain ifchecks to work
	 * when called from objects.
	 */

	if ( mob == supermob)
	{
		if (supermob->tempnum == TEMPNUM_OBJ)
		{
			/* This means that we're working from an object.
			 * So, the mob is whoever is holding the object.
			 */
			OBJ_DATA * in_obj;
			OBJ_DATA * obj;
			obj = (OBJ_DATA *) supermob->spare_ptr;

			for (in_obj = obj; in_obj->GetInObj(); in_obj = in_obj->GetInObj())
				;
			mob = obj->GetCarriedBy();
			if (!mob)
				mob = supermob; // just in case
		}
	}

	if ( !str_cmp(chck, "rand") )
	{
		return (number_percent() <= atoi(cvar));
	}

	/*
	   Aiyan, 20/03/05 - roll a dice and check it's value, it was designed for the 0.8%(1/125)
	   drop rate.  Some extensibility can't hurt though
	 */

	if ( !str_cmp(chck, "dice") )
	{
		if ( atoi(cvar) < 1 || atoi(cvar) > 65536 )
		{
			progbug("Dice ifcheck, number is less than 1 or greater than 65536", mob);
			return BERR;
		}

		if ( atoi(rval) > atoi(cvar) )
		{
			progbug("Dice ifcheck, searching for a value greater than the dice sides", mob);
			return BERR;
		}

		if ( atoi(rval) < 1 )
		{
			progbug("Dice ifcheck, searcing for 0 or negative value", mob);
			return BERR;
		}

		return mprog_veval(number_range(0, atoi(cvar)), opr, atoi(rval), mob);
	}

	if( !str_cmp(chck, "hasflag" ) ) 		/* Ifcheck for new flag/quest system */
	{                                   	/* usage: if hasflag($n) == QuestName */
		return(CheckForFlag(rval,chkchar));
	}

	if ( !str_cmp(chck, "mpwalkok" ) )
	{
		return bMpwalkSuccess;
	}
	if ( !str_cmp(chck, "ismpwalking" ) )
	{
		return chkchar ? chkchar->vnum_destination>0 :
			mob->vnum_destination>0;
	}

	if ( !str_cmp(chck, "cansee") )
	{
		if ( !chkchar ) {
			progbug("Cansee: no victim!", mob);
			return BERR;
		}

		return can_see(mob, chkchar);
	}

	if ( !str_cmp(chck, "rarelimit" ) )
	{
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);
		OBJ_INDEX_DATA* pObj;

		pObj = get_obj_index(vnum);

		if ( !pObj ) {
			sprintf(rval, "rarelimit:  vnum %s is invalid!", cvar);
			progbug(rval, mob);
			return BERR;
		}

		if ( pObj->total_count >= pObj->rare && pObj->rare > 0 )
			return 1;
		else
			return 0;
	}

	// Added by Ksilyan, nov-12-2004
	if ( !str_cmp(chck, "isinvfull" ) )
	{
		if ( !chkchar ) {
			progbug("isinvfull: no character!", mob);
			return BERR;
		}

		int ok = 1;

		if ( chkchar->carry_number + (get_obj_number(obj)/obj->count) > can_carry_n( chkchar ) )
			ok = 0;

		if ( chkchar->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_w( chkchar ) )
			ok = 0;

		return ok;
	}

	if ( !str_cmp(chck, "isexit" ) )
	{
		int vdir = atoi(cvar);

		ROOM_INDEX_DATA* room;
		ExitData* exit;

		room = mob->GetInRoom();

		for ( exit = room->first_exit; exit; exit = exit->next ) {
			if ( exit->vdir == vdir ) {
				return 1;
			}
		}

		return 0;
	}


	if ( !str_cmp(chck, "issmell" ) )
	{
		int scent;

		if ( !chkchar )
		{
			progbug("issmell ifcheck: no mob!", mob);
			return BERR;
		}

		scent = atoi(rval);

		if ( scent == chkchar->getScentId() )
		{
			return 1;
		}

		return 0;
	}

	if( !str_cmp(chck, "iscarryingobj") ) {
		int vnum;
		OBJ_DATA *pObj;

		if ( !chkchar ) {
			progbug("iscarryingobj ifcheck: no mob!", mob);
			return BERR;
		}

		vnum = dotted_to_vnum(mob->GetInRoom()->vnum, rval);

		if ( vnum == -1 ) {
			progbug("iscarryingobj ifcheck: rval must be a number!", mob);
			return BERR;
		}

		for ( pObj = chkchar->first_carrying; pObj; pObj = pObj->next_content ) {
			if ( pObj->pIndexData->vnum == vnum ) {
				return 1;
			}
		}
		return 0;
	}
	if ( !str_cmp(chck, "iswearingobj") ) {
		int vnum;
		OBJ_DATA * pObj;

		if ( !chkchar ) {
			progbug("iswearingobj ifcheck: no mob!", mob);
			return BERR;
		}

		vnum = dotted_to_vnum(mob->GetInRoom()->vnum, rval);

		if ( vnum == -1 ) {
			progbug("iswearingobj ifcheck: rval must be a number!", mob);
			return BERR;
		}

		for ( pObj = chkchar->first_carrying; pObj; pObj = pObj->next_content ) {
			if ( pObj->pIndexData->vnum == vnum) {
				if (pObj->wear_loc != -1)
					return 1;
			}
		}
		return 0;
	}
	if ( !str_cmp(chck, "iscarried")) {
		if ( !obj ) {
			progbug("iscarried ifcheck: no object!", mob);
			return BERR;
		}

		if ( obj->GetCarriedBy() ) {
			return 1;
		}

		return 0;
	}
	if ( !str_cmp(chck, "economy") )
	{
		int idx = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);
		ROOM_INDEX_DATA *room;

		if ( !idx )
		{
			if ( !mob->GetInRoom() )
			{
				progbug( "'economy' ifcheck: mob in NULL room with no room vnum "
						"argument", mob );
				return BERR;
			}
			room = mob->GetInRoom();
		}
		else
			room = get_room_index(idx);
		if ( !room )
		{
			progbug( "Bad room vnum passed to 'economy'", mob );
			return BERR;
		}
		return mprog_veval( ((room->area->high_economy > 0) ? 1000000000 : 0)
				+ room->area->low_economy, opr, atoi(rval), mob );
	}
	if ( !str_cmp(chck, "sectormob") )
	{
		if ( !chkchar ) {
			progbug("sectormob ifcheck: no mob!", mob);
			return BERR;
		}

		return mprog_veval( chkchar->GetInRoom()->sector_type, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "sector") )
	{
		ROOM_INDEX_DATA * iRoom;

		iRoom = get_room_index(dotted_to_vnum(mob->GetInRoom()->vnum, cvar));

		if ( !iRoom)
		{
			progbug("Invalid room vnum passed to sector", mob);
			return BERR;
		}

		return mprog_veval( iRoom->sector_type, opr, atoi(rval),
				mob );
	}
	if ( !str_cmp(chck, "numinroom") )
	{
		ROOM_INDEX_DATA* iRoom;
		CHAR_DATA      * vch;
		int count = 0;

		iRoom = get_room_index(dotted_to_vnum(mob->GetInRoom()->vnum, cvar));

		if ( !iRoom ) {
			progbug("Invalid room vnum passed to numinroom", mob);
			return BERR;
		}

		for ( vch = iRoom->first_person; vch; vch = vch->next_in_room )
		{
			if ( IS_NPC(vch) && vch->pIndexData->vnum == 3 ) {
				continue;
			}
			++count;
		}

		return mprog_veval(count, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "num_pc_inarea") )
	{
		int count = 0;
		count = mob->GetInRoom()->area->nplayer;
		return mprog_veval(count, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "num_pc_inroom") )
	{
		ROOM_INDEX_DATA* iRoom;
		CHAR_DATA      * vch;
		int count = 0;

		iRoom = get_room_index(dotted_to_vnum(mob->GetInRoom()->vnum, cvar));

		if ( !iRoom ) {
			progbug("Invalid room vnum passed to num_pc_inroom", mob);
			return BERR;
		}

		for ( vch = iRoom->first_person; vch; vch = vch->next_in_room )
		{
			if ( !IS_NPC(vch) ){
				++count;
			}
		}

		return mprog_veval(count, opr, atoi(rval), mob);
	}

	if ( !str_cmp(chck, "num_npc_inroom") )
	{
		ROOM_INDEX_DATA* iRoom;
		CHAR_DATA      * vch;
		int count = 0;

		iRoom = get_room_index(dotted_to_vnum(mob->GetInRoom()->vnum, cvar));

		if ( !iRoom ) {
			progbug("Invalid room vnum passed to num_npc_inroom", mob);
			return BERR;
		}

		for ( vch = iRoom->first_person; vch; vch = vch->next_in_room )
		{
			if ( IS_NPC(vch) && vch->pIndexData->vnum != 3) {
				++count;
			}
		}

		return mprog_veval(count, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "mobinroom") )
	{
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);
		int lhsvl;
		CHAR_DATA *oMob;

		if ( vnum < 1 || vnum > 1048576000 )
		{
			char buf[255];
			sprintf(buf, "Bad vnum (%s) to 'mobinroom'", cvar);
			progbug( buf, mob );
			return BERR;
		}
		lhsvl = 0;
		for ( oMob = mob->GetInRoom()->first_person; oMob;
				oMob = oMob->next_in_room )
		{
			if ( IS_NPC(oMob) && oMob->pIndexData->vnum == vnum )
			{
				lhsvl++;
			}
		}
		rhsvl = atoi(rval);
		if ( rhsvl < 1 ) rhsvl = 1;
		if ( !*opr )
			strcpy( opr, "==" );
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "isworn" ) )
	{
		OBJ_DATA* pObj;
		int count = 0;
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);

		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next ) {
			if ( pObj->pIndexData->vnum == vnum || !is_name_prefix(cvar, pObj->name_.c_str()) ) {
				if ( pObj->wear_loc == WEAR_NONE )
					++count;
			}
		}

		return mprog_veval(count, opr, atoi(rval), mob);
	}

	if ( !str_cmp(chck, "time_hour" ) )
	{
		return mprog_veval(time_info.hour, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "date_day" ) )
	{
		return mprog_veval(time_info.day, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "date_month" ) )
	{
		return mprog_veval(time_info.month, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "date_year" ) )
	{
		return mprog_veval(time_info.year, opr, atoi(rval), mob);
	}

	if ( !str_cmp(chck, "ininvent" ) )
	{
		OBJ_DATA* pObj;
		int count = 0;
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);

		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next ) {
			if ( pObj->pIndexData->vnum == vnum || !is_name_prefix(cvar, pObj->name_.c_str()) ) {
				++count;
			}
		}

		return mprog_veval(count, opr, atoi(rval), mob);
	}

	if ( !str_cmp(chck, "timeskilled") )
	{
		MOB_INDEX_DATA *pMob;

		if ( chkchar )
			pMob = chkchar->pIndexData;
		else if ( !(pMob = get_mob_index(dotted_to_vnum(mob->GetInRoom()->vnum, cvar))))
		{
			progbug("TimesKilled ifcheck: bad vnum", mob);
			return BERR;
		}
		return mprog_veval(pMob->killed, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "ovnumhere") )
	{
		OBJ_DATA *pObj;
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);

		if ( vnum < 1 || vnum > 1048576000 )
		{
			progbug("OvnumHere: bad vnum", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
			if ( can_see_obj(mob, pObj) && pObj->pIndexData->vnum == vnum )
				lhsvl++;
		for ( pObj = mob->GetInRoom()->first_content; pObj;
				pObj = pObj->next_content )
			if ( can_see_obj(mob, pObj) && pObj->pIndexData->vnum == vnum )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "otypehere") )
	{
		OBJ_DATA *pObj;
		int64 type;

		if ( is_number(cvar) )
			type = atoi(cvar);
		else
			type = itemtype_name_to_number(cvar);
		if ( type < 0 || type > MAX_ITEM_TYPE )
		{
			progbug("OtypeHere: bad type", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
			if ( can_see_obj(mob, pObj) && pObj->item_type == type )
				lhsvl++;
		for ( pObj = mob->GetInRoom()->first_content; pObj;
				pObj = pObj->next_content )
			if ( can_see_obj(mob, pObj) && pObj->item_type == type )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "ovnumroom") )
	{
		OBJ_DATA *pObj;
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);

		if ( vnum < 1 || vnum > 1048576000 )
		{
			progbug("OvnumRoom: bad vnum", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->GetInRoom()->first_content; pObj;
				pObj = pObj->next_content )
			if ( can_see_obj(mob, pObj) && pObj->pIndexData->vnum == vnum )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "otyperoom") )
	{
		OBJ_DATA *pObj;
		int64 type;

		if ( is_number(cvar) )
			type = atoi(cvar);
		else
			type = itemtype_name_to_number(cvar);
		if ( type < 0 || type > MAX_ITEM_TYPE )
		{
			progbug("OtypeRoom: bad type", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->GetInRoom()->first_content; pObj;
				pObj = pObj->next_content )
			if ( can_see_obj(mob, pObj) && pObj->item_type == type )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "ovnumcarry") )
	{
		OBJ_DATA *pObj;
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);

		if ( vnum < 1 || vnum > 1048576000 )
		{
			progbug("OvnumCarry: bad vnum", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
		{
			if ( can_see_obj(mob, pObj) && pObj->pIndexData->vnum == vnum )
				lhsvl++;
		}
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 0 )
		{
			progbug("OvnumCarry: bad right-hand side value. Using 0 instead.", mob);
			rhsvl = 0;
		}
		if ( !*opr )
		{
			progbug("OvnumCarry: bad operator. Using == instead.", mob);
			strcpy(opr, "==");
		}


		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "otypecarry") )
	{
		OBJ_DATA *pObj;
		int64 type;

		if ( is_number(cvar) )
			type = atoi(cvar);
		else
			type = itemtype_name_to_number(cvar);
		if ( type < 0 || type > MAX_ITEM_TYPE )
		{
			progbug("OtypeCarry: bad type", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
			if ( can_see_obj(mob, pObj) && pObj->item_type == type )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "ovnumwear") )
	{
		OBJ_DATA *pObj;
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);

		if ( vnum < 1 || vnum > 1048576000 )
		{
			progbug("OvnumWear: bad vnum", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
			if ( pObj->wear_loc != WEAR_NONE && can_see_obj(mob, pObj) &&
					pObj->pIndexData->vnum == vnum )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "otypewear") )
	{
		OBJ_DATA *pObj;
		int64 type;

		if ( is_number(cvar) )
			type = atoi(cvar);
		else
			type = itemtype_name_to_number(cvar);
		if ( type < 0 || type > MAX_ITEM_TYPE )
		{
			progbug("OtypeWear: bad type", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
			if ( pObj->wear_loc != WEAR_NONE && can_see_obj(mob, pObj) &&
					pObj->item_type == type )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "ovnuminv") )
	{
		OBJ_DATA *pObj;
		int vnum = dotted_to_vnum(mob->GetInRoom()->vnum, cvar);

		if ( vnum < 1 || vnum > 1048576000 )
		{
			progbug("OvnumInv: bad vnum", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
			if ( pObj->wear_loc == WEAR_NONE && can_see_obj(mob, pObj) &&
					pObj->pIndexData->vnum == vnum )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "otypeinv") )
	{
		OBJ_DATA *pObj;
		int64 type;

		if ( is_number(cvar) )
			type = atoi(cvar);
		else
			type = itemtype_name_to_number(cvar);
		if ( type < 0 || type > MAX_ITEM_TYPE )
		{
			progbug("OtypeInv: bad type", mob);
			return BERR;
		}
		lhsvl = 0;
		for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
			if ( pObj->wear_loc == WEAR_NONE && can_see_obj(mob, pObj) &&
					pObj->item_type == type )
				lhsvl++;
		rhsvl = is_number(rval) ? atoi(rval) : -1;
		if ( rhsvl < 1 )
			rhsvl = 1;
		if ( !*opr )
			strcpy(opr, "==");
		return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( chkchar )
	{
		if ( !str_cmp(chck, "ismobinvis") )
		{
			return (IS_NPC(chkchar) && IS_SET(chkchar->act, ACT_MOBINVIS));
		}
		if ( !str_cmp(chck, "mobinvislevel") )
		{
			return (IS_NPC(chkchar) ?
					mprog_veval(chkchar->mobinvis, opr, atoi(rval), mob) : FALSE);
		}
		if ( !str_cmp(chck, "ispc") )
		{
			return IS_NPC(chkchar) ? FALSE : TRUE;
		}
		if ( !str_cmp(chck, "isnpc") )
		{
			return IS_NPC(chkchar) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "ispkill") )
		{
			progbug("ispkill ifcheck -- no longer valid!", actor);
			return FALSE;
		}
		if ( !str_cmp(chck, "isdevoted") )
		{
			return IS_DEVOTED(chkchar) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "canpkill") )
		{
			progbug("canpkill ifcheck -- no longer valid!", actor);
			return FALSE;
		}
		if ( !str_cmp(chck, "ismounted") )
		{
			return (chkchar->position == POS_MOUNTED);
		}
		if ( !str_cmp(chck, "isgood") )
		{
			return IS_GOOD(chkchar) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "isneutral") )
		{
			return IS_NEUTRAL(chkchar) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "isevil") )
		{
			return IS_EVIL(chkchar) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "isfight") )
		{
			return chkchar->IsFighting() ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "isimmort") )
		{
			return (get_trust(chkchar) >= LEVEL_IMMORTAL);
		}
		if ( !str_cmp(chck, "isbuilder") )
		{
			return(!IS_NPC(chkchar)
					&& chkchar->pcdata
					&& chkchar->pcdata->area
					&& chkchar->pcdata->area->low_r_vnum
				  );
		}
		if ( !str_cmp(chck, "ischarmed") )
		{
			return IS_AFFECTED(chkchar, AFF_CHARM) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "isfollow") )
		{
			return (chkchar->GetMaster() != NULL &&
					chkchar->GetMaster()->GetInRoom() == chkchar->GetInRoom());
		}
		if ( !str_cmp(chck, "isaffected") )
		{
			int value = get_aflag(rval);

			if ( value < 0 || value > 31 )
			{
				progbug("Unknown affect being checked", mob);
				return BERR;
			}
			return IS_AFFECTED(chkchar, 1 << value) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "hasactflag"))
		{
			int value;
			bool pcflag = FALSE;

			if ( IS_NPC(chkchar) ) {
				value = get_actflag(rval);
			} else {
				value = get_plrflag(rval);

				if ( value < 0 || value > 31 )
				{
					value = get_pcflag(rval);
					pcflag = 1;
				}
			}

			if ( value < 0 || value > 31 )
			{
				progbug("Unknown flag being checked", mob);
				return BERR;
			}

			if ( IS_NPC(chkchar) || !pcflag ) {
				return IS_SET(chkchar->act, 1<<value) ? TRUE : FALSE;
			} else {
				return IS_SET(chkchar->pcdata->flags, 1<<value) ? TRUE : FALSE;
			}
		}

		if ( !str_cmp(chck, "isresistant" ) )
		{
			int value = get_risflag(rval);

			if ( value < 0 || value > 31 )
			{
				progbug("isresistant -- unknown flag", mob);
				return BERR;
			}

			return IS_SET(chkchar->resistant, 1 << value) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "isimmune" ) )
		{
			int value = get_risflag(rval);

			if ( value < 0 || value > 31 )
			{
				progbug("isimmune -- unknown flag", mob);
				return BERR;
			}

			return IS_SET(chkchar->immune, 1 << value) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "issusceptible" ) )
		{
			int value = get_risflag(rval);

			if ( value < 0 || value > 31 )
			{
				progbug("issusceptible -- unknown flag", mob);
				return BERR;
			}

			return IS_SET(chkchar->susceptible, 1 << value) ? TRUE : FALSE;
		}

		if ( !str_cmp(chck, "hitprcnt") )
		{
			return mprog_veval(chkchar->hit/chkchar->max_hit, opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "inroom") )
		{
			return mprog_veval(chkchar->GetInRoom()->vnum, opr, dotted_to_vnum(chkchar->GetInRoom()->vnum, rval), mob);
		}
		if ( !str_cmp(chck, "wasinroom") )
		{
			if (!chkchar->GetWasInRoom())
			{
				progbug("wasinroom -- null previous room! Does the mob ever move?", mob);
				return BERR;
			}
			return mprog_veval(chkchar->GetWasInRoom()->vnum, opr, dotted_to_vnum(chkchar->GetInRoom()->vnum, rval), mob);
		}
		if ( !str_cmp(chck, "norecall") )
		{
			return IS_SET(chkchar->GetInRoom()->room_flags, ROOM_NO_RECALL) ? TRUE : FALSE;
		}
		if ( !str_cmp(chck, "sex") )
		{
			return mprog_veval(chkchar->sex, opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "position") )
		{
			return mprog_veval(chkchar->position, opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "doingquest") )
		{
			return IS_NPC(actor) ? FALSE :
				mprog_veval(chkchar->pcdata->quest_number, opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "ishelled") )
		{
			return IS_NPC(actor) ? FALSE :
				mprog_veval(chkchar->pcdata->secReleaseDate, opr, atoi(rval), mob);
		}

		if ( !str_cmp(chck, "level") )
		{
			return mprog_veval(get_trust(chkchar), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "goldamt") )
		{
			return mprog_veval(chkchar->gold, opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "class") )
		{
			if ( IS_NPC(chkchar) )
				return mprog_seval(npc_class[chkchar->Class], opr, rval, mob);
			return mprog_seval((char *)class_table[chkchar->Class]->whoName_.c_str(), opr,
					rval, mob);
		}
		if ( !str_cmp(chck, "vnum") )
		{
			if ( IS_NPC(chkchar) ) {
				return mprog_veval(chkchar->pIndexData->vnum, opr, dotted_to_vnum(mob->GetInRoom()->vnum, rval), mob);
			} else {
				return mprog_veval(0, opr, dotted_to_vnum(mob->GetInRoom()->vnum, rval), mob);
			}
		}
		if ( !str_cmp(chck, "race") )
		{
			if ( IS_NPC(chkchar) )
				return mprog_seval(npc_race[chkchar->race], opr, rval, mob);
			return mprog_seval((char *)race_table[chkchar->race].race_name, opr,
					rval, mob);
		}
		// Ksilyan:
		if ( !str_cmp(chck, "area") )
		{
			return mprog_seval(chkchar->GetInRoom()->area->filename, opr, rval, mob);
		}
		if ( !str_cmp(chck, "clan") )
		{
			if ( IS_NPC(chkchar) || !chkchar->pcdata->clan )
				return FALSE;
			return mprog_seval(chkchar->pcdata->clan->name_.c_str(), opr, rval, mob);
		}
		if ( !str_cmp(chck, "deity") )
		{
			if (IS_NPC(chkchar) || !chkchar->pcdata->deity )
				return FALSE;
			return mprog_seval(chkchar->pcdata->deity->name_.c_str(), opr, rval, mob);
		}
		if ( !str_cmp(chck, "guild") )
		{
			if ( IS_NPC(chkchar) || !IS_GUILDED(chkchar) )
				return FALSE;
			return mprog_seval(chkchar->pcdata->clan->name_.c_str(), opr, rval, mob);
		}
		if ( !str_cmp(chck, "council") )
		{
			if (IS_NPC(chkchar) || !chkchar->pcdata->council)
				return(FALSE);
			return(mprog_seval(chkchar->pcdata->council->name_.c_str(), opr, rval, mob));
		}
		if ( !str_cmp(chck, "clantype") )
		{
			if ( IS_NPC(chkchar) || !chkchar->pcdata->clan )
				return FALSE;
			return mprog_veval(chkchar->pcdata->clan->clan_type, opr, atoi(rval),
					mob);
		}
		if ( !str_cmp(chck, "favor") )
		{
			return mprog_veval(chkchar->pcdata->favor, opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "str") )
		{
			return mprog_veval(chkchar->getStr(), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "wis") )
		{
			return mprog_veval(chkchar->getWis(), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "int") )
		{
			return mprog_veval(chkchar->getInt(), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "dex") )
		{
			return mprog_veval(chkchar->getDex(), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "con") )
		{
			return mprog_veval(chkchar->getCon(), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "cha") )
		{
			return mprog_veval(chkchar->getCha(), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "lck") )
		{
			return mprog_veval(chkchar->getLck(), opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "iswantedincity") ) // Added by Ksilyan
		{
			if (IS_NPC(chkchar))
				return BERR;
			if (atoi(rval) < 0)
				return BERR;

			if (atoi(rval) >= MAX_CITIES) {
				sprintf(log_buf, "Invalid city number '%d'", atoi(rval) );
				progbug(log_buf, mob);
				return BERR;
			}

			return mprog_veval(chkchar->pcdata->CityWanted[atoi(rval)], opr, 1, mob);
		}
		if ( !str_cmp(chck, "isthiefincity") )   // Added by Ksilyan
		{
			if (IS_NPC(chkchar))
				return BERR;

			if (atoi(rval) < 0 )
				return BERR;

			if (atoi(rval) >= MAX_CITIES) {
				sprintf( log_buf, "Invalid city number '%d'", atoi(rval) );
				log_string( log_buf );
				return BERR;
			}
			return mprog_veval(chkchar->pcdata->citythief[atoi(rval)], opr, 1, mob);
		}
	}
	if ( chkobj )
	{
		if ( !str_cmp(chck, "objtype") )
		{
			return mprog_veval(chkobj->item_type, opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "objval0") )
		{
			return mprog_veval(chkobj->value[0], opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "objval1") )
		{
			return mprog_veval(chkobj->value[1], opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "objval2") )
		{
			return mprog_veval(chkobj->value[2], opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "objval3") )
		{
			return mprog_veval(chkobj->value[3], opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "objval4") )
		{
			return mprog_veval(chkobj->value[4], opr, atoi(rval), mob);
		}
		if ( !str_cmp(chck, "objval5") )
		{
			return mprog_veval(chkobj->value[5], opr, atoi(rval), mob);
		}
	}
	/* The following checks depend on the fact that cval[1] can only contain
	   one character, and that NULL checks were made previously. */
	if ( !str_cmp(chck, "number") )
	{
		if ( chkchar )
		{
			if ( !IS_NPC(chkchar) )
				return FALSE;
			lhsvl = (chkchar == mob) ? chkchar->gold : chkchar->pIndexData->vnum;
			return mprog_veval(lhsvl, opr, atoi(rval), mob);
		}
		return mprog_veval(chkobj->pIndexData->vnum, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "name") )
	{
		if ( chkchar )
			return mprog_seval(chkchar->getName().c_str(), opr, rval, mob);
		return mprog_seval(chkobj->name_.c_str(), opr, rval, mob);
	}

	/* Ok... all the ifchecks are done, so if we didnt find ours then something
	 * odd happened.  So report the bug and abort the MUDprogram (return error)
	 */
	progbug( "Unknown ifcheck", mob );
	return BERR;
}


/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */

/*
 *  There's no reason to make the mud crash when a variable's
 *  fubared.  I added some ifs.  I'm willing to trade some
 *  performance for stability. -Haus
 *
 *  Narn's fubar ***ANNIHILATES*** you!  Hmm, could we add that
 *  as a weapon type? -Narn
 *
 *  Added char_died and obj_extracted checks	-Thoric
 */
void mprog_translate( char ch, char *t, CHAR_DATA *mob, CHAR_DATA *actor,
                    OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
	static const char *he_she        [] = { "it",  "he",  "she" };
	static const char *him_her       [] = { "it",  "him", "her" };
	static const char *his_her       [] = { "its", "his", "her" };
	CHAR_DATA   *vict             = (CHAR_DATA *) vo;
	OBJ_DATA    *v_obj            = (OBJ_DATA  *) vo;

	*t = '\0';
	switch ( ch )
	{
		case 'a':
			if ( obj && !obj_extracted(obj) )
				strcpy( t, aoran(obj->name_.c_str()) );
			else
				strcpy( t, "a" );
			break;

		case 'A':
			if ( v_obj && !obj_extracted(v_obj) )
				strcpy( t, aoran(v_obj->name_.c_str()) );
			else
				strcpy( t, "a" );
			break;

		case 'c':
			if ( obj && obj->GetCarriedBy() )
			{
				strcpy(t, NAME(obj->GetCarriedBy()));
			}
			else {
				strcpy(t, " <@@@> ");
			}
			break;

		case 'e':
			if ( actor && !char_died(actor) )
			{
				can_see( mob, actor ) ? strcpy( t, he_she[ actor->sex ] )
					: strcpy( t, "someone" );
			}
			else
				strcpy( t, "it" );
			break;

		case 'E':
			if ( vict && !char_died(vict) )
			{
				can_see( mob, vict ) ? strcpy( t, he_she[ vict->sex ] )
					: strcpy( t, "someone" );
			}
			else
				strcpy( t, "it" );
			break;

		case 'f': // Ksilyan: who is the actor fighting?
			if ( actor && !char_died(actor) )
			{
				if (actor->GetVictim() != NULL )
					one_argument(actor->GetVictim()->getName().c_str(), t );
				else
					strcpy(t, "someone" );
			}
			else
				strcpy(t, "someone");
			break;

		case 'F': // Ksilyan: who is the actor following?
			if ( actor && actor->GetMaster() )
				one_argument(actor->GetMaster()->getName().c_str(), t );
			else
				strcpy(t, "someone");
			break;

		case 'i': // name of mob running the program
			if ( mob && !char_died(mob) )
			{
				if (mob->getName().length() > 0)
					one_argument( mob->getName().c_str(), t );
			} else
				strcpy( t, "someone" );
			break;

		case 'I':
			if ( mob && !char_died(mob) )
			{
				if (mob->getShort().length() > 0)
				{
					strcpy( t, mob->getShort().c_str() );
				} else {
					strcpy( t, "someone" );
				}
			} else
				strcpy( t, "someone" );
			break;

		case 'j':
			if (mob && !char_died(mob))
			{
				strcpy( t, he_she[ mob->sex ] );
			} else {
				strcpy( t, "it" );
			}
			break;

		case 'J':
			if ( rndm && !char_died(rndm) )
			{
				can_see( mob, rndm ) ? strcpy( t, he_she[ rndm->sex ] )
					: strcpy( t, "someone" );
			}
			else
				strcpy( t, "it" );
			break;

		case 'k':
			if( mob && !char_died(mob) )
			{
				strcpy( t, him_her[ mob->sex ] );
			} else {
				strcpy( t, "it" );
			}
			break;

		case 'K':
			if ( rndm && !char_died(rndm) )
			{
				can_see( mob, rndm ) ? strcpy( t, him_her[ rndm->sex ] )
					: strcpy( t, "someone's" );
			}
			else
				strcpy( t, "its'" );
			break;

		case 'l':
			if( mob && !char_died(mob) )
			{
				strcpy( t, his_her[ mob->sex ] );
			} else {
				strcpy( t, "it" );
			}
			break;

		case 'L':
			if ( rndm && !char_died(rndm) )
			{
				can_see( mob, rndm ) ? strcpy( t, his_her[ rndm->sex ] )
					: strcpy( t, "someone" );
			}
			else
				strcpy( t, "its" );
			break;

		case 'm':
			if ( actor && !char_died(actor) )
			{
				can_see( mob, actor ) ? strcpy( t, him_her[ actor->sex ] )
					: strcpy( t, "someone" );
			}
			else
				strcpy( t, "it" );
			break;

		case 'M':
			if ( vict && !char_died(vict) )
			{
				can_see( mob, vict ) ? strcpy( t, him_her[ vict->sex ] )
					: strcpy( t, "someone" );
			}
			else
				strcpy( t, "it" );
			break;

		case 'n':
			if ( actor && !char_died(actor) )
			{
				if ( can_see( mob,actor ) )
					one_argument( actor->getName().c_str(), t );
				if ( !IS_NPC( actor ) )
					*t = UPPER( *t );
			}
			else
				strcpy( t, "someone" );
			break;

		case 'N':
			if ( vict && !char_died(vict) )
			{
				if ( can_see( mob, vict ) )
					one_argument( vict->getName().c_str(), t );
				if ( !IS_NPC( vict ) )
					*t = UPPER( *t );
			}
			else
				strcpy( t, "someone" );

			break;

		case 'o':
			if ( obj && !obj_extracted(obj) )
			{
				can_see_obj( mob, obj ) ? one_argument( obj->name_.c_str(), t )
					: strcpy( t, "something" );
			}
			else
				strcpy( t, "something" );
			break;

		case 'O':
			if ( obj && !obj_extracted(obj) )
			{
				can_see_obj( mob, obj ) ? strcpy( t, obj->shortDesc_.c_str() )
					: strcpy( t, "something" );
			}
			else
				strcpy( t, "something" );
			break;

		case 'p':
			if ( v_obj && !obj_extracted(v_obj) )
			{
				can_see_obj( mob, v_obj ) ? one_argument( v_obj->name_.c_str(), t )
					: strcpy( t, "something" );
			}
			else
				strcpy( t, "something" );
			break;

		case 'P':
			if ( v_obj && !obj_extracted(v_obj) )
			{
				can_see_obj( mob, v_obj ) ? strcpy( t, v_obj->shortDesc_.c_str() )
					: strcpy( t, "something" );
			}
			else
				strcpy( t, "something" );
			break;

		case 'r':
			if ( rndm && !char_died(rndm) )
			{
				if ( can_see( mob, rndm ) )
				{
					one_argument( rndm->getName().c_str(), t );
				}
				if ( !IS_NPC( rndm ) )
				{
					*t = UPPER( *t );
				}
			}
			else
				strcpy( t, "someone" );
			break;

		case 'R':
			if ( rndm && !char_died(rndm) )
			{
				if ( can_see( mob, rndm ) )
					if ( IS_NPC( rndm ) )
						strcpy(t,rndm->getShort().c_str() );
					else
					{
						strcpy( t, rndm->getShort().c_str() );
						strcat( t, player_title(rndm->pcdata->title_.c_str()) );
					}
				else
					strcpy( t, "someone" );
			}
			else
				strcpy( t, "someone" );
			break;

		case 's':
			if ( actor && !char_died(actor) )
			{
				can_see( mob, actor ) ? strcpy( t, his_her[ actor->sex ] )
					: strcpy( t, "someone's" );
			}
			else
				strcpy( t, "its'" );
			break;
		case 'S':
			if ( vict && !char_died(vict) )
			{
				can_see( mob, vict ) ? strcpy( t, his_her[ vict->sex ] )
					: strcpy( t, "someone's" );
			}
			else
				strcpy( t, "its'" );
			break;

		case 't':
			if ( actor && !char_died(actor) )
			{
				if ( can_see( mob, actor ) )
					if ( IS_NPC( actor ) )
						strcpy( t, actor->getShort().c_str() );
					else
					{
						strcpy( t, actor->getShort().c_str() );
						strcat( t, player_title(actor->pcdata->title_.c_str()) );
					}
				else
					strcpy( t, "someone" );
			}
			else
				strcpy( t, "someone" );
			break;

		case 'T':
			if ( vict && !char_died(vict) )
			{
				if ( can_see( mob, vict ) )
					if ( IS_NPC( vict ) )
						strcpy( t, vict->getShort().c_str());
					else
					{
						strcpy( t, vict->getShort().c_str() );
						strcat( t, player_title(vict->pcdata->title_.c_str()) );
					}
				else
					strcpy( t, "someone" );
			}
			else
				strcpy( t, "someone" );
			break;

		case '$':
			strcpy( t, "$" );
			break;

		default:
			progbug( "Bad $var", mob );
			break;
	}

	return;

}

/*  The main focus of the MOBprograms.  This routine is called
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 *
 *  This function rewritten by Narn for Realms of Despair, Dec/95.
 *
 */
void mprog_driver ( char *com_list, CHAR_DATA *mob, CHAR_DATA *actor,
		   OBJ_DATA *obj, void *vo, bool single_step)
{
	char tmpcmndlst[ MAX_STRING_LENGTH ];
	char *command_list;
	char *cmnd;
	CHAR_DATA *rndm  = NULL;
	CHAR_DATA *vch   = NULL;
	int count        = 0;
	int ignorelevel  = 0;
	int iflevel, result;
	bool ifstate[MAX_IFS][ DO_ELSE + 1 ];
	static int prog_nest;

	if IS_AFFECTED( mob, AFF_CHARM )
		return;

	/* Next couple of checks stop program looping. -- Altrag */
	if ( mob == actor )
	{
		progbug( "triggering oneself.", mob );
		return;
	}

	if ( ++prog_nest > MAX_PROG_NEST )
	{
		progbug( "max_prog_nest exceeded.", mob );
		--prog_nest;
		return;
	}

	/* Make sure all ifstate bools are set to FALSE */
	for ( iflevel = 0; iflevel < MAX_IFS; iflevel++ )
	{
		for ( count = 0; count < DO_ELSE; count++ )
		{
			ifstate[iflevel][count] = FALSE;
		}
	}

	iflevel = 0;

	/*
	 * get a random visible player who is in the room with the mob.
	 *
	 *  If there isn't a random player in the room, rndm stays NULL.
	 *  If you do a $r, $R, $j, or $k with rndm = NULL, you'll crash
	 *  in mprog_translate.
	 *
	 *  Adding appropriate error checking in mprog_translate.
	 *    -Haus
	 *
	 * This used to ignore players MAX_LEVEL - 3 and higher (standard
	 * Merc has 4 immlevels).  Thought about changing it to ignore all
	 * imms, but decided to just take it out.  If the mob can see you,
	 * you may be chosen as the random player. -Narn
	 *
	 */

	count = 0;
	for ( vch = mob->GetInRoom()->first_person; vch; vch = vch->next_in_room )
		if ( !IS_NPC( vch )
				&&  can_see( mob, vch ) )
		{
			if ( number_range( 0, count ) == 0 )
				rndm = vch;
			count++;
		}

	strcpy( tmpcmndlst, com_list );
	command_list = tmpcmndlst;
	if ( single_step )
	{
		if ( mob->mpscriptpos > (int) strlen( tmpcmndlst ) )
			mob->mpscriptpos = 0;
		else
			command_list += mob->mpscriptpos;
		if ( *command_list == '\0' )
		{
			command_list = tmpcmndlst;
			mob->mpscriptpos = 0;
		}
	}

	/* From here on down, the function is all mine.  The original code
	   did not support nested ifs, so it had to be redone.  The max
	   logiclevel (MAX_IFS) is defined at the beginning of this file,
	   use it to increase/decrease max allowed nesting.  -Narn
	 */

	while ( TRUE )
	{
		/* With these two lines, cmnd becomes the current line from the prog,
		   and command_list becomes everything after that line. */
		cmnd         = command_list;
		command_list = mprog_next_command( command_list );

		/* Are we at the end? */
		if ( cmnd[0] == '\0' )
		{
			if ( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] )
			{
				progbug( "Missing endif", mob );
			}
			--prog_nest;
			return;
		}

		/* Evaluate/execute the command, check what happened. */
		result = mprog_do_command( cmnd, mob, actor, obj, vo, rndm,
				( ifstate[iflevel][IN_IF] && !ifstate[iflevel][DO_IF] )
				|| ( ifstate[iflevel][IN_ELSE] && !ifstate[iflevel][DO_ELSE] ),
				( ignorelevel > 0 ) );

		/* Script prog support  -Thoric */
		if ( single_step )
		{
			mob->mpscriptpos = command_list - tmpcmndlst;
			--prog_nest;
			return;
		}

		/* This is the complicated part.  Act on the returned value from
		   mprog_do_command according to the current logic state. */
		switch ( result )
		{
			case COMMANDOK:
#ifdef DEBUG
				log_string( "COMMANDOK" );
#endif
				/* Ok, this one's a no-brainer. */
				continue;
				break;

			case IFTRUE:
#ifdef DEBUG
				log_string( "IFTRUE" );
#endif
				/* An if was evaluated and found true.  Note that we are in an
				   if section and that we want to execute it. */
				iflevel++;
				if ( iflevel == MAX_IFS )
				{
					progbug( "Maximum nested ifs exceeded", mob );
					--prog_nest;
					return;
				}

				ifstate[iflevel][IN_IF] = TRUE;
				ifstate[iflevel][DO_IF] = TRUE;
				break;

			case IFFALSE:
#ifdef DEBUG
				log_string( "IFFALSE" );
#endif
				/* An if was evaluated and found false.  Note that we are in an
				   if section and that we don't want to execute it unless we find
				   an or that evaluates to true. */
				iflevel++;
				if ( iflevel == MAX_IFS )
				{
					progbug( "Maximum nested ifs exceeded", mob );
					--prog_nest;
					return;
				}
				ifstate[iflevel][IN_IF] = TRUE;
				ifstate[iflevel][DO_IF] = FALSE;
				break;

			case ORTRUE:
#ifdef DEBUG
				log_string( "ORTRUE" );
#endif
				/* An or was evaluated and found true.  We should already be in an
				   if section, so note that we want to execute it. */
				if ( !ifstate[iflevel][IN_IF] )
				{
					progbug( "Unmatched or", mob );
					--prog_nest;
					return;
				}
				ifstate[iflevel][DO_IF] = TRUE;
				break;

			case ORFALSE:
#ifdef DEBUG
				log_string( "ORFALSE" );
#endif
				/* An or was evaluated and found false.  We should already be in an
				   if section, and we don't need to do much.  If the if was true or
				   there were/will be other ors that evaluate(d) to true, they'll set
				   do_if to true. */
				if ( !ifstate[iflevel][IN_IF] )
				{
					progbug( "Unmatched or", mob );
					--prog_nest;
					return;
				}
				continue;
				break;

				// Added in by Ksilyan
			case ANDTRUE:
				if ( !ifstate[iflevel][IN_IF] )
				{
					progbug("Unmatched and", mob);
					--prog_nest;
					return;
				}
				break;

			case ANDFALSE:
				if ( !ifstate[iflevel][IN_IF] )
				{
					progbug("Unmatched and", mob);
					--prog_nest;
					return;
				}
				ifstate[iflevel][DO_IF] = FALSE;
				break;

			case FOUNDELSE:
#ifdef DEBUG
				log_string( "FOUNDELSE" );
#endif
				/* Found an else.  Make sure we're in an if section, bug out if not.
				   If this else is not one that we wish to ignore, note that we're now
				   in an else section, and look at whether or not we executed the if
				   section to decide whether to execute the else section.  Ca marche
				   bien. */
				if ( ignorelevel > 0 )
					continue;

				if ( ifstate[iflevel][IN_ELSE] )
				{
					progbug( "Found else in an else section", mob );
					--prog_nest;
					return;
				}
				if ( !ifstate[iflevel][IN_IF] )
				{
					progbug( "Unmatched else", mob );
					--prog_nest;
					return;
				}

				ifstate[iflevel][IN_ELSE] = TRUE;
				ifstate[iflevel][DO_ELSE] = !ifstate[iflevel][DO_IF];
				ifstate[iflevel][IN_IF]   = FALSE;
				ifstate[iflevel][DO_IF]   = FALSE;

				break;

			case FOUNDENDIF:
#ifdef DEBUG
				log_string( "FOUNDENDIF" );
#endif
				/* Hmm, let's see... FOUNDENDIF must mean that we found an endif.
				   So let's make sure we were expecting one, return if not.  If this
				   endif matches the if or else that we're executing, note that we are
				   now no longer executing an if.  If not, keep track of what we're
				   ignoring. */
				if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
				{
					progbug( "Unmatched endif", mob );
					--prog_nest;
					return;
				}

				if ( ignorelevel > 0 )
				{
					ignorelevel--;
					continue;
				}

				ifstate[iflevel][IN_IF]   = FALSE;
				ifstate[iflevel][DO_IF]   = FALSE;
				ifstate[iflevel][IN_ELSE] = FALSE;
				ifstate[iflevel][DO_ELSE] = FALSE;

				iflevel--;
				break;

			case IFIGNORED:
#ifdef DEBUG
				log_string( "IFIGNORED" );
#endif
				if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
				{
					progbug( "Parse error, ignoring if while not in if or else", mob );
					--prog_nest;
					return;
				}
				ignorelevel++;
				break;

			case ORIGNORED:
#ifdef DEBUG
				log_string( "ORIGNORED" );
#endif
				if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
				{
					progbug( "Unmatched or", mob );
					--prog_nest;
					return;
				}
				if ( ignorelevel == 0 )
				{
					progbug( "Parse error, mistakenly ignoring or", mob );
					--prog_nest;
					return;
				}

				break;

			case BERR:
#ifdef DEBUG
				log_string( "BERR" );
#endif
				--prog_nest;
				return;
				break;
		}
	}
	--prog_nest;
	return;
}

/* This function replaces mprog_process_cmnd.  It is called from
 * mprog_driver, once for each line in a mud prog.  This function
 * checks what the line is, executes if/or checks and calls interpret
 * to perform the the commands.  Written by Narn, Dec 95.
 */
int mprog_do_command( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor,
                      OBJ_DATA *obj, void *vo, CHAR_DATA *rndm,
                      bool ignore, bool ignore_ors )
{
	char firstword[MAX_INPUT_LENGTH];
	char *ifcheck;
	char buf[ MAX_INPUT_LENGTH ];
	char tmp[ MAX_INPUT_LENGTH ];
	char *point, *str, *i;
	int validif, vnum;

	/* Isolate the first word of the line, it gives us a clue what
	   we want to do. */
	ifcheck = one_argument( cmnd, firstword );

	if ( !str_cmp( firstword, "if" ) )
	{
		/* Ok, we found an if.  According to the boolean 'ignore', either
		   ignore the ifcheck and report that back to mprog_driver or do
		   the ifcheck and report whether it was successful. */
		if ( ignore )
			return IFIGNORED;
		else
			validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

		if ( validif == 1 )
			return IFTRUE;

		if ( validif == 0 )
			return IFFALSE;

		return BERR;
	}

	if ( !str_cmp( firstword, "or" ) )
	{
		/* Same behavior as with ifs, but use the boolean 'ignore_ors' to
		   decide which way to go. */
		if ( ignore_ors )
			return ORIGNORED;
		else
			validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

		if ( validif == 1 )
			return ORTRUE;

		if ( validif == 0 )
			return ORFALSE;

		return BERR;
	}

	if ( !str_cmp( firstword, "and" ) )
	{
		/* Same behavior as with ifs, but use the boolean 'ignore_ors' to
		   decide which way to go. */
		if ( ignore_ors )
			return ORIGNORED;
		else
			validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

		if ( validif == 1 )
			return ANDTRUE;

		if ( validif == 0 )
			return ANDFALSE;

		return BERR;
	}

	/* For else and endif, just report back what we found.  Mprog_driver
	   keeps track of logiclevels. */
	if ( !str_cmp( firstword, "else" ) )
	{
		return FOUNDELSE;
	}

	if ( !str_cmp( firstword, "endif" ) )
	{
		return FOUNDENDIF;
	}

	/* Ok, didn't find an if, an or, an else or an endif.
	   If the command is in an if or else section that is not to be
	   performed, the boolean 'ignore' is set to true and we just
	   return.  If not, we try to execute the command. */

	if ( ignore )
		return COMMANDOK;

	if ( !str_prefix("mpwalk", firstword) ) {
		CHAR_DATA* chkchar = NULL;
		if ( !ifcheck || ifcheck[0] == '\0' ) {
			progbug("mpwalk -- no target specified", mob);
			return BERR;
		}
		if ( ifcheck && ifcheck[0] != '$' ) {
			do_mpwalk(mob, ifcheck);
			return COMMANDOK;
		}
		switch(ifcheck[1])
		{
			case 'i':	chkchar = mob;			break;
			case 'n':	chkchar = actor;		break;
			case 't':	chkchar = (CHAR_DATA *)vo;	break;
			case 'r':	chkchar = rndm;			break;
						//case 'f':   chkchar = (actor && actor->fighting)?actor->fighting->who:NULL; break;
			case 'f':   chkchar = actor->GetVictim(); break;
			default:
						{
							char rval[MAX_STRING_LENGTH];
							sprintf(rval, "Bad argument '%c' to 'mpwalk'", ifcheck[1]);
							progbug(rval, mob);
							return BERR;
						}
		}
		if ( !chkchar ) {
			progbug("mpwalk -- null target", mob);
			return BERR;
		}

		mob->DestinationCharId = chkchar->GetId();
		mob->vnum_destination = chkchar->GetInRoom()->vnum;

		return COMMANDOK;
	}

	if ( !str_prefix("mphunt", firstword) ) {
		CHAR_DATA* chkchar = NULL;
		if ( !ifcheck || ifcheck[0] == '\0' ) {
			progbug("mphunt -- no target specified", mob);
			return BERR;
		}
		if ( ifcheck && ifcheck[0] != '$' ) {
			do_mphunt(mob, ifcheck);
			return COMMANDOK;
		}
		switch(ifcheck[1])
		{
			case 'i':	chkchar = mob;			break;
			case 'n':	chkchar = actor;		break;
			case 't':	chkchar = (CHAR_DATA *)vo;	break;
			case 'r':	chkchar = rndm;			break;
						//case 'f':   chkchar = (actor && actor->fighting)?actor->fighting->who:NULL; break;
			case 'f':   chkchar = actor->GetVictim(); break;
			default:
						{
							char rval[MAX_STRING_LENGTH];
							sprintf(rval, "Bad argument '%c' to 'mphunt'", ifcheck[1]);
							progbug(rval, mob);
							return BERR;
						}
		}
		if ( !chkchar ) {
			progbug("mphunt -- null target", mob);
			return BERR;
		}

		start_hunting(mob, chkchar);

		return COMMANDOK;
	}

	/* If the command is 'break', that's all folks. */
	if ( !str_cmp( firstword, "break" ) )
		return BERR;

	vnum = mob->pIndexData->vnum;
	point   = buf;
	str     = cmnd;

	/* This chunk of code taken from mprog_process_cmnd. */
	while ( *str != '\0' )
	{
		if ( *str != '$' )
		{
			*point++ = *str++;
			continue;
		}
		str++;
		mprog_translate( *str, tmp, mob, actor, obj, vo, rndm );
		i = tmp;
		++str;
		while ( ( *point = *i ) != '\0' )
			++point, ++i;
	}
	*point = '\0';

	interpret( mob, buf );

	/* If the mob is mentally unstable and does things like fireball
	   itself, let's make sure it's still alive. */
	if ( char_died( mob ) )
	{
		bug( "Mob died while executing program, vnum %s.", vnum_to_dotted(vnum) );
		return BERR;
	}

	return COMMANDOK;
}

int mprog_command_check(char *arg, CHAR_DATA*mob, CHAR_DATA *actor,
        OBJ_DATA *obj, void *vo, bool bExact)
{
    MPROG_DATA *mprg;
    char command[MAX_INPUT_LENGTH];
    char cur_check[MAX_INPUT_LENGTH];
    char *check_list;
    int count = 0;

    one_argument(arg, command);

    for ( mprg = (MPROG_DATA*)vo; mprg; mprg = mprg->next )
    {
        check_list = mprg->arglist;

        while (check_list && check_list[0])
        {
            check_list = one_argument(check_list, cur_check);

            if ( mprg->type & COMMAND_PROG )
            {
                if ( cur_check[0] == '\0' )
                    continue;
                if ( !bExact && (strlen(command) == 0 || str_prefix(command, cur_check)))
                    continue;
                if ( bExact  && str_cmp(command, cur_check))
                    continue;

                rset_supermob(actor->GetInRoom());
                mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE);
                release_supermob();

                count++;
            }
        }
    }

    return count;
}

/***************************************************************************
 * Global function code and brief comments.
 */

bool mprog_keyword_check( const char *argu, const char *argl )
{
    char word[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    unsigned int i;
    char *arg, *arglist;
    char *start, *end;

    strcpy( arg1, strlower( argu ) );
    arg = arg1;
    strcpy( arg2, strlower( argl ) );
    arglist = arg2;

    for ( i = 0; i < strlen( arglist ); i++ )
	arglist[i] = LOWER( arglist[i] );
    for ( i = 0; i < strlen( arg ); i++ )
	arg[i] = LOWER( arg[i] );
    if ( ( arglist[0] == 'p' ) && ( arglist[1] == ' ' ) )
    {
	arglist += 2;
	while ( ( start = strstr( arg, arglist ) ) )
	    if ( (start == arg || *(start-1) == ' ' )
    	    && ( *(end = start + strlen( arglist ) ) == ' '
    	    ||   *end == '\n'
    	    ||   *end == '\r'
    	    ||   *end == '\0' ) )
		return TRUE;
	    else
		arg = start+1;
    }
    else
    {
	arglist = one_argument( arglist, word );
	for ( ; word[0] != '\0'; arglist = one_argument( arglist, word ) )
	    while ( ( start = strstr( arg, word ) ) )
		if ( ( start == arg || *(start-1) == ' ' )
	    	&& ( *(end = start + strlen( word ) ) == ' '
	    	||   *end == '\n'
	    	||   *end == '\r'
	    	||   *end == '\0' ) )
		    return TRUE;
		else
		    arg = start +1;
    }
    return FALSE;
}

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( const char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, void *vo, int64 type )
{

  char	      temp1[ MAX_STRING_LENGTH ];
  char	      temp2[ MAX_INPUT_LENGTH ];
  char	      word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  unsigned int	      i;

  for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
    if ( mprg->type & type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
          list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
		  mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
		    mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		    break;
		  }
		else
		  dupl = start+1;
	  }
      }

  return;

}

void mprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			 void *vo, int64 type)
{
 MPROG_DATA * mprg;

 for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
   if ( ( mprg->type & type )
       && ( number_percent( ) <= atoi( mprg->arglist ) ) )
     {
       mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
       if ( type != GREET_PROG && type != ALL_GREET_PROG )
	 break;
     }

 return;

}

void mprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
                         void *vo, int64 type)
{
	MPROG_DATA * mprg;
	bool       trigger_time;
	sh_int trigger_hour;

	for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	{
		/* Ksilyan
		 * Added support for sunrise/sunset.
		 */
		trigger_hour = atoi(mprg->arglist);
		switch (trigger_hour)
		{
			case -1:
				trigger_hour = sunrise_hour();
				break;
			case -2:
				trigger_hour = sunset_hour();
				break;
		}
		trigger_time = ( time_info.hour == trigger_hour );

		if ( !trigger_time )
		{
			if ( mprg->triggered )
				mprg->triggered = FALSE;
			continue;
		}

		if ( ( mprg->type & type )
				&& ( ( !mprg->triggered ) || ( mprg->type && HOUR_PROG ) ) )
		{
			mprg->triggered = TRUE;
			mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		}
	}
	return;
}


void mob_act_add( CHAR_DATA *mob )
{
    struct act_prog_data *runner;

    for ( runner = mob_act_list; runner; runner = runner->next )
	if ( runner->vo == mob )
	   return;
    CREATE(runner, struct act_prog_data, 1);
    runner->vo = mob;
    runner->next = mob_act_list;
    mob_act_list = runner;
}


/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger( char *buf, CHAR_DATA *mob, CHAR_DATA *ch,
		       OBJ_DATA *obj, void *vo)
{
    MPROG_ACT_LIST * tmp_act;
    MPROG_DATA *mprg;
    bool found = FALSE;

    if ( IS_NPC( mob )
    &&   IS_SET( mob->pIndexData->progtypes, ACT_PROG ) )
    {
	/* Don't let a mob trigger itself, nor one instance of a mob
	  trigger another instance. */
	if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
	  return;

	/* make sure this is a matching trigger */
	for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	    if ( mprg->type & ACT_PROG
	    &&   mprog_keyword_check( buf, mprg->arglist ) )
	    {
		found = TRUE;
		break;
	    }
	if ( !found )
	    return;

	CREATE( tmp_act, MPROG_ACT_LIST, 1 );
	if ( mob->mpactnum > 0 )
	  tmp_act->next = mob->mpact;
	else
	  tmp_act->next = NULL;

	mob->mpact      = tmp_act;
	mob->mpact->buf = str_dup( buf );
	mob->mpact->ch  = ch;
	mob->mpact->obj = obj;
	mob->mpact->vo  = vo;
	mob->mpactnum++;
	mob_act_add( mob );
    }
    return;
}

void mprog_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{

	char		buf[ MAX_STRING_LENGTH ];
	MPROG_DATA *mprg;
	OBJ_DATA   *obj;


	if ( IS_NPC( mob ) )
	{
		ArgumentMobile arg1 = ArgumentMobile(ch);
		ArgumentNumber arg2 = ArgumentNumber(amount);
		list<Argument*> list;
		list.push_back(&arg1);
		list.push_back(&arg2);
		mob->skryptSendEvent("bribe", list);
		list.clear();

		if ( can_see( mob, ch )
			&& ( mob->pIndexData->progtypes & BRIBE_PROG ) )
		{
			/* Don't let a mob trigger itself, nor one instance of a mob
			trigger another instance. */
			if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
				return;

			obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
			sprintf( buf, obj->shortDesc_.c_str(), amount );
			obj->shortDesc_ = buf;
			obj->value[0]	 = amount;
			obj = obj_to_char( obj, mob );
			mob->gold -= amount;

			for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
				if ( ( mprg->type & BRIBE_PROG )
					&& ( amount >= atoi( mprg->arglist ) ) )
				{
					mprog_driver( mprg->comlist, mob, ch, obj, NULL, FALSE );
					break;
				}
		}
	}

	return;

}

void mprog_otherdeath_trigger( CHAR_DATA *actor )
{
    CHAR_DATA *vmob;

    for ( vmob = actor->GetInRoom()->first_person; vmob; vmob = vmob->next_in_room )
    {
        if ( IS_NPC( vmob ) && actor != vmob )
		{
			ArgumentMobile arg1 = ArgumentMobile(actor); // person who died
			list<Argument*> list;
			list.push_back(&arg1);
			vmob->skryptSendEvent("other_death", list);
			list.clear();

			if ( vmob->pIndexData->progtypes & OTHERDEATH_PROG )
			{
				if ( IS_NPC( actor ) && actor->pIndexData == vmob->pIndexData )
					continue;
				mprog_percent_check( vmob, actor, NULL, NULL, OTHERDEATH_PROG );
			}
		}
    }
    return;
}

void mprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *mob )
{
	if ( IS_NPC( mob ) && killer != mob )
	{
		ArgumentMobile arg1 = ArgumentMobile(killer);
		list<Argument*> list;
		list.push_back(&arg1);
		mob->skryptSendEvent("selfDeath", list);
		list.clear();

		if ( ( mob->pIndexData->progtypes & DEATH_PROG ) )
		{
			mprog_percent_check( mob, killer, NULL, NULL, DEATH_PROG );
		}
	}

	// make sure that the victim is not an NPC with Inanimate set
	if ( !(IS_NPC(mob) && IS_SET(mob->act, ACT_INANIMATE) ) )
		death_cry( mob );
	return;
}

void mprog_entry_trigger( CHAR_DATA *mob )
{

	if ( IS_NPC( mob ) )
	{
		list<Argument*> list;
		mob->skryptSendEvent("self_enter", list);
		list.clear();

		if ( ( mob->pIndexData->progtypes & ENTRY_PROG ) )
			mprog_percent_check( mob, NULL, NULL, NULL, ENTRY_PROG );
	}
	return;

}

void mpwalk_finished_trigger(CHAR_DATA *mob, bool bSuccess)
{
    bMpwalkSuccess = bSuccess;
    if ( IS_NPC(mob) )
	{
		ArgumentBool arg1 = ArgumentBool(bSuccess);
		list<Argument*> list;
		list.push_back(&arg1);
		mob->skryptSendEvent("walk_finished", list);
		list.clear();

		if ( (mob->pIndexData->progtypes & MPWALK_FINISHED_PROG) )
			mprog_percent_check(mob, mob->GetDestinationChar(), NULL, NULL, MPWALK_FINISHED_PROG);
	}
    bMpwalkSuccess = FALSE;
}

// mob = aggressor, ch = victim
void mprog_fight_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
	CHAR_DATA * vmob;
	for (vmob = mob->GetInRoom()->first_person; vmob != NULL; vmob = vmob->next)
	{
		if ( vmob != mob )
		{
			if ( IS_NPC( vmob ) )
			{
				ArgumentMobile arg1 = ArgumentMobile(mob);
				ArgumentMobile arg2 = ArgumentMobile(ch);
				list<Argument*> list;
				list.push_back(&arg1);
				list.push_back(&arg2);
				vmob->skryptSendEvent("in_fight", list);
				list.clear();
			}
		}
	}
	if ( IS_NPC(mob) )
		if ( ( mob->pIndexData->progtypes & FIGHT_PROG ) )
			mprog_percent_check( mob, ch, NULL, NULL, FIGHT_PROG );

	return;

}

void mprog_get_trigger( CHAR_DATA *mob, OBJ_DATA *obj )
{

	char		buf[MAX_INPUT_LENGTH];
	MPROG_DATA *mprg;

	if ( IS_NPC( mob )
		&& ( mob->pIndexData->progtypes & GET_PROG ) )
	{
		for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
		{
			one_argument( mprg->arglist, buf );

			if ( ( mprg->type & GET_PROG )
				&& ( ( !str_cmp( obj->name_.c_str(), mprg->arglist ) )
				|| ( !str_cmp( "all", buf ) )
				|| ( is_number(buf) && atoi(buf) == obj->pIndexData->vnum) ) )
			{

				mprog_driver( mprg->comlist, mob, NULL, obj, NULL, FALSE );
				break;
			}
		}
	}
	return;
}

/* give trigger fixed up by Ksilyan */

void mprog_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{
	char        buf[MAX_INPUT_LENGTH];
	MPROG_DATA *mprg;

	if ( IS_NPC( mob )
	     && ( mob->pIndexData->progtypes & GIVE_PROG ) )
	{
		/* Don't let a mob trigger itself, nor one instance of a mob
		trigger another instance. */
		if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
			return;

		for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
		{
			int vnum;

			one_argument( mprg->arglist, buf );

			// Ksilyan removed ifcheck
			//if ( is_number(buf) )
			vnum = dotted_to_vnum(mob->GetInRoom()->vnum, buf);

			if ( ( mprg->type & GIVE_PROG )
			     && ( ( !str_cmp( obj->name_.c_str(), mprg->arglist ) )
			     || ( !str_cmp( "all", buf ) )
			     || ( vnum == obj->pIndexData->vnum) ) )
			{
				mprog_driver( mprg->comlist, mob, ch, obj, NULL, FALSE );
				break;
			}
		}
	}
	return;
}

void mprog_greet_dir_trigger( CHAR_DATA *ch, int dir)
{
    CHAR_DATA *vmob, *vmob_next;

    for ( vmob = ch->GetInRoom()->first_person; vmob; vmob = vmob_next )
    {
        vmob_next = vmob->next_in_room;

        if ( !IS_NPC(vmob)
                ||!can_see(vmob, ch)
                ||vmob->IsFighting()
                ||!IS_AWAKE(vmob))
            continue;

        if ( IS_NPC(ch) && ch->pIndexData == vmob->pIndexData )
            continue;

        if ( vmob->pIndexData->progtypes & GREET_DIR_PROG )
        {
            MPROG_DATA * mprg;

            for ( mprg = vmob->pIndexData->mudprogs; mprg; mprg = mprg->next )
            {
                if ( mprg->type == GREET_DIR_PROG &&
                        atoi(mprg->arglist) == dir ) {
                    mprog_driver(mprg->comlist, vmob, ch, NULL, NULL, FALSE);
                }
            }
        }
    }

}

void mprog_greet_trigger( CHAR_DATA *ch )
{
	CHAR_DATA *vmob, *vmob_next;

#ifdef DEBUG
	char buf[MAX_STRING_LENGTH];
	sprintf( buf, "mprog_greet_trigger -> %s", ch->getName().c_str() );
	log_string( buf );
#endif

	for ( vmob = ch->GetInRoom()->first_person; vmob; vmob = vmob_next )
	{
		vmob_next = vmob->next_in_room;
		if ( !IS_NPC( vmob )
			|| !can_see( vmob, ch )
			|| vmob->IsFighting()
			|| !IS_AWAKE( vmob ) )
			continue;

			/* Don't let a mob trigger itself, nor one instance of a mob
		trigger another instance. */
		if ( IS_NPC( ch ) && ch->pIndexData == vmob->pIndexData )
			continue;

		if ( vmob->pIndexData->progtypes & GREET_PROG )
			mprog_percent_check( vmob, ch, NULL, NULL, GREET_PROG );
		else if ( vmob->pIndexData->progtypes & ALL_GREET_PROG )
			mprog_percent_check(vmob,ch,NULL,NULL,ALL_GREET_PROG);
	}
	return;

}

void mprog_hitprcnt_trigger( CHAR_DATA *mob, CHAR_DATA *ch)
{

 MPROG_DATA *mprg;

 if ( IS_NPC( mob )
     && ( mob->pIndexData->progtypes & HITPRCNT_PROG ) )
   for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
     if ( ( mprg->type & HITPRCNT_PROG )
	 && ( ( 100*mob->hit / mob->max_hit ) < atoi( mprg->arglist ) ) )
       {
	 mprog_driver( mprg->comlist, mob, ch, NULL, NULL, FALSE );
	 break;
       }

 return;

}

void mprog_random_trigger( CHAR_DATA *mob )
{
  if ( mob->pIndexData->progtypes & RAND_PROG)
    mprog_percent_check(mob,NULL,NULL,NULL,RAND_PROG);

  return;
}

void mprog_time_trigger( CHAR_DATA *mob )
{
  if ( mob->pIndexData->progtypes & TIME_PROG)
       mprog_time_check(mob,NULL,NULL,NULL,TIME_PROG);
  return;
}

void mprog_hour_trigger( CHAR_DATA *mob )
{
  if ( mob->pIndexData->progtypes & HOUR_PROG)
       mprog_time_check(mob,NULL,NULL,NULL,HOUR_PROG);
  return;
}

void mprog_yell_trigger(const char* txt, CHAR_DATA *ch)
{
    AREA_DATA  *area;
    CHAR_DATA  *vmob;
    MPROG_DATA *prog;

    if ( !ch || !ch->GetInRoom() || !ch->GetInRoom()->area )
        return;

    area = ch->GetInRoom()->area;

    for ( vmob = first_char; vmob; vmob = vmob->next ) {
        if ( !IS_NPC(vmob) || !vmob->GetInRoom() || vmob->GetInRoom()->area != area )
            continue;

        if ( !(vmob->pIndexData->progtypes & YELL_PROG) )
            continue;

        for ( prog = vmob->pIndexData->mudprogs; prog; prog = prog->next ) {
            if ( !(prog->type & YELL_PROG) )
                continue;

            mprog_wordlist_check(txt, vmob, ch, NULL, NULL, YELL_PROG);
        }
    }
}

void mprog_speech_trigger( const char *txt, CHAR_DATA *actor )
{

	CHAR_DATA *vmob;

	for ( vmob = actor->GetInRoom()->first_person; vmob; vmob = vmob->next_in_room )
	{
		if ( IS_NPC( vmob ) && vmob != actor )
		{
			ArgumentMobile arg1 = ArgumentMobile(actor); // speaker
			ArgumentString arg2 = ArgumentString(txt);
			list<Argument*> list;
			list.push_back(&arg1);
			list.push_back(&arg2);
			vmob->skryptSendEvent("speak", list);
			list.clear();

			if ( ( vmob->pIndexData->progtypes & SPEECH_PROG ) )
			{
				if ( IS_NPC( actor ) && actor->pIndexData == vmob->pIndexData )
					continue;
				mprog_wordlist_check( txt, vmob, actor, NULL, NULL, SPEECH_PROG );
			}
		}
	}
	return;

}


/*
	KSILYAN
	Added this for when you look at a mob/object and it reacts.
*/

void mprog_look_trigger( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( IS_NPC( ch ) && ch->pIndexData == victim->pIndexData )
		return;

	if ( !IS_NPC(victim) )
		return;

	mprog_percent_check( victim, ch, NULL, NULL, LOOK_PROG );
}

void oprog_look_trigger( CHAR_DATA *ch, OBJ_DATA *vobj )
{
	set_supermob( vobj );  /* not very efficient to do here */
	oprog_percent_check( supermob, ch, vobj, NULL, LOOK_PROG );
	release_supermob();

	return;
}

/*
    KSILYAN
    Added this for when someone is stolen from...
    for player thief flags mostly
*/

void mprog_steal_trigger( CHAR_DATA *ch )
{
    CHAR_DATA *vmob, *vmob_next;

    for ( vmob = ch->GetInRoom()->first_person; vmob; vmob = vmob_next )
    {
        vmob_next = vmob->next_in_room;
        if ( !IS_NPC( vmob )
            //|| !can_see( vmob, ch )
            || vmob->IsFighting()
            || !IS_AWAKE( vmob ) )
                continue;

        /* Don't let a mob trigger itself, nor one instance of a mob
            trigger another instance. */
        if ( IS_NPC( ch ) && ch->pIndexData == vmob->pIndexData )
            continue;

        mprog_percent_check( vmob, ch, NULL, NULL, STEAL_PROG );
    }
    return;
}

void mprog_script_trigger( CHAR_DATA *mob )
{
    MPROG_DATA * mprg;

    if ( mob->pIndexData->progtypes & SCRIPT_PROG)
      for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	if ( ( mprg->type & SCRIPT_PROG ) )
	{
	  if ( mprg->arglist[0] == '\0'
	  ||   mob->mpscriptpos != 0
	  ||   atoi( mprg->arglist ) == time_info.hour )
	    mprog_driver( mprg->comlist, mob, NULL, NULL, NULL, TRUE );
	}
    return;
}

void oprog_script_trigger( OBJ_DATA *obj )
{
    MPROG_DATA * mprg;

    if ( obj->pIndexData->progtypes & SCRIPT_PROG)
      for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
	if ( ( mprg->type & SCRIPT_PROG ) )
	{
	  if ( mprg->arglist[0] == '\0'
	  ||   obj->mpscriptpos != 0
	  ||   atoi( mprg->arglist ) == time_info.hour )
	  {
	     set_supermob( obj );
	     mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, TRUE );
	     obj->mpscriptpos = supermob->mpscriptpos;
	     release_supermob();
	  }
	}
    return;
}

void rprog_script_trigger( ROOM_INDEX_DATA *room )
{
    MPROG_DATA * mprg;

    if ( room->progtypes & SCRIPT_PROG)
      for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
	if ( ( mprg->type & SCRIPT_PROG ) )
	{
	  if ( mprg->arglist[0] == '\0'
	  ||   room->mpscriptpos != 0
	  ||   atoi( mprg->arglist ) == time_info.hour )
	  {
	     rset_supermob( room );
	     mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, TRUE );
	     room->mpscriptpos = supermob->mpscriptpos;
	     release_supermob();
	  }
	}
    return;
}


/*
 *  Mudprogram additions begin here
 */
void set_supermob( OBJ_DATA *obj)
{
  ROOM_INDEX_DATA *room;
  OBJ_DATA *in_obj;
  CHAR_DATA *mob;
  char buf[200];

  if ( !supermob )
    supermob = create_mobile(get_mob_index( 3 ));

  mob = supermob;   /* debugging */

  if(!obj)
     return;

  for ( in_obj = obj; in_obj->GetInObj(); in_obj = in_obj->GetInObj() )
    ;

  if ( in_obj->GetCarriedBy() )
  {
      room = in_obj->GetCarriedBy()->GetInRoom();
  }
  else
  {
      room = obj->GetInRoom();
  }

  if(!room)
     return;

  supermob->setShort( obj->shortDesc_ );
  supermob->mpscriptpos = obj->mpscriptpos;

  /* Added by Jenny to allow bug messages to show the vnum
     of the object, and not just supermob's vnum */
  sprintf( buf, "Object #%s", vnum_to_dotted(obj->pIndexData->vnum) );
  supermob->description_ = STRALLOC( buf );

	/* Added by Ksilyan so that the supermob knows which obj it's representing */
  supermob->spare_ptr = obj;
  supermob->tempnum = TEMPNUM_OBJ;

  if(room != NULL)
  {
    char_from_room (supermob );
    char_to_room( supermob, room);
  }
}

void release_supermob( )
{
  supermob->tempnum = 0;
  supermob->spare_ptr = NULL;
  char_from_room( supermob );
  char_to_room( supermob, get_room_index( 3 ) );
}


bool oprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			 void *vo, int64 type)
{
 MPROG_DATA * mprg;
 bool executed = FALSE;

 for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
   if ( ( mprg->type & type )
       && ( number_percent( ) <= atoi( mprg->arglist ) ) )
     {
       executed = TRUE;
       mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
       if ( type != GREET_PROG )
	 break;
     }

 return executed;

}

/*
 * Triggers follow
 */


/*
 *  Hold on this
 *
void oprog_act_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   set_supermob( obj );
   if ( obj->pIndexData->progtypes & ACT_PROG )
     oprog_percent_check( supermob, ch, obj, NULL, ACT_PROG );

 release_supermob();
 return;
}
 *
 *
 */

void oprog_greet_dir_trigger( CHAR_DATA *ch, int dir )
{
  OBJ_DATA *vobj;

  for ( vobj=ch->GetInRoom()->first_content; vobj; vobj = vobj->next_content )
    if  ( vobj->pIndexData->progtypes & GREET_DIR_PROG )
    {
        MPROG_DATA *mprg;

        for ( mprg = vobj->pIndexData->mudprogs; mprg; mprg=mprg->next ) {
            if ( mprg->type == GREET_DIR_PROG &&
                    atoi(mprg->arglist) == dir ) {
                set_supermob( vobj );  /* not very efficient to do here */
                mprog_driver(mprg->comlist, supermob, ch, vobj, NULL, FALSE);
                release_supermob();
            }
        }
    }

  return;
}

void oprog_greet_trigger( CHAR_DATA *ch )
{
  OBJ_DATA *vobj;

  for ( vobj=ch->GetInRoom()->first_content; vobj; vobj = vobj->next_content )
    if  ( vobj->pIndexData->progtypes & GREET_PROG )
    {
     set_supermob( vobj );  /* not very efficient to do here */
     oprog_percent_check( supermob, ch, vobj, NULL, GREET_PROG );
     release_supermob();
    }

  return;
}

void oprog_speech_trigger( const char *txt, CHAR_DATA *ch )
{
OBJ_DATA *vobj;

  /* supermob is set and released in oprog_wordlist_check */
  for ( vobj=ch->GetInRoom()->first_content; vobj; vobj = vobj->next_content )
    if  ( vobj->pIndexData->progtypes & SPEECH_PROG )
    {
      oprog_wordlist_check( txt, supermob, ch, vobj, NULL, SPEECH_PROG, vobj );
    }

 return;
}

/*
 * Called at top of obj_update
 * make sure to put an if(!obj) continue
 * after it
 */
void oprog_random_trigger( OBJ_DATA *obj )
{

  if ( obj->pIndexData->progtypes & RAND_PROG)
  {
     set_supermob( obj );
     oprog_percent_check(supermob,NULL,obj,NULL,RAND_PROG);
     release_supermob();
  }
  return;
}

/*
 * in wear_obj, between each successful equip_char
 * the subsequent return
 */
void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & WEAR_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, WEAR_PROG );
      release_supermob();
   }
   return;
}

bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict,
                        OBJ_DATA *targ, void *vo )
{
   bool executed = FALSE;

   if ( obj->pIndexData->progtypes & USE_PROG )
   {
      set_supermob( obj );
      if ( obj->item_type == ITEM_STAFF )
      {
        if ( vict )
          executed = oprog_percent_check( supermob, ch, obj, vict, USE_PROG );
        else
          executed = oprog_percent_check( supermob, ch, obj, targ, USE_PROG );
      }
      else
      {
        executed = oprog_percent_check( supermob, ch, obj, NULL, USE_PROG );
      }
      release_supermob();
   }
   return executed;
}

/*
 * call in remove_obj, right after unequip_char
 * do a if(!ch) return right after, and return TRUE (?)
 * if !ch
 */
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & REMOVE_PROG )
   {
     set_supermob( obj );
     oprog_percent_check( supermob, ch, obj, NULL, REMOVE_PROG );
     release_supermob();
   }
   return;
}


/*
 * call in do_sac, right before extract_obj
 */
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & SAC_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, SAC_PROG );
      release_supermob();
   }
   return;
}


/*
 * call in do_get, right before check_for_trap
 * do a if(!ch) return right after
 */
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & GET_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, GET_PROG );
      release_supermob();
   }
   return;
}

/*
 * called in damage_obj in act_obj.c
 */
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & DAMAGE_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, DAMAGE_PROG );
      release_supermob();
   }
   return;
}

/*
 * called in do_repair in shops.c
 */
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{

   if ( obj->pIndexData->progtypes & REPAIR_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, REPAIR_PROG );
      release_supermob();
   }
   return;
}

/*
 * call twice in do_drop, right after the act( AT_ACTION,...)
 * do a if(!ch) return right after
 */
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & DROP_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, DROP_PROG );
      release_supermob();
   }
   return;
}

/*
 * call towards end of do_examine, right before check_for_trap
 */
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & EXA_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, EXA_PROG );
      release_supermob();
   }
   return;
}


/*
 * call in fight.c, group_gain, after (?) the obj_to_room
 */
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & ZAP_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, ZAP_PROG );
      release_supermob();
   }
   return;
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there
 */
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & PULL_PROG )
   {
     set_supermob( obj );
     oprog_percent_check( supermob, ch, obj, NULL, PULL_PROG );
     release_supermob();
   }
   return;
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there
 */
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
   if ( obj->pIndexData->progtypes & PUSH_PROG )
   {
      set_supermob( obj );
      oprog_percent_check( supermob, ch, obj, NULL, PUSH_PROG );
      release_supermob();
   }
   return;
}

void obj_act_add( OBJ_DATA *obj );
void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo )
{
   if ( mobj->pIndexData->progtypes & ACT_PROG )
   {
      MPROG_ACT_LIST *tmp_act;

      CREATE(tmp_act, MPROG_ACT_LIST, 1);
      if ( mobj->mpactnum > 0 )
        tmp_act->next = mobj->mpact;
      else
        tmp_act->next = NULL;

      mobj->mpact = tmp_act;
      mobj->mpact->buf = str_dup(buf);
      mobj->mpact->ch = ch;
      mobj->mpact->obj = obj;
      mobj->mpact->vo = vo;
      mobj->mpactnum++;
      obj_act_add(mobj);
   }
   return;
}

void oprog_wordlist_check( const char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, void *vo, int64 type, OBJ_DATA *iobj )
{

  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  unsigned int         i;

  for ( mprg = iobj->pIndexData->mudprogs; mprg; mprg = mprg->next )
    if ( mprg->type & type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
		  set_supermob( iobj );
		  mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		  release_supermob() ;
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
		    set_supermob( iobj );
		    mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		    release_supermob();
		    break;
		  }
		else
		  dupl = start+1;
	  }
      }

  return;
}



/*
 *  room_prog support starts here
 *
 *
 */

void rset_supermob( ROOM_INDEX_DATA *room)
{
  char buf[200];

  if (room)
  {
	  if ( !supermob )
		supermob = create_mobile(get_mob_index( 3 ));

    supermob->setShort( room->name_ );
    supermob->setName( room->name_ );
    supermob->mpscriptpos = room->mpscriptpos;

    /* Added by Jenny to allow bug messages to show the vnum
       of the room, and not just supermob's vnum */
    sprintf( buf, "Room #%s", vnum_to_dotted(room->vnum) );
    supermob->description_ = buf;

    char_from_room (supermob );
    char_to_room( supermob, room);
  }
}


void rprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			 void *vo, int64 type)
{
 MPROG_DATA * mprg;

 if(!mob->GetInRoom())
   return;

 for ( mprg = mob->GetInRoom()->mudprogs; mprg; mprg = mprg->next )
   if ( ( mprg->type & type )
       && ( number_percent( ) <= atoi( mprg->arglist ) ) )
     {
       mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
       if(type!=ENTER_PROG)
          break;
     }

     return;
}

/*
 * Triggers follow
 */


void rprog_greet_dir_trigger(CHAR_DATA *ch, int dir ){
    MPROG_DATA *mprg;

    if ( !(ch->GetInRoom()->progtypes & GREET_DIR_PROG) ) {
        return;
    }

    for ( mprg = ch->GetInRoom()->mudprogs; mprg; mprg = mprg->next )
    {
        if ( mprg->type == GREET_DIR_PROG && atoi(mprg->arglist) == dir ) {
            rset_supermob(ch->GetInRoom());
            mprog_driver(mprg->comlist, supermob, ch, NULL, NULL, FALSE);
            release_supermob();
        }
    }
}
/*
 *  Hold on this
 * Unhold. -- Alty
 */
void room_act_add( ROOM_INDEX_DATA *room );
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo )
{
	if ( room->progtypes & ACT_PROG )
	{
		MPROG_ACT_LIST *tmp_act;

		CREATE(tmp_act, MPROG_ACT_LIST, 1);
		if ( room->mpactnum > 0 )
			tmp_act->next = room->mpact;
		else
			tmp_act->next = NULL;

		room->mpact = tmp_act;
		room->mpact->buf = str_dup(buf);
		room->mpact->ch = ch;
		room->mpact->obj = obj;
		room->mpact->vo = vo;
		room->mpactnum++;
		room_act_add(room);
	}
	return;
}
/*
 *
 */


void rprog_leave_trigger( CHAR_DATA *ch )
{
  if( ch->GetInRoom()->progtypes & LEAVE_PROG )
  {
    rset_supermob( ch->GetInRoom() );
    rprog_percent_check( supermob, ch, NULL, NULL, LEAVE_PROG );
    release_supermob();
  }
  return;
}

void rprog_enter_trigger( CHAR_DATA *ch )
{
	if ( !ch->GetInRoom() )
	{
		gTheWorld->LogBugString( string("ERROR! rprog_enter_trigger ch '") + ch->getShort().str() + " has null inroom!" );
		return;
	}
	ArgumentMobile arg1(ch);
	list<Argument*> list;
	list.push_back(&arg1);
	ch->GetInRoom()->skryptSendEvent("enter", list);
	list.clear();

	if( ch->GetInRoom()->progtypes & ENTER_PROG )
	{
		rset_supermob( ch->GetInRoom() );
		rprog_percent_check( supermob, ch, NULL, NULL, ENTER_PROG );
		release_supermob();
	}
	return;
}

void rprog_sleep_trigger( CHAR_DATA *ch )
{
  if( ch->GetInRoom()->progtypes & SLEEP_PROG )
  {
    rset_supermob( ch->GetInRoom() );
    rprog_percent_check( supermob, ch, NULL, NULL, SLEEP_PROG );
    release_supermob();
  }
  return;
}

void rprog_rest_trigger( CHAR_DATA *ch )
{
  if( ch->GetInRoom()->progtypes & REST_PROG )
  {
    rset_supermob( ch->GetInRoom() );
    rprog_percent_check( supermob, ch, NULL, NULL, REST_PROG );
    release_supermob();
  }
  return;
}

void rprog_rfight_trigger( CHAR_DATA *ch )
{
  if( ch->GetInRoom()->progtypes & RFIGHT_PROG )
  {
    rset_supermob( ch->GetInRoom() );
    rprog_percent_check( supermob, ch, NULL, NULL, RFIGHT_PROG );
    release_supermob();
  }
  return;
}

void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch )
{
  if( ch->GetInRoom()->progtypes & RDEATH_PROG )
  {
    rset_supermob( ch->GetInRoom() );
    rprog_percent_check( supermob, ch, NULL, NULL, RDEATH_PROG );
    release_supermob();
  }
  return;
}

/* KSILYAN
	Small rptrigger to allow for more interesting successful search messages.
	Return true if we have a search program; return false if not.
*/

bool rprog_search_trigger( CHAR_DATA * ch )
{
	if ( ch->GetInRoom()->progtypes & SEARCH_PROG )
	{
		rset_supermob( ch->GetInRoom() );
		rprog_percent_check( supermob, ch, NULL, NULL, SEARCH_PROG );
		release_supermob();
		return TRUE;
	}
	else
		return FALSE;
}

signed int prog_command_trigger( CHAR_DATA *ch, char *txt, bool bExact )
{
    CHAR_DATA* vch;
    int count = 0;

    fcontinue_after_command = 0;

	if ( ch->GetInRoom() == NULL )
	{
		gTheWorld->LogBugString( string("ERROR! prog_command_trigger ch '")
				+ ch->getShort().str() + " has null inroom!" );
		return -1;
	}

    if ( ch->GetInRoom()->progtypes & COMMAND_PROG )
    {
/*        rset_supermob(ch->GetInRoom());*/
        count += mprog_command_check(txt, supermob, ch, NULL, ch->GetInRoom()->mudprogs, bExact);
/*        release_supermob();*/
    }

	if ( ch->GetInRoom() == NULL )
	{
		gTheWorld->LogBugString( string("ERROR! prog_command_trigger ch '")
				+ ch->getName().str() + " has null inroom after command prog!" );
		return -1;
	}

    for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next_in_room )
    {
        if ( !IS_NPC(vch) )
            continue;

        if ( ch->pIndexData == vch->pIndexData )
            continue;

        if ( !(vch->pIndexData->progtypes & COMMAND_PROG) )
            continue;

        count += mprog_command_check(txt, vch, ch, NULL, vch->pIndexData->mudprogs, bExact);
    }

    if ( !count ) {
        return -1;
    }

    /* check later to see if the program said to try the original type */
    return fcontinue_after_command;
}

void rprog_speech_trigger( const char *txt, CHAR_DATA *ch )
{
  if( ch->GetInRoom()->progtypes & SPEECH_PROG )
  {
    /* supermob is set and released in rprog_wordlist_check */
    rprog_wordlist_check( txt, supermob, ch, NULL, NULL, SPEECH_PROG, ch->GetInRoom() );
  }
 return;
}

void rprog_random_trigger( CHAR_DATA *ch )
{
  if ( ch->GetInRoom()->progtypes & RAND_PROG)
  {
    rset_supermob( ch->GetInRoom() );
    rprog_percent_check(supermob,ch,NULL,NULL,RAND_PROG);
    release_supermob();
  }
  return;
}

void rprog_wordlist_check( const char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, void *vo, int64 type, ROOM_INDEX_DATA *room )
{

  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  unsigned int         i;

  if ( actor && !char_died(actor) && actor->GetInRoom() )
    room = actor->GetInRoom();

  for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
    if ( mprg->type & type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
		  rset_supermob( room );
		  mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		  release_supermob() ;
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
		    rset_supermob( room );
		    mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		    release_supermob();
		    break;
		  }
		else
		  dupl = start+1;
	  }
      }
      return;
}

void rprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			void *vo, int64 type )
{
	ROOM_INDEX_DATA * room = (ROOM_INDEX_DATA *) vo;
	MPROG_DATA * mprg;
	sh_int		trigger_hour;
	bool 	       trigger_time;

	for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
	{
		/* Ksilyan
		 * Added support for sunrise / sunset
		 */
		trigger_hour = atoi(mprg->arglist);
		switch (trigger_hour)
		{
			case -1:
				trigger_hour = sunrise_hour();
				break;
			case -2:
				trigger_hour = sunset_hour();
				break;
		}
		trigger_time = ( time_info.hour == trigger_hour );

		if ( !trigger_time )
		{
			if ( mprg->triggered )
				mprg->triggered = FALSE;
			continue;
		}

		if ( ( mprg->type & type )
				&& ( ( !mprg->triggered ) || ( mprg->type & HOUR_PROG ) ) )
		{
			mprg->triggered = TRUE;
			mprog_driver( mprg->comlist, mob, actor, obj, vo, FALSE );
		}
	}
	return;
}

void rprog_time_trigger( CHAR_DATA *ch )
{
  if ( ch->GetInRoom()->progtypes & TIME_PROG )
  {
    rset_supermob( ch->GetInRoom() );
    rprog_time_check( supermob, NULL, NULL, ch->GetInRoom(), TIME_PROG );
    release_supermob();
  }
  return;
}

void rprog_hour_trigger( CHAR_DATA *ch )
{
  if ( ch->GetInRoom()->progtypes & HOUR_PROG )
  {
    rset_supermob( ch->GetInRoom() );
    rprog_time_check( supermob, NULL, NULL, ch->GetInRoom(), HOUR_PROG );
    release_supermob();
  }
  return;
}

/* Written by Jenny, Nov 29/95 */
void progbug( const char *str, CHAR_DATA *mob )
{
  char buf[MAX_STRING_LENGTH];

  /* Check if we're dealing with supermob, which means the bug occurred
     in a room or obj prog. */
  if ( mob && mob->pIndexData && mob->pIndexData->vnum == 3 )
  {
    /* It's supermob.  In set_supermob and rset_supermob, the description
       was set to indicate the object or room, so we just need to show
       the description in the bug message. */
    sprintf( buf, "%s, %s.", str,
             mob->description_.length() > 0 ? "(unknown)" : mob->description_.c_str() );
  }
  else
  {
	  if ( IS_NPC(mob) )
		  sprintf( buf, "%s, Mob #%s.", str, vnum_to_dotted(mob->pIndexData->vnum) );
	  else
		  sprintf( buf, "%s, Player %s.", str, mob->getName().c_str() );
  }

  lbug( LEVEL_ARTIFICER, buf, 0 );
  return;
}


/* Room act prog updates.  Use a separate list cuz we dont really wanna go
   thru 5-10000 rooms every pulse.. can we say lag? -- Alty */

void room_act_add( ROOM_INDEX_DATA *room )
{
	struct act_prog_data *runner;

	for ( runner = room_act_list; runner; runner = runner->next )
		if ( runner->vo == room )
			return;
	CREATE(runner, struct act_prog_data, 1);
	runner->vo = room;
	runner->next = room_act_list;
	room_act_list = runner;

}


void room_act_update( void )
{
	struct act_prog_data *runner;
	MPROG_ACT_LIST *mpact;

	while ( (runner = room_act_list) != NULL )
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) runner->vo;

		while ( (mpact = room->mpact) != NULL )
		{
			if ( mpact->ch->GetInRoom() == room )
			{
				rprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
						mpact->vo, ACT_PROG, room);
			}
			room->mpact = mpact->next;
			DISPOSE(mpact->buf);
			DISPOSE(mpact);
		}
		room->mpact = NULL;
		room->mpactnum = 0;
		room_act_list = runner->next;
		DISPOSE(runner);
	}
	return;
}

void obj_act_add( OBJ_DATA *obj )
{
  struct act_prog_data *runner;

  for ( runner = obj_act_list; runner; runner = runner->next )
    if ( runner->vo == obj )
      return;
  CREATE(runner, struct act_prog_data, 1);
  runner->vo = obj;
  runner->next = obj_act_list;
  obj_act_list = runner;
}
void obj_act_update( void )
{
	struct act_prog_data *runner;
	MPROG_ACT_LIST *mpact;

	while ( (runner = obj_act_list) != NULL )
	{
		OBJ_DATA *obj = (OBJ_DATA *) runner->vo;

		while ( (mpact = obj->mpact) != NULL )
		{
			oprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
					mpact->vo, ACT_PROG, obj);
			obj->mpact = mpact->next;
			DISPOSE(mpact->buf);
			DISPOSE(mpact);
		}
		obj->mpact = NULL;
		obj->mpactnum = 0;
		obj_act_list = runner->next;
		DISPOSE(runner);
	}
	return;
}

void do_mpcontinue(CHAR_DATA *ch, const char* argument)
{
    fcontinue_after_command++;
}

void do_mpdispel(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA* vch;

    if ( !IS_NPC(ch) ) {
        send_to_char("Huh?\r\n", ch);
        return;
    }

    if ( !argument || argument[0] == '\0' || !(vch = get_char_world(ch, argument)) ) {
        progbug("do_mpdispel: victim not found", ch);
        return;
    }

    while(vch->first_affect) {
        affect_remove(vch, vch->first_affect);
    }
    fix_affected_by( vch );
}

/* Syntax: mpflag $n QuestName - Zoie */
void do_mpflag( CHAR_DATA *ch, const char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	progbug( "Mpflag - No argument", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	progbug( "Mpflag - victim does not exist", ch );
	return;
    }

	if( IS_NPC(victim) )
	{
		send_to_char("Mobs can't have quests!\n\r",ch);
		return;
	}

    if( argument[0] == '\0' )
	{
		progbug( "Mpflag - no quest to complete",ch);
		return;
	}

	AddFlag(argument, victim);

/* WTF -Zoie
   if(argument[strlen(argument)-1] == '\0')
   {
      argument[strlen(argument)-1] = '\0';
   }
*/

}

