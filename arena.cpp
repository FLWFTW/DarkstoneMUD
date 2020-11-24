/* Matthew's Arena Routines */

#include <string.h> /* for strncpy */
#include "mud.h"

#include "paths.const.h"

/* Ksilyan */
ARENA_RANK_DATA ArenaRankings[NUMBER_OF_ARENA_RANKINGS];

ARENA_DATA current_arena; /* The current arena! */

// forward declaration
void do_look(Character * ch, const char* argument);

void do_arena(CHAR_DATA* ch, const char* argument)
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH] = "\0";

   if ( IS_NPC(ch) )
     {
	send_to_char("You can't do that!\r\n", ch);
	return;
     }

   argument = one_argument(argument, arg);

   if ( !str_cmp(arg, "enter") )
     {
	if ( !current_arena.open || !current_arena.pArea || !current_arena.pSRoom )
	  {
	     send_to_char("The arena is not open!\r\n", ch);
	     return;
	  }
    if ( get_timer(ch, TIMER_RECENTFIGHT) > 0
    &&  !IS_IMMORTAL(ch) )
    {
	set_char_color( AT_RED, ch );
	send_to_char( "Your adrenaline is pumping too hard to enter the arena!\n\r", ch );
	return;
    }
	else if ( in_arena(ch) )
	  {
	     send_to_char("You are already in the arena!\r\n", ch);
	     return;
	  }
        else if ( is_playbuilding(ch) )
          {
             send_to_char("You are not allowed in the arena while building.\r\n",ch);
             return;
          }
	else if ( ch->level < current_arena.min_level && !IS_IMMORTAL(ch) )
	  {
	     ch_printf(ch, "The arena is open from levels %d to %d.\r\n", current_arena.min_level,
		       current_arena.max_level);
	     return;
	  }
	else if ( ch->level > current_arena.max_level && !IS_IMMORTAL(ch) )
	  {
	     ch_printf(ch, "The arena is open from levels %d to %d.\r\n", current_arena.min_level,
		       current_arena.max_level);
	     return;
	  }

	act(AT_ARENA, "Great flames roar around $n, and he disappears.", ch,
	    NULL, NULL, TO_NOTVICT );
	act(AT_ARENA, "A great ball of flame engulfs you, and then deposits you elsewhere.", ch,
	    NULL, NULL, TO_CHAR );

    char_from_room(ch);
	char_to_room(ch, current_arena.pSRoom);

	act(AT_ARENA, "A great ball of flame descends from the sky, carrying $n with it.", 
        ch, NULL, NULL, TO_NOTVICT );
    do_look(ch, "auto");
	
	arena_clear_char(ch);
	
	sprintf(buf, "%s decides to show %s true worth by entering the arena!", NAME(ch), SEX_HIS(ch) );
	talk_channel(ch, buf, CHANNEL_ARENA, ""); 
     }
   else if ( !str_cmp(arg, "open") && get_trust(ch) >= 54 /* cheap fix for now - KSILYAN */ )
     {
	int min = 1;
	int max = MAX_LEVEL;
	int vnum = -1;
	char area_name[MAX_INPUT_LENGTH] = "arena.are";
	AREA_DATA* pArea;
	ROOM_INDEX_DATA* rid;
	
	if ( current_arena.open )
	  {
	     send_to_char("The arena is already open!\r\n", ch);
	     return;
	  }

    if ( argument && argument[0] != '\0' )
    {
        argument = one_argument(argument, area_name);
    }
    
	if ( argument && argument[0] != '\0' )
	  {
	     argument = one_argument(argument, arg);
	     min = atoi(arg);
	  }

	if ( argument && argument[0] != '\0' )
	  {
	     argument = one_argument(argument, arg);
	     max = atoi(arg);
	  }

	if ( min < 1 || min > MAX_LEVEL )
	  {
	     ch_printf(ch, "Invalid min level %d.\r\n", min);
	     return;
	  }
	else if ( max < min || max > MAX_LEVEL )
	  {
	     ch_printf(ch, "Invalid max level %d.\r\n", max);
	     return;
	  }

	  {    /* Ok, now find the area */
	     for ( pArea = first_area; pArea; pArea = pArea->next )
	       {
		  if ( !str_cmp(pArea->filename, area_name) )
		    break;
	       }
	
	     if ( !pArea )
	       {
		  ch_printf(ch, "Area %s not found!\r\n", area_name);
		  return;
	       }
	  }
	  {     /* Find a valid starting vnum */
	     vnum = pArea->low_r_vnum;
	     
	     if ( !(rid = get_room_index(vnum)) )
	       {
		  ch_printf(ch, "Room %s is not valid, searching for a valid start room...\r\n", vnum_to_dotted(vnum));
	       }
	     
	     for ( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; ++vnum )
	       {
		  rid = get_room_index(vnum);
		  
		  if ( rid )
		    break;
	       }
	     
	     if ( !rid )
	       {
		  send_to_char("No valid room found!\r\n", ch);
		  return;
	       }
	  }
  
	current_arena.min_level = min;
	current_arena.max_level = max;
	current_arena.open = TRUE;
	current_arena.pArea = pArea;
	current_arena.pSRoom = rid;
	strncpy(current_arena.opened_by, ch->getName().c_str(), 24);
	current_arena.opened_by[25] = '\0';
	
	sprintf(buf, "The immortals have deemed that fighters from the meager level of %d to the grand level of %d may battle in the arena.", min, max);
	talk_channel(ch, buf, CHANNEL_ARENA, ""); 
    ch_printf(ch, "Arena now open for levels %d to %d.\r\n", min, max);

    sprintf(buf, "%s opening arena %s from %d to %d",
            ch->getName().c_str(), pArea->filename, min, max);
    log_string_plus(buf, LOG_NORMAL, ch->level);
     }
   else if ( !str_cmp(arg, "close") && get_trust(ch) >= 54 /* cheap fix for now - KSILYAN */ )
     {
	if ( !current_arena.open)
	  {
	     send_to_char("There is no arena to close!\r\n", ch);
	     return;
	  }
	else
	  {
	     talk_channel(ch, "The immortals have taken pleasure in the gory spectacle of the arena, but have decided to now close it to mortals.", CHANNEL_ARENA, "");
	     send_to_char("The arena is now closed.\r\n", ch);
	     current_arena.open = FALSE;
         sprintf(buf, "%s closing the arena.", ch->getName().c_str());
         log_string_plus(buf, LOG_NORMAL, ch->level);
      }
    }
    else if ( !str_cmp(arg, "clear") && IS_IMMORTAL(ch) )
    {
        if ( !current_arena.pArea ) {
            send_to_char("No arena to clear.\r\n", ch);
            return;
        }

        current_arena.open  = FALSE;
        current_arena.pArea = NULL;
        current_arena.opened_by[0] = '\0';
        current_arena.min_level = current_arena.max_level = 0;
    }
	else if ( !str_cmp(arg, "top10") )
	{
		int x;
		ch_printf(ch, " Rank | Name\n");
		ch_printf(ch, "------+-------------------\n");
		for (x = 0; x < 10; x++)
		{
			ch_printf(ch, " %4d | %s\n", ArenaRankings[x].Rank, ArenaRankings[x].Name);
		}
	}
	else if ( !str_cmp(arg, "top20") )
	{
		int x;
		ch_printf(ch, " Rank | Name\n");
		ch_printf(ch, "------+-------------------\n");
		for (x = 10; x < 20; x++)
		{
			ch_printf(ch, " %4d | %s\n", ArenaRankings[x].Rank, ArenaRankings[x].Name);
		}
	}
	else if ( !str_cmp(arg, "top30") )
	{
		int x;
		ch_printf(ch, " Rank | Name\n");
		ch_printf(ch, "------+-------------------\n");
		for (x = 20; x < 30; x++)
		{
			ch_printf(ch, " %4d | %s\n", ArenaRankings[x].Rank, ArenaRankings[x].Name);
		}
	}
	else if ( !str_cmp(arg, "top40") )
	{
		int x;
		ch_printf(ch, " Rank | Name\n");
		ch_printf(ch, "------+-------------------\n");
		for (x = 30; x < 40; x++)
		{
			ch_printf(ch, " %4d | %s\n", ArenaRankings[x].Rank, ArenaRankings[x].Name);
		}
	}
    else
    {
        CHAR_DATA *vch;

        if ( in_arena(ch) ) {
            send_to_char("You are in the arena!\r\n", ch);
            return;
        }
        
	    ch_printf(ch, "The arena is currently %s.\r\n", (current_arena.open) ? "open" : "closed" );

        if ( current_arena.open ) {
    	    ch_printf(ch, "Min Level: %d\r\nMax Level: %d\r\n",
                    current_arena.min_level, current_arena.max_level);
        }

        if ( IS_IMMORTAL(ch) ) {
    	    ch_printf(ch, "The arena was opened by: %s\r\n", current_arena.opened_by);
        }

        if ( !current_arena.pArea )
    	    return;

        ch_printf(ch, "-----------------------------------------------------------------------\n");
        ch_printf(ch, "       Fighter Name      | Hitpoints |  Mana-Bp  |    Mv.    | Fighting\n");

        for ( vch = first_char; vch; vch = vch->next ) {
            if ( !in_arena(vch) || !can_see(ch, vch))
                continue;

            if ( IS_VAMPIRE(vch) ) {
                ch_printf(ch, "%-25s %5d/%5d %5d/%5d %5d/%5d",
                        PERS(vch, ch),
                        vch->hit,  vch->max_hit,
                        vch->pcdata->condition[COND_BLOODTHIRST], 10 + vch->level,
                        vch->move, vch->max_move
                        );
            } else {
                ch_printf(ch, "%-25s %5d/%5d %5d/%5d %5d/%5d",
                        PERS(vch, ch),
                        vch->hit,  vch->max_hit,
                        vch->mana, vch->max_mana,
                        vch->move, vch->max_move
                        );
            }

            if ( vch->GetVictim() ) {
                ch_printf(ch, "  %s\n", PERS(vch->GetVictim(), ch));
            } else {
                ch_printf(ch, "\n");
            }
        }
    }
}

