/* matthew -- stable stuff */

#include <stdio.h>
#include "mud.h"

#include "commands.h"

// Forward declaration
void do_say(Character * ch, const char* argument);

/* mostly copied from find_keeper */
CHAR_DATA* find_groom(CHAR_DATA *ch) {
	CHAR_DATA *keeper;
	STABLE_DATA *pStable;

	pStable = NULL;

	for ( keeper = ch->GetInRoom()->first_person;
		  keeper;
		  keeper = keeper->next_in_room) {

		if ( IS_NPC(keeper) && (pStable = keeper->pIndexData->pStable) != NULL){			break;
		}
	}

	if ( !pStable ) {
		send_to_char("You aren't in a stable.\r\n", ch);
		return NULL;
	}

	if ( time_info.hour < pStable->open_hour ) {
		do_say(keeper, "Sorry, come back later.");
		return NULL;
	}

	if ( time_info.hour > pStable->close_hour ) {
		do_say(keeper, "Sorry, come back tomorrow.");
		return NULL;
	}

	/*
	 * Invisible or hidden people.
	 */
	if ( !can_see( keeper, ch ) )
	{
		do_say( keeper, "I don't trade with folks I can't see." );
		return NULL;
	}
	
	if ( !knows_language( keeper, ch->speaking, ch ) )
	{
		do_say( keeper, "I can't understand you." );
		return NULL;
	}

	return keeper;
}

void do_stable(CHAR_DATA* ch, const char* argument) {
	char arg[MAX_INPUT_LENGTH]  = "\0";
	char buf[MAX_STRING_LENGTH] = "\0";
	ExtraDescData *ed        = NULL;
	CHAR_DATA   *keeper;
	CHAR_DATA   *mobile;
	OBJ_DATA    *note;
	
	argument = one_argument(argument, arg);
	
	if ( ( keeper = find_groom(ch) ) == NULL ) {
		return;
	}

	if ( arg[0] == '\0' ) {
		send_to_char("Which animal do you wish to stable?\r\n", ch);
		return;
	}
	
	if ( ( mobile = get_char_room(ch, arg) ) == NULL ) {
		send_to_char("I couldn't find that animal anywhere!\r\n", ch);
		return;
	}

	if ( !IS_NPC(mobile) ) {
		send_to_char("What?  He isn't an animal!\r\n", ch);
		return;
	}

    if ( !IS_IMMORTAL(ch) ) {
    	if ( mobile->MasterId != ch->GetId() ) {
	    	send_to_char("You can't control that animal.\r\n", ch);
    		return;
    	}

    	if ( ch->MountId == mobile->GetId() ) {
    		dofun_wrapper( do_say, keeper,
					string("But you are sitting on that animal, " + ch->getName().str() + "!").c_str() );
    		return;
	    }

    	if ( IS_SET(mobile->act, ACT_MOUNTED) ) {
    		do_say(keeper, "But that animal has someone mounted upon it!" );
    		return;
    	}

    	if ( !IS_SET(mobile->act, ACT_MOUNTABLE) ) {
    		act(AT_SAY, "$n screams 'You can't bring that in here!  Go find some other hovel!'", keeper, NULL, ch, TO_ROOM);
	    	return;
    	}
	
	    if ( ch->gold < keeper->pIndexData->pStable->stable_cost ) {
    		do_say(keeper, "I'm sorry, but you don't have enough money.");
    		return;
    	}
	
	    ch->gold -= keeper->pIndexData->pStable->stable_cost;
    }

	note = create_object(get_obj_index(OBJ_VNUM_NOTE), 0);
	
	CREATE(note->pObjNPC, OBJ_NPC_DATA, 1);
	note->pObjNPC->m_vnum = mobile->pIndexData->vnum;
	note->pObjNPC->r_vnum = keeper->GetInRoom()->vnum;

	sprintf(buf, "stable note free animal %s",
					mobile->getShort().c_str());
	note->name_ = buf;
	
	sprintf(buf, "A stable note from %s is lying here.",
					keeper->GetInRoom()->name_.c_str());
	note->longDesc_ = buf;

	sprintf(buf, "a stable note");
	note->shortDesc_ = buf;

	sprintf(buf,
		"Scribbled upon the note are the words:\r\n\r\n"
		"Good for one '%s' at '%s'.\r\n",
		mobile->getShort().c_str(),
		keeper->GetInRoom()->name_.c_str());
	ed = SetOExtra(note, note->name_.c_str());
	ed->description_ = buf;
	
	extract_char(mobile, TRUE);

	note->value[1] = 100; /* This makes it not usable for a bboard */
	
	obj_to_char(note, ch);
	
	act(AT_ACTION, "You give $N some gold, and $E hands you a piece of paper.",
					ch, NULL, keeper, TO_CHAR);

	act(AT_ACTION, "$n takes $N back to the stables.",
					keeper, NULL, mobile, TO_ROOM);
}

void do_makestable(CHAR_DATA *ch, const char* argument) {
	STABLE_DATA *stable;
	int vnum;
	MOB_INDEX_DATA *mob;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makestable <mobvnum>\n\r", ch );
		return;
	}

	vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);
	
	if ( (mob = get_mob_index(vnum)) == NULL )
	{
		send_to_char( "Mobile not found.\n\r", ch );
		return;
	}

	if ( !can_medit(ch, mob) )
		return;

	if ( mob->pStable )
	{
		send_to_char( "This mobile already has a stable.\n\r", ch );
		return;
	}

	CREATE( stable, STABLE_DATA, 1 );

	LINK( stable, first_stable, last_stable, next, prev );
	stable->keeper        = vnum;
	stable->unstable_cost = 10;
	stable->stable_cost   = 10;
	stable->open_hour     = 0;
	stable->close_hour    = 23;
	mob->pStable          = stable;
	send_to_char( "Done.\n\r", ch );
	return;
}

void do_stableset(CHAR_DATA *ch, const char* argument) {
	STABLE_DATA *stable;
	MOB_INDEX_DATA *mob, *mob2;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int vnum;
	int value;
	
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Usage: stableset <mob vnum> <field> value\n\r", ch );
		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( "  stable unstable open close\n\r", ch );
		return;
	}

	vnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg1);

	if ( (mob = get_mob_index(vnum)) == NULL )
	{
		send_to_char( "Mobile not found.\n\r", ch );
		return;
	}

	if ( !can_medit(ch, mob) )
	{
		return;
	}

	if ( !mob->pStable )
	{
		send_to_char( "This mobile doesn't keep a stable.\n\r", ch );
		return;
	}

	stable = mob->pStable;
	value = atoi( argument );

	if ( !str_cmp( arg2, "stable" ) )
	{
		if ( value > 5000 )
		{
		    send_to_char( "Out of range.\n\r", ch );
		    return;
		}
	
		stable->stable_cost = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "unstable" ) )
	{
		if ( value < 0 )
		{
		    send_to_char( "Out of range.\n\r", ch );
		    return;
		}

		stable->unstable_cost = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "open" ) )
	{
		if ( value < 0 || value > 23 )
		{
		    send_to_char( "Out of range.\n\r", ch );
		    return;
		}
	
		stable->open_hour = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "close" ) )
	{
		if ( value < 0 || value > 23 )
		{
		    send_to_char( "Out of range.\n\r", ch );
		    return;
		}
		stable->close_hour = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "keeper" ) )
	{
		if ( (mob2 = get_mob_index(vnum)) == NULL )
		{
		    send_to_char( "Mobile not found.\n\r", ch );
		    return;
		}
		if ( !can_medit(ch, mob) )
		    return;
		if ( mob2->pStable )
		{
		    send_to_char( "That mobile already has a stable.\n\r", ch );
		    return;
		}

		mob->pStable  = NULL;
		mob2->pStable = stable;
		stable->keeper = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	do_stableset( ch, "" );
	return;
}

void do_stablestat(CHAR_DATA *ch, const char* argument)
{
	STABLE_DATA *stable;
	MOB_INDEX_DATA *mob;
	int vnum;
	
	if ( argument[0] == '\0' )
	{
		send_to_char( "Usage: stablestat <keeper vnum>\n\r", ch );
		return;
	}

	vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);
	
	if ( (mob = get_mob_index(vnum)) == NULL )
	{
		send_to_char( "Mobile not found.\n\r", ch );
		return;
	}

	if ( !mob->pStable )
	{
		send_to_char( "This mobile doesn't keep a stable.\n\r", ch );
		return;
	}

	stable = mob->pStable;

	ch_printf( ch, "Keeper: %d  %s\n\r", stable->keeper, mob->shortDesc_.c_str() );
	ch_printf( ch, "Cost to:  Stable: %3d  Unstable: %3d\n\r",
			stable->stable_cost,
			stable->unstable_cost );
	ch_printf( ch, "Hours:   open %2d  close %2d\n\r",
			stable->open_hour,
			stable->close_hour );
	return;
}


void do_stables(CHAR_DATA *ch, const char* argument)
{
	STABLE_DATA *stable;

	if ( !first_stable )
	{
		send_to_char( "There are no stables.\n\r", ch );
		return;
	}

	set_char_color( AT_NOTE, ch );
	for ( stable = first_stable; stable; stable = stable->next )
	ch_printf( ch, "Keeper: %5d Stable: %3d Unstable: %3d Open: %2d Close: %2d\n\r",
		stable->keeper,	   stable->stable_cost, stable->unstable_cost,
		stable->open_hour,   stable->close_hour);
	return;
}

/* matthew */
/* All three are valid pointers (checked in do_give) */
void stable_give(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *keeper)
{
	CHAR_DATA* pet;
	MOB_INDEX_DATA *pPetIndex;
	AFFECT_DATA af;
		
	if ( obj->pObjNPC->r_vnum != keeper->GetInRoom()->vnum ) {
		do_say(keeper, "I'm sorry, but that is worthless to me.");
		return;
	}
	
	if ( ch->gold < keeper->pIndexData->pStable->unstable_cost ) {
		do_say(keeper, "I'm afraid you don't have enough money.");
		return;
	}

	pPetIndex = get_mob_index(obj->pObjNPC->m_vnum);

	if ( !pPetIndex ) {
		do_say(keeper, "I seem to have misplaced your animal.  I'll let you have this one instead.");
		pPetIndex = get_mob_index(MOB_VNUM_HORSE);
	}

	pet = create_mobile(pPetIndex);

	if ( !pet ) {
		do_say(keeper, "I'm sorry, I just don't feel like doing buisness right now.");
		return;
	}
	
	char_to_room(pet, ch->GetInRoom());

	act(AT_ACTION, "You hand $N your stable note and some money.",
					ch, NULL, keeper, TO_CHAR);
	act(AT_ACTION, "$N hands a stable note and some money to $n.",
					keeper, NULL, ch, TO_NOTVICT);
	act(AT_ACTION, "$N goes back to the stables and comes out with $n in tow.",
					pet, NULL, keeper, TO_ROOM);

	add_follower(pet, ch);

	af.type       = -1;
	af.duration   = -1;
	af.location   =  0;
	af.modifier   =  0;
	af.bitvector  = AFF_CHARM;
	affect_to_char(pet, &af);

	extract_obj(obj, TRUE);
}