void arena_clear_char(CHAR_DATA* ch)
{
    OBJ_DATA *pCont;
    
	ch->StopAllFights();
    //stop_fighting(ch, TRUE);
    stop_hunting(ch);
    stop_fearing(ch);
    stop_hating(ch);

    pCont = arena_strip_eq(ch);
  
    if ( ch->affected_by ) {
        while ( ch->first_affect ) {
	        affect_remove( ch, ch->first_affect );
        }
    }

   fix_affected_by( ch );
   ch->mental_state       = 0;
   ch->position	          = POS_STANDING;

   if ( !IS_NPC(ch) ) {
       ch->pcdata->condition
         [COND_FULL]          = 50;
       ch->pcdata->condition
         [COND_THIRST]        = 50;
       ch->pcdata->condition
         [COND_BLOODTHIRST]   = ch->level+10;
    }

    arena_reequip_char(ch, pCont);

   ch->hit	          = ch->max_hit;
   ch->mana	          = ch->max_mana;
   ch->move           = ch->max_move;
}

/* Moved from fight.c, seemed more appropriate here! :) */
/* Returns true if in the arena, false if not, duh! */
bool in_arena( CHAR_DATA *ch )
{
	if ( !current_arena.pArea )
		return false;

	// bugfix, Ksilyan - sep 16 2004
	// make sure ch's room is valid
	Room * room = ch->GetInRoom();
	
	if ( room && !str_cmp( room->area->filename, current_arena.pArea->filename ) )
		return true;

	return false;
}

/* arena strip eq */
/* strips the eq from a character, leaving wearlocs in the object
   data.
   Only for use with arena_reequip_char
   */
OBJ_DATA * arena_strip_eq(CHAR_DATA *ch) {
    OBJ_DATA* pCont;
    OBJ_DATA* pObj;

    pCont = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0); 
    
    while ( (pObj = ch->first_carrying) ) {
        sh_int wear_loc = pObj->wear_loc;

        if ( wear_loc != WEAR_NONE ) {
            unequip_char(ch, pObj);
        }

        obj_from_char(pObj);
        obj_to_obj(pObj, pCont);
        pObj->wear_loc = wear_loc;
    }

    return pCont;
} 

/* arena_reequip_char */
/* Takes the container from arena_strip_char and places the
   equipment back on the character
   */
void arena_reequip_char(CHAR_DATA* ch, OBJ_DATA *pCont) {
    OBJ_DATA *pObj;

    while ( (pObj = pCont->first_content) ) {
        sh_int wear_loc = pObj->wear_loc;
        pObj->wear_loc = WEAR_NONE;
        obj_from_obj(pObj);
        obj_to_char(pObj, ch);

        if ( wear_loc != WEAR_NONE ) {
            equip_char(ch, pObj, wear_loc);
        }
    }

    extract_obj(pCont, TRUE);
}


/* Ksilyan: write_arena_file
 * Write out the information to be read in at startup.
 */

void write_arena_file ()
{
	FILE * fp;

	if ( (fp = fopen(ARENA_RANK_FILE, "w")) != NULL )
	{
		int x;
		for (x = 0; x < NUMBER_OF_ARENA_RANKINGS; x++)
		{
			if (ArenaRankings[x].Rank > 0)
				fprintf(fp, "%3d %s\n", ArenaRankings[x].Rank, ArenaRankings[x].Name);
		}
	}
	else
	{
		bug("Unable to open arena rank file!");
	}
	fclose(fp);
}



/* Ksilyan: arena_rank_sort
 * function used to sort using qsort.
 */

int arena_rank_sort( const void * i, const void * j)
{
	ARENA_RANK_DATA * Rank1 = (ARENA_RANK_DATA *) i;
	ARENA_RANK_DATA * Rank2 = (ARENA_RANK_DATA *) j;

	if (Rank1->Rank > Rank2->Rank)
		return -1;
	if (Rank1->Rank == Rank2->Rank)
		return 0;
	
	return 1;
}

/* KSILYAN: update_arena_rank 
 * Takes a character and sees if he goes into the top list.
 * If so then update the array ArenaRankings.
 */

void update_arena_rank(CHAR_DATA * ch)
{
	sh_int rank;
	sh_int place;
	sh_int x;

	place = -1;

	if (IS_NPC(ch))
		return;
	
	if (IS_IMMORTAL(ch))
		return; /* don't want immies in the listing */
	
	rank = ch->pcdata->rating;

	/*
	 * Am I on the list? If so, update my ranking
	 * and then sort the list.
	 * While we're searching, find the place where
	 * I am to be inserted if I'm not there already.
	 */
	for (x = 0; x < NUMBER_OF_ARENA_RANKINGS; x++)
	{
		if ( !strcmp(ArenaRankings[x].Name, ch->getName().c_str()) )
		{
			ArenaRankings[x].Rank = rank;
			qsort(ArenaRankings, NUMBER_OF_ARENA_RANKINGS, sizeof(ARENA_RANK_DATA), arena_rank_sort);
			write_arena_file();
			return;
		}
		if (rank >= ArenaRankings[x].Rank)
		{
			if (place == -1)
				place = x;
		}
	}
	
	// If we don't have a place, then don't do anything.
	if ( place != -1 && place < NUMBER_OF_ARENA_RANKINGS )
	{
		/* Loop through backwards until we get to place
		 * and bump items down the list.
		 * We do -2 because we want the subscript of the before-last element.
		 */
		for (x = NUMBER_OF_ARENA_RANKINGS - 2; x >= place; x--)
		{
			ArenaRankings[x+1].Rank = ArenaRankings[x].Rank;
			strcpy(ArenaRankings[x+1].Name, ArenaRankings[x].Name);
		}
		// Finally, put the data into the right spot.
		ArenaRankings[place].Rank = rank;
		strcpy(ArenaRankings[place].Name, capitalize(ch->getName().c_str()));
	}
	
	write_arena_file();
	
	return;
}
