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
 *		     Character saving and loading module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include <dirent.h>

#include "mud.h"
#include "connection.h"
#include "connection_manager.h"

#include "paths.const.h"

/*
 * Increment with every major format change.
 */
#define SAVEVERSION	8

/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;

/*
 * Array of containers read for proper re-nesting of objects.
 */
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

/*
 * Local functions.
 */
void	fwrite_char	 ( CHAR_DATA *ch, FILE *fp ) ;
void	fread_char	 ( CHAR_DATA *ch, FILE *fp, bool preload) ;
void	write_corpses	 ( CHAR_DATA *ch, const char *name ) ;


/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int x,y;

	for ( x = 0; x < MAX_WEAR; x++ )
		for ( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;

	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		if ( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
		{
#ifdef USE_OBJECT_LEVELS
			if ( get_trust( ch ) >= obj->level )
			{
#endif
				for ( x = 0; x < MAX_LAYERS; x++ )
					if ( !save_equipment[obj->wear_loc][x] )
					{
						save_equipment[obj->wear_loc][x] = obj;
						break;
					}
				if ( x == MAX_LAYERS )
				{
					sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s",
							ch->getShort().c_str(), MAX_LAYERS, obj->wear_loc, obj->name_.c_str() );
					bug( buf, 0 );
				}
#ifdef USE_OBJECT_LEVELS
			}
			else
			{
				sprintf( buf, "%s had on %s:  ch->level = %d  obj->level = %d",
						ch->name, obj->name,
						ch->level, obj->level );
				bug( buf, 0 );
			}
#endif

			unequip_char(ch, obj);
		}
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA *ch )
{
    int x,y;

    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	   if ( save_equipment[x][y] != NULL )
	   {
		if ( quitting_char != ch )
		   equip_char(ch, save_equipment[x][y], x);
		save_equipment[x][y] = NULL;
	   }
	   else
		break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char strback[MAX_INPUT_LENGTH];
    FILE *fp;

	if(is_playbuilding(ch))
	{
		return; /* ALREADY SAVED - DON'T OVERWRITE! */
	}

    if ( !ch )
    {
        bug( "Save_char_obj: null ch!", 0 );
        return;
    }

    if ( IS_NPC(ch) ) {
        return;
    }

	extern void SanityCheck(Character*ch);
	SanityCheck(ch);

    saving_char = ch;

    /* save pc's clan's data while we're at it to keep the data in sync */
    if ( !IS_NPC(ch) && ch->pcdata->clan )
	    save_clan( ch->pcdata->clan );

    /* save deity's data to keep it in sync -ren */
    if ( !IS_NPC(ch) && ch->pcdata->deity )
    	save_deity( ch->pcdata->deity );

    //if ( ch->GetConnection() && ch->GetConnection()->original )
    	//ch = ch->GetConnection()->original;
	if ( ch->GetConnection() )
		ch = ch->GetConnection()->GetOriginalCharacter();

    de_equip_char( ch );

    ch->secSaveTime = secCurrentTime;

    if ( fCopyOver ) {
        sprintf(strsave, "%splayers/%s", COPYOVER_DIR, capitalize(ch->getName().c_str()));
    } else {
        sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(ch->getName().c_str()[0]),
				capitalize( ch->getName().c_str() ) );
    }

    /*
     * Auto-backup pfile (can cause lag with high disk access situtations
     */
    if ( IS_SET( sysdata.save_flags, SV_BACKUP ) && !fCopyOver )
    {
    	sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower(ch->getName().c_str()[0]),
				 capitalize( ch->getName().c_str() ) );
    	rename( strsave, strback );
    }

    /*
     * Save immortal stats, level & vnums for wizlist		-Thoric
     * and do_vnums command
     *
     * Also save the player flags so we the wizlist builder can see
     * who is a guest and who is retired.
     */
#if 0 /* was... */
    if  ( ch->level >= LEVEL_IMMORTAL )
#endif
    if  (ch->pcdata->r_range_lo
      || ch->pcdata->m_range_lo
      || ch->pcdata->o_range_lo)
    {
      	sprintf( strback, "%s%s", GOD_DIR, capitalize( ch->getName().c_str() ) );

      	if  ( ( fp = fopen( strback, "w" ) ) == NULL )
       	{
	        bug( "Save_god_level: fopen", 0 );
    	    perror( strsave );
      	}
      	else
      	{
	        fprintf( fp, "Level        %d\n", ch->level );
    	    fprintf( fp, "Pcflags      %d\n", ch->pcdata->flags );

            if  ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	       	    fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
	  			        ch->pcdata->r_range_hi	);

            if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
                fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
	  				       ch->pcdata->o_range_hi	);
    	    if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	            fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
	  				       ch->pcdata->m_range_hi	);
    	    fclose( fp );
      	}
    }

    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
        bug( "Save_char_obj: fopen", 0 );
        perror( strsave );
    }
    else
    {
        fwrite_char( ch, fp );

        if ( ch->first_carrying )
            fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY );


        fprintf( fp, "#END\n" );

        fclose( fp );
    }

    re_equip_char( ch );

    write_corpses(ch, NULL);
    quitting_char = NULL;
    saving_char   = NULL;
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, track;
    SkillType *skill;

    if ( IS_NPC(ch) ) {
        fprintf(fp, "#%s %d\n", "MOB", ch->pIndexData->vnum );
    } else {
        fprintf( fp, "#PLAYER\n");
    }

    fprintf( fp, "Version      %d\n",   SAVEVERSION		);
    fprintf( fp, "Name         %s~\n",	ch->getName().c_str()		);

    if ( ch->getShort(false).length() > 0 )
        fprintf( fp, "ShortDescr   %s~\n",	ch->getShort().c_str() );
    if ( ch->longDesc_.length() > 0 )
        fprintf( fp, "LongDescr    %s~\n",	ch->longDesc_.c_str() );
    if ( ch->description_.length() > 0 )
        fprintf( fp, "Description  %s~\n",	ch->description_.c_str());
    if ( ch->enterDesc_.length() > 0 )
        fprintf( fp, "Enterdescr   %s~\n",  ch->enterDesc_.c_str());
	if ( ch->exitDesc_.length() > 0 )
	    fprintf( fp, "Exitdescr    %s~\n",  ch->exitDesc_.c_str() );
	if ( ch->getMovementMessage() != "" && ch->getMovementMessage() != "default" )
		fprintf( fp, "EntryMessage %s~\n", ch->getMovementMessage().c_str() );

    fprintf( fp, "Sex          %d\n",	ch->sex			);
    fprintf( fp, "Class        %d\n",	ch->Class		);
    fprintf( fp, "Race         %d\n",	ch->race		);
    fprintf( fp, "Languages    %d %d\n", ch->speaks, ch->speaking );
    fprintf( fp, "Level        %d\n",	ch->level		);
    if ( ch->trust )
        fprintf( fp, "Trust        %d\n",	ch->trust		);
    if ( ch->getScent() != NULL )
	fprintf( fp, "Scent	   %d\n",	ch->getScentId()		);
    fprintf( fp, "Played       %d\n",
                ch->played + (int) (secCurrentTime- ch->secLogonTime)		);
    fprintf( fp, "Room         %d\n",
	            (  ch->GetInRoom() == get_room_index( ROOM_VNUM_LIMBO )
            	&& ch->GetWasInRoom() )
	            ? ch->GetWasInRoom()->vnum
        	    : ch->GetInRoom()->vnum );

    fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
    	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
	fprintf( fp, "BaseMaxHp    %d\n", ch->BaseMaxHp );
	fprintf( fp, "BaseMaxMana  %d\n", ch->BaseMaxMana );
	fprintf( fp, "BaseMaxMove  %d\n", ch->BaseMaxMove );
    fprintf( fp, "Gold         %d\n",	ch->gold		);
    fprintf( fp, "Exp          %d\n",	ch->exp			);
    if ( ch->act )
        fprintf( fp, "Act          %d\n", ch->act			);
    if ( ch->affected_by )
        fprintf( fp, "AffectedBy   %d\n",	ch->affected_by		);

	if ( !fCopyOver )
    {
        int tmp;
		tmp = ch->position;
		tmp = tmp == POS_FIGHTING ? POS_STANDING : tmp;
		tmp = tmp == POS_MOUNTED  ? POS_STANDING : tmp;

		fprintf(fp, "Position     %d\n", tmp);
	}

    fprintf( fp, "Practice     %d\n",	ch->practice		);
    fprintf( fp, "SavingThrows %d %d %d %d %d\n",
                ch->saving_poison_death,
                ch->saving_wand,
                ch->saving_para_petri,
                ch->saving_breath,
                ch->saving_spell_staff			);
    fprintf( fp, "Alignment    %d\n",	ch->alignment		);

    fprintf( fp, "Hitroll      %d\n",	ch->hitroll		);
    fprintf( fp, "Damroll      %d\n",	ch->damroll		);


    fprintf( fp, "Armor        %d\n",	ch->armor		);
    if ( ch->wimpy )
        fprintf( fp, "Wimpy        %d\n",	ch->wimpy		);
    if ( ch->deaf )
        fprintf( fp, "Deaf         %d\n",	ch->deaf		);
    if ( ch->resistant )
        fprintf( fp, "Resistant    %d\n",	ch->resistant		);
    if ( ch->immune )
        fprintf( fp, "Immune       %d\n",	ch->immune		);
    if ( ch->susceptible )
        fprintf( fp, "Susceptible  %d\n",	ch->susceptible		);

    if ( ch->mental_state != -10 )
        fprintf( fp, "Mentalstate  %d\n",	ch->mental_state	);

    if ( IS_NPC(ch) )
    {
        fprintf( fp, "Vnum         %d\n",	ch->pIndexData->vnum	);
        fprintf( fp, "Mobinvis     %d\n",	ch->mobinvis		);
    }
    else
	{
		{
			int x;
			fprintf( fp, "MaxColors    %d\n",     MAX_PCCOLORS            );
			fprintf( fp, "Colors       ");
			for ( x = 0; x < MAX_PCCOLORS ; x++ )
				fprintf( fp, "%d ", ch->pcdata->pc_colors[x]);
			fprintf (fp, "\n");
		}
		// Ksilyan:
		{
			int x;
			fprintf( fp, "PlayerThief  %d ", MAX_CITIES);
			for ( x = 0; x < MAX_CITIES ; x++ )
				fprintf( fp, "%d ", ch->pcdata->citythief[x]);
			fprintf( fp, "\n");

			fprintf( fp, "PlayerWanted %d ", MAX_CITIES);
			for ( x = 0; x < MAX_CITIES; x++)
				fprintf( fp, "%d ", ch->pcdata->CityWanted[x]);
			fprintf( fp, "\n");
		}
		if ( ch->pcdata && ch->pcdata->secOutcastTime)
			fprintf( fp, "Outcast_time %ld\n",ch->pcdata->secOutcastTime);
		if ( ch->pcdata && ch->pcdata->secRestoreTime)
			fprintf( fp, "Restore_time %ld\n",ch->pcdata->secRestoreTime);
		if ( ch->pcdata->bamfIn_.length() > 0 )
			fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfIn_.c_str()	);
		if ( ch->pcdata->bamfOut_.length() > 0 )
			fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfOut_.c_str()	);
		if ( ch->pcdata->rank_.length() > 0 )
			fprintf( fp, "Rank         %s~\n",	ch->pcdata->rank_.c_str()	);
		if ( ch->pcdata->bestowments_.length() > 0 )
			fprintf( fp, "Bestowments  %s~\n", 	ch->pcdata->bestowments_.c_str() );
		if ( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
			fprintf( fp, "Homepage     %s~\n",	ch->pcdata->homepage	);
		if ( ch->pcdata->email && ch->pcdata->email[0] != '\0' )
			fprintf( fp, "Email        %s~\n", ch->pcdata->email);
		if ( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
			fprintf( fp, "Bio          %s~\n",	ch->pcdata->bio 	);
		if ( ch->pcdata->authedBy_.length() > 0 )
			fprintf( fp, "AuthedBy     %s~\n",	ch->pcdata->authedBy_.c_str() );
		if ( ch->pcdata->min_snoop )
			fprintf( fp, "Minsnoop     %d\n",	ch->pcdata->min_snoop	);
		if ( ch->pcdata->prompt_.length() > 0 )
			fprintf( fp, "Prompt       %s~\n",	ch->pcdata->prompt_.c_str()	);
		if ( ch->pcdata->prompt2_.length() > 0 )
			fprintf( fp, "Prompt2      %s~\n",	ch->pcdata->prompt2_.c_str()	);
		if ( ch->pcdata->pagerlen != 24 )
			fprintf( fp, "Pagerlen     %d\n",	ch->pcdata->pagerlen	);

		fprintf( fp, "Favor	       %d\n",	ch->pcdata->favor	);
		fprintf( fp, "Title        %s~\n",	ch->pcdata->title_.c_str()	);
		fprintf( fp, "Glory        %d\n",   ch->pcdata->quest_curr  );
		fprintf( fp, "DGlory       %d\n",   ch->pcdata->quest_deci  );
		fprintf( fp, "MGlory       %d\n",   ch->pcdata->quest_accum );
		fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
		fprintf( fp, "Flags        %d\n",	ch->pcdata->flags	);
		fprintf( fp, "MKills       %d\n",	ch->pcdata->mkills	);
		fprintf( fp, "MDeaths      %d\n",	ch->pcdata->mdeaths	);
		/* Testaur - make these two fields optional for now */
		if(ch->pcdata->bank_gold)
			fprintf( fp,     "BankGold     %d\n",   ch->pcdata->bank_gold );
		if(ch->pcdata->recall_room)
			fprintf( fp,     "RecallRoom   %d\n",   ch->pcdata->recall_room);
		if(ch->pcdata->ethos)
			fprintf( fp,     "Ethos        %d\n",   ch->pcdata->ethos);

		fprintf( fp,     "Rating       %d\n",   ch->pcdata->rating );

		if ( IS_IMMORTAL( ch ) )
		{
			fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );
		}
		if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
			fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
					ch->pcdata->r_range_hi	);
		if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
			fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
					ch->pcdata->o_range_hi	);
		if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
			fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
					ch->pcdata->m_range_hi	);


		if ( ch->pcdata->council)
			fprintf( fp, "Council      %s~\n", 	ch->pcdata->councilName_.c_str() );
		if ( ch->pcdata->deityName_.length() > 0 )
			fprintf( fp, "Deity	     %s~\n",	ch->pcdata->deityName_.c_str()	 );
		if ( ch->pcdata->clanName_.length() > 0 )
			fprintf( fp, "Clan         %s~\n",	ch->pcdata->clanName_.c_str()	);
		if ( ch->pcdata->secReleaseDate > secCurrentTime)
			fprintf( fp, "Helled       %d %s~\n",
					(int)ch->pcdata->secReleaseDate, ch->pcdata->helledBy_.c_str() );
		if ( ch->pcdata->pkills )
			fprintf( fp, "PKills       %d\n",	ch->pcdata->pkills	);
		if ( ch->pcdata->pdeaths )
			fprintf( fp, "PDeaths      %d\n",	ch->pcdata->pdeaths	);

		if ( ch->pcdata->illegal_pk )
			fprintf( fp, "IllegalPK    %d\n",	ch->pcdata->illegal_pk	);
		fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
				ch->perm_str,
				ch->perm_int,
				ch->perm_wis,
				ch->perm_dex,
				ch->perm_con,
				ch->perm_cha,
				ch->perm_lck );

		fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
				ch->mod_str,
				ch->mod_int,
				ch->mod_wis,
				ch->mod_dex,
				ch->mod_con,
				ch->mod_cha,
				ch->mod_lck );

		fprintf( fp, "Condition    %d %d %d %d\n",
				ch->pcdata->condition[0],
				ch->pcdata->condition[1],
				ch->pcdata->condition[2],
				ch->pcdata->condition[3] );

		if ( ch->GetConnection() && ch->GetConnection()->GetHost() )
			fprintf( fp, "Site         %s\n", ch->GetConnection()->GetHost() );
		else
			fprintf( fp, "Site         (Link-Dead)\n" );

		for ( sn = 1; sn < top_sn; sn++ )
		{
			if ( skill_table[sn]->name_.c_str() && ch->pcdata->learned[sn] > 0 )
			{
				switch( skill_table[sn]->type )
				{
					default:
						fprintf( fp, "Skill        %d '%s'\n",
								ch->pcdata->learned[sn], skill_table[sn]->name_.c_str() );
						break;
					case SKILL_SPELL:
						fprintf( fp, "Spell        %d '%s'\n",
								ch->pcdata->learned[sn], skill_table[sn]->name_.c_str() );
						break;
					case SKILL_WEAPON:
						fprintf( fp, "Weapon       %d '%s'\n",
								ch->pcdata->learned[sn], skill_table[sn]->name_.c_str() );
						break;
					case SKILL_TONGUE:
						fprintf( fp, "Tongue       %d '%s'\n",
								ch->pcdata->learned[sn], skill_table[sn]->name_.c_str() );
						break;
				}
			}
		}
	}

    for ( paf = ch->first_affect; paf; paf = paf->next )
    {
    	if ( paf->type >= 0 && (skill=get_skilltype(paf->type)) == NULL )
    	    continue;

    	if ( paf->type >= 0 && paf->type < TYPE_PERSONAL )
            fprintf( fp, "AffectData   '%s' %3d %3d %3d %10d\n",
    	        skill->name_.c_str(),
        	    paf->duration,
        	    paf->modifier,
        	    paf->location,
        	    paf->bitvector
    	    );
    	else
            fprintf( fp, "Affect       %3d %3d %3d %3d %10d\n",
                paf->type,
                paf->duration,
                paf->modifier,
                paf->location,
                paf->bitvector
            );
    }

    if ( !IS_NPC(ch) ) {

        track = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_HERO_MAX, MAX_KILLTRACK );

        for ( sn = 0; sn < track; sn++ )
        {
    	    if ( ch->pcdata->killed[sn].vnum == 0 )
    	        break;

            fprintf( fp, "Killed       %d %d\n",
    		ch->pcdata->killed[sn].vnum,
    		ch->pcdata->killed[sn].count );
        }
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest,
		 sh_int os_type )
{
	ExtraDescData *ed;
	AFFECT_DATA *paf;
	sh_int wear, wear_loc, x;

	if ( iNest >= MAX_NEST )
	{
		bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
		return;
	}

	/*
	 * Slick recursion to write lists backwards,
	 *   so loading them will load in forwards order.
	 */
	if ( obj->prev_content && os_type != OS_CORPSE )
		fwrite_obj( ch, obj->prev_content, fp, iNest, os_type );

	if ( IS_OBJ_STAT(obj, ITEM_BURIED) ) {
		if ( os_type != OS_BURIED ) {
			return;
		}
	} else if ( os_type == OS_BURIED ) {
		return;
	}

	/*
	 * Castrate storage characters.
	 */
#ifdef USE_OBJECT_LEVELS
	if ( (ch && ch->level < obj->level)||
#else
			if(
#endif
				( obj->item_type == ITEM_KEY && !IS_OBJ_STAT(obj, ITEM_CLANOBJECT )))
			return;

			/*
			 * Catch deleted objects					-Thoric
			 */
			if ( obj_extracted(obj) )
			return;

			/*
			 * Do NOT save prototype items!				-Thoric
			 */
			if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
			return;

			/* Corpse saving. -- Altrag */
			fprintf( fp, (os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n") );

			if ( iNest )
				fprintf( fp, "Nest         %d\n",	iNest		     );
			if ( obj->count > 1 )
				fprintf( fp, "Count        %d\n",	obj->count	     );
			if ( obj->name_ != obj->pIndexData->name_ )
				fprintf( fp, "Name         %s~\n",	obj->name_.c_str()	     );
			if ( obj->shortDesc_ != obj->pIndexData->shortDesc_ )
				fprintf( fp, "ShortDescr   %s~\n",	obj->shortDesc_.c_str()     );
			if ( obj->longDesc_ != obj->pIndexData->longDesc_ )
				fprintf( fp, "Description  %s~\n",	obj->longDesc_.c_str() );
			if ( obj->actionDesc_ != obj->pIndexData->actionDesc_ )
				fprintf( fp, "ActionDesc   %s~\n",	obj->actionDesc_.c_str()     );

			fprintf( fp, "Vnum         %d\n",	obj->pIndexData->vnum     );

			for (int i = 0; i <= 1; i++)
			{
				if ( obj->Holders[i]->Name != "" )
				{
					fprintf( fp, "Holder%dName  %s~\n", i, obj->Holders[i]->Name.c_str());
					fprintf( fp, "Holder%dHost  %s~\n", i, obj->Holders[i]->Host.c_str());
					fprintf( fp, "Holder%dTime  %s~\n", i, obj->Holders[i]->Time.c_str());
					fprintf( fp, "Holder%dAccount %s~\n", i, obj->Holders[i]->Account.c_str());
				}
			}

			if ( os_type == OS_CORPSE && obj->GetInRoom() )
				fprintf( fp, "Room         %d\n",   obj->GetInRoom()->vnum         );
			if ( obj->extra_flags != obj->pIndexData->extra_flags )
				fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags     );
			if (obj->extra_flags_2 != obj->pIndexData->extra_flags_2)
				fprintf( fp, "ExtraFlags2  %d\n", obj->extra_flags_2	);
			if ( obj->wear_flags != obj->pIndexData->wear_flags )
				fprintf( fp, "WearFlags    %d\n",	obj->wear_flags	     );
			wear_loc = -1;
			for ( wear = 0; wear < MAX_WEAR; wear++ )
				for ( x = 0; x < MAX_LAYERS; x++ )
					if ( obj == save_equipment[wear][x] )
					{
						wear_loc = wear;
						break;
					}
					else
						if ( !save_equipment[wear][x] )
							break;
			if ( wear_loc != -1 )
				fprintf( fp, "WearLoc      %d\n",	wear_loc	     );
			if ( obj->item_type != obj->pIndexData->item_type )
				fprintf( fp, "ItemType     %d\n",	obj->item_type	     );
			if ( obj->weight != obj->pIndexData->weight )
				fprintf( fp, "Weight       %d\n",	obj->weight		     );
#ifdef USE_OBJECT_LEVELS
			if ( obj->level )
				fprintf( fp, "Level        %d\n",	obj->level		     );
#endif
			if ( obj->timer )
				fprintf( fp, "Timer        %d\n",	obj->timer		     );
			if ( obj->cost != obj->pIndexData->cost )
				fprintf( fp, "Cost         %d\n",	obj->cost		     );
			if ( obj->max_condition != obj->pIndexData->max_condition)
				fprintf( fp, "MaxCondition %d\n", obj->max_condition	 );
			fprintf( fp, "Condition    %d\n", obj->condition		 );
			if ( obj->value[0] || obj->value[1] || obj->value[2]
					||   obj->value[3] || obj->value[4] || obj->value[5] )
				fprintf( fp, "Values       %d %d %d %d %d %d\n",
						obj->value[0], obj->value[1], obj->value[2],
						obj->value[3], obj->value[4], obj->value[5]     );

			switch ( obj->item_type )
			{
				case ITEM_PILL: /* was down there with staff and wand, wrongly - Scryn */
				case ITEM_POTION:
				case ITEM_SCROLL:
					if ( IS_VALID_SN(obj->value[1]) )
						fprintf( fp, "Spell 1      '%s'\n",
								skill_table[obj->value[1]]->name_.c_str() );

					if ( IS_VALID_SN(obj->value[2]) )
						fprintf( fp, "Spell 2      '%s'\n",
								skill_table[obj->value[2]]->name_.c_str() );

					if ( IS_VALID_SN(obj->value[3]) )
						fprintf( fp, "Spell 3      '%s'\n",
								skill_table[obj->value[3]]->name_.c_str() );

					break;

				case ITEM_STAFF:
				case ITEM_WAND:
					if ( IS_VALID_SN(obj->value[3]) )
						fprintf( fp, "Spell 3      '%s'\n",
								skill_table[obj->value[3]]->name_.c_str() );

					break;
				case ITEM_SALVE:
					if ( IS_VALID_SN(obj->value[4]) )
						fprintf( fp, "Spell 4      '%s'\n",
								skill_table[obj->value[4]]->name_.c_str() );

					if ( IS_VALID_SN(obj->value[5]) )
						fprintf( fp, "Spell 5      '%s'\n",
								skill_table[obj->value[5]]->name_.c_str() );
					break;
			}

			for ( paf = obj->first_affect; paf; paf = paf->next )
			{
				/*
				 * Save extra object affects				-Thoric
				 */
				if ( paf->type < 0 || paf->type >= top_sn )
				{
					fprintf( fp, "Affect       %d %d %d %d %d\n",
							paf->type,
							paf->duration,
							((paf->location == APPLY_WEAPONSPELL
							  || paf->location == APPLY_WEARSPELL
							  || paf->location == APPLY_REMOVESPELL
							  || paf->location == APPLY_STRIPSN)
							 && IS_VALID_SN(paf->modifier))
							? skill_table[paf->modifier]->slot : paf->modifier,
							paf->location,
							paf->bitvector
						   );
				}
				else
					fprintf( fp, "AffectData   '%s' %d %d %d %d\n",
							skill_table[paf->type]->name_.c_str(),
							paf->duration,
							((paf->location == APPLY_WEAPONSPELL
							  || paf->location == APPLY_WEARSPELL
							  || paf->location == APPLY_REMOVESPELL
							  || paf->location == APPLY_STRIPSN)
							 && IS_VALID_SN(paf->modifier))
							? skill_table[paf->modifier]->slot : paf->modifier,
							paf->location,
							paf->bitvector
						   );
			}

			for ( ed = obj->first_extradesc; ed; ed = ed->next )
				fprintf( fp, "ExtraDescr   %s~ %s~\n",
						ed->keyword_.c_str(), ed->description_.c_str() );


			/* matthew */
			if ( obj->pObjNPC ) {
				fprintf( fp, "ObjNPC_rvnum  %d\n",
						obj->pObjNPC->r_vnum);
				fprintf( fp, "ObjNPC_mvnum  %d\n",
						obj->pObjNPC->m_vnum);
			}

			fprintf( fp, "End\n\n" );

			if ( obj->first_content )
				fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY );

			return;
}

/*
 * See if the account is already loaded, and, if is, return it
 */
ACCOUNT_DATA* find_account_data(const char* account)
{
	ACCOUNT_DATA* act;

	for ( act = first_account; act; act = act->next )
	{
		if ( !str_cmp(act->email, account) )
		{
			return act;
		}
	}

	return NULL;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( PlayerConnection *d, const char *name, bool preload )
{
	char strsave[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	FILE *fp;
	bool found;
	struct stat fst;
	int i, x;
	extern FILE *fpArea;
	extern char strArea[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	ch = new Character();

	for ( x = 0; x < MAX_WEAR; x++ )
	{
		for ( i = 0; i < MAX_LAYERS; i++ )
		{
			save_equipment[x][i] = NULL;
		}
	}

	loading_char = ch;

	ch->pcdata = new PlayerData;
	d->CurrentCharId = ch->GetId();

	ch->SetConnection(d);
	ch->setName			( name );
	ch->act 			= PLR_BLANK | PLR_COMBINE | PLR_PROMPT;
	ch->perm_str			= 13;
	ch->perm_int			= 13;
	ch->perm_wis			= 13;
	ch->perm_dex			= 13;
	ch->perm_con			= 13;
	ch->perm_cha			= 13;
	ch->perm_lck			= 13;
	ch->pcdata->condition[COND_THIRST]	= 48;
	ch->pcdata->condition[COND_FULL]	= 48;
	ch->pcdata->condition[COND_BLOODTHIRST] = 10;
	ch->pcdata->wizinvis		= 0;
	ch->mental_state			= -10;
	ch->mobinvis			= 0;

	ch->saving_poison_death 		= 0;
	ch->saving_wand			= 0;
	ch->saving_para_petri		= 0;
	ch->saving_breath			= 0;
	ch->saving_spell_staff		= 0;
	//    ch->comments                        = NULL;    /* comments */

	found = FALSE;

	if ( fCopyOver ) {
		sprintf(strsave, "%splayers/%s", COPYOVER_DIR, capitalize(ch->getName().c_str()));
	} else {
		sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
				capitalize( name ) );
	}

	if ( stat( strsave, &fst ) != -1 )
	{
		if ( fst.st_size == 0 )
		{
			sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower(name[0]),
					capitalize( name ) );
			send_to_char( "Restoring your backup player file...", ch );
		}
		else
		{
			sprintf( buf, "%s player data for: %s (%dK)",
					preload ? "Preloading" : "Loading", ch->getName().c_str(),
					(int) fst.st_size/1024 );
			log_string_plus( buf, LOG_COMM, LEVEL_STONE_MASTER );
		}
	}
	/* else no player file */

	if ( ( fp = fopen( strsave, "r" ) ) != NULL )
	{
		int iNest;

		if ( fCopyOver ) {
			unlink(strsave); /* can still read from it */
		}

		for ( iNest = 0; iNest < MAX_NEST; iNest++ )
			rgObjNest[iNest] = NULL;

		found = TRUE;
		/* Cheat so that bug will show line #'s -- Altrag */
		fpArea = fp;
		strcpy(strArea, strsave);
		for ( ; ; )
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
				bug( "Load_char_obj: # not found.", 0 );
				bug( name, 0 );
				break;
			}

			word = fread_word( fp );
			if ( !str_cmp( word, "PLAYER" ) )
			{
				fread_char ( ch, fp, preload );
				if ( preload )
					break;
			}
			else
				if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
					fread_obj  ( ch, fp, OS_CARRY_NOINC );

			//    	    else
			//	            if ( !str_cmp( word, "COMMENT") )
			//            		fread_comment(ch, fp );		/* Comments	*/
				else
					if ( !str_cmp( word, "END"    ) )	/* Done		*/
						break;
					else
					{
						bug( "Load_char_obj: bad section.", 0 );
						bug( name, 0 );
						break;
					}
		}

		fclose( fp );
		fpArea = NULL;
		strcpy(strArea, "$");
	}

	if ( !found )
	{
		ch->editor			= NULL;
		ch->pcdata->pwd			= str_dup( "" );
		ch->pcdata->homepage		= str_dup( "" );
		ch->pcdata->email       = str_dup("");
		ch->pcdata->bio 		= STRALLOC( "" );
	}
	else
	{
		if ( ch->pcdata->clanName_.length() == 0 )
			ch->pcdata->clan	= NULL;

		if ( ch->pcdata->councilName_.length() == 0 )
			ch->pcdata->council 	= NULL;

		if ( ch->pcdata->deityName_.length() == 0 )
			ch->pcdata->deity	 = NULL;

		if ( !ch->pcdata->bio )
			ch->pcdata->bio	 = STRALLOC( "" );

		if ( !IS_NPC( ch ) && (IS_IMMORTAL( ch ) || ch->pcdata->r_range_lo > 0 ))
		{
			if ( ch->pcdata->wizinvis < 2 )
				ch->pcdata->wizinvis = ch->level;

			assign_area( ch );
		}

		if ( file_ver <= 6 )
		{
			// We need to recalculate stats, hitpoints and mana.
			ch->perm_str = ch->perm_int = ch->perm_con = ch->perm_wis =
				ch->perm_dex = ch->perm_cha = ch->perm_lck = 13;
			ch->mod_str = ch->mod_int = ch->mod_con = ch->mod_wis =
				ch->mod_dex = ch->mod_cha = ch->mod_lck = 0;

			switch ( class_table[ch->Class]->attr_prime )
			{
				case APPLY_STR: ch->perm_str = 16; break;
				case APPLY_INT: ch->perm_int = 16; break;
				case APPLY_WIS: ch->perm_wis = 16; break;
				case APPLY_DEX: ch->perm_dex = 16; break;
				case APPLY_CON: ch->perm_con = 16; break;
				case APPLY_CHA: ch->perm_cha = 16; break;
				case APPLY_LCK: ch->perm_lck = 16; break;
			}
			ch->perm_str     += race_table[ch->race].str_plus;
			ch->perm_int     += race_table[ch->race].int_plus;
			ch->perm_wis     += race_table[ch->race].wis_plus;
			ch->perm_dex     += race_table[ch->race].dex_plus;
			ch->perm_con     += race_table[ch->race].con_plus;
			ch->perm_cha     += race_table[ch->race].cha_plus;
			ch->perm_lck     += race_table[ch->race].lck_plus;

			// Now, recalculate mana and hp.
			ch->BaseMaxHp = 20;
			ch->BaseMaxMana = ch->BaseMaxMove = 100;
			for ( int i = 2; i <= ch->level; i++ )
			{
				ch->BaseMaxHp += ch->GetLevelGainHP();
				ch->BaseMaxMana += ch->GetLevelGainMana();
				ch->BaseMaxMove += ch->GetLevelGainMove();
			}
			ch->max_hit = ch->hit = ch->BaseMaxHp;
			ch->max_mana = ch->mana = ch->BaseMaxMana;
			ch->max_move = ch->move = ch->BaseMaxMove;
		}
		else if ( file_ver == 7 )
		{
			ch->max_hit = ch->hit = ch->BaseMaxHp;
			ch->max_mana = ch->mana = ch->BaseMaxMana;
			ch->max_move = ch->move = ch->BaseMaxMove;

			ch->mod_str = ch->mod_int = ch->mod_con = ch->mod_wis =
				ch->mod_dex = ch->mod_cha = ch->mod_lck = 0;
		}
		else
		{
			for ( i = 0; i < MAX_WEAR; i++ )
			{
				for ( x = 0; x < MAX_LAYERS; x++ )
				{
					if ( save_equipment[i][x] )
					{
						equip_char( ch, save_equipment[i][x], i );
						save_equipment[i][x] = NULL;
					}
					else
					{
						break;
					}
				}
			}
		}

		extern void SanityCheck(Character*);
		SanityCheck(ch);
	}

	loading_char = NULL;
	return found;
}



/*
 * Read in a char.
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

void fread_char( CHAR_DATA *ch, FILE *fp, bool preload )
{
	char buf[MAX_STRING_LENGTH];

	char sbuf[MAX_STRING_LENGTH];
	char *sname;

	char *line;
	const char *word;
	int x1, x2, x3, x4, x5, x6, x7;
	sh_int killcnt;
	bool fMatch;
	int max_colors;

	int i;

	file_ver = 0;
	killcnt = 0;

	if ( ch->pcdata )
		memcpy(&ch->pcdata->pc_colors, &default_set, sizeof(default_set));

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
				KEY( "Act",		ch->act,		fread_number( fp ) );
				KEY( "AffectedBy",	ch->affected_by,	fread_number( fp ) );
				KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
				KEY( "Armor",	ch->armor,		fread_number( fp ) );

				if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
				{
					AFFECT_DATA *paf;

					if ( preload )
					{
						fMatch = TRUE;
						fread_to_eol( fp );
						break;
					}

					CREATE( paf, AFFECT_DATA, 1 );

					if ( !str_cmp( word, "Affect" ) )
					{
						paf->type	= fread_number( fp );
					}
					else
					{
						int sn;

						sname = fread_word(fp);

						if ( (sn=skill_lookup(sname)) < 0 )
						{
							if ( (sn=herb_lookup(sname)) < 0 )
							{
								sprintf(sbuf,"Fread_char: unknown skill '%s'",sname);
								bug( sbuf, 0 );
							}
							else
								sn += TYPE_HERB;
						}

						paf->type = sn;
					}

					paf->duration	= fread_number( fp );
					paf->modifier	= fread_number( fp );
					paf->location	= fread_number( fp );
					paf->bitvector	= fread_number( fp );
					LINK(paf, ch->first_affect, ch->last_affect, next, prev );
					fMatch = TRUE;
					break;
				}

				if  ( !str_cmp( word, "AttrMod"  ) )
				{
					line = fread_line( fp );
					x1=x2=x3=x4=x5=x6=x7=13;
					sscanf( line, "%d %d %d %d %d %d %d",
							&x1, &x2, &x3, &x4, &x5, &x6, &x7 );
					ch->mod_str = x1;
					ch->mod_int = x2;
					ch->mod_wis = x3;
					ch->mod_dex = x4;
					ch->mod_con = x5;
					ch->mod_cha = x6;
					ch->mod_lck = x7;

					if (!x7)
						ch->mod_lck = 0;

					fMatch = TRUE;
					break;
				}

				if ( !str_cmp( word, "AttrPerm" ) )
				{
					line = fread_line( fp );
					x1=x2=x3=x4=x5=x6=x7=0;
					sscanf( line, "%d %d %d %d %d %d %d",
							&x1, &x2, &x3, &x4, &x5, &x6, &x7 );
					ch->perm_str = x1;
					ch->perm_int = x2;
					ch->perm_wis = x3;
					ch->perm_dex = x4;
					ch->perm_con = x5;
					ch->perm_cha = x6;
					ch->perm_lck = x7;
					if (!x7 || x7 == 0)
						ch->perm_lck = 13;
					fMatch = TRUE;
					break;
				}

				if ( ch->pcdata ) {
					KEY( "AuthedBy",	ch->pcdata->authedBy_,	fread_string_noheap( fp ) );
				}

				break;

			case 'B':
				if ( ch->pcdata ) {
					KEY( "Bamfin",	ch->pcdata->bamfIn_,	fread_string_noheap( fp ) );
					KEY( "Bamfout",	ch->pcdata->bamfOut_,	fread_string_noheap( fp ) );
					KEY( "Bestowments", ch->pcdata->bestowments_, fread_string_noheap( fp ) );
					KEY( "Bio",		ch->pcdata->bio,	fread_string( fp ) );
					KEY( "BankGold",    ch->pcdata->bank_gold,	fread_number( fp ) );
					KEY( "BaseMaxHp", ch->BaseMaxHp, fread_number(fp));
					KEY( "BaseMaxMana", ch->BaseMaxMana, fread_number(fp));
					KEY( "BaseMaxMove", ch->BaseMaxMove, fread_number(fp));
				}
				break;

			case 'C':
				if (ch->pcdata)
				{
					int x;
					if ( !str_cmp( word, "Colors" ) )
					{
						char c;
						/*
						   KSILYAN
						   have to fake the new color addition... moo
						   it needs to set it to blue if it doesn't exist otherwise we'll fread_number the next line of text which is no good
						 */
						for ( x = 0; x < max_colors - 1; x++ )
							ch->pcdata->pc_colors[x] = fread_number( fp );

						c = getc (fp);

						if ( (c == ' ') || (c != '\n') )
						{
							ungetc(c, fp);
							ch->pcdata->pc_colors[max_colors-1] = fread_number(fp);
						}
						else
						{
							ungetc(c, fp);
							sprintf(buf, "Adding night color for %s", ch->getName().c_str());
							log_string(buf);
							ch->pcdata->pc_colors[max_colors - 1] = AT_BLUE;
						}

						fMatch = TRUE;
						break;
					}

					if ( !str_cmp( word, "Clan" ) )
					{
						ch->pcdata->clanName_ = fread_string_noheap( fp );

						if ( !preload
								&&   ch->pcdata->clanName_.length() > 0
								&& ( ch->pcdata->clan = get_clan( ch->pcdata->clanName_.c_str() )) == NULL )
						{
							sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
									ch->pcdata->clanName_.c_str() );
							send_to_char( buf, ch );
							ch->pcdata->clanName_ = "";
						}
						fMatch = TRUE;
						break;
					}

					if ( !str_cmp( word, "Condition" ) )
					{
						line = fread_line( fp );
						sscanf( line, "%d %d %d %d",
								&x1, &x2, &x3, &x4 );
						ch->pcdata->condition[0] = x1;
						ch->pcdata->condition[1] = x2;
						ch->pcdata->condition[2] = x3;
						ch->pcdata->condition[3] = x4;
						fMatch = TRUE;
						break;
					}

					if ( !str_cmp( word, "Council" ) )
					{
						ch->pcdata->councilName_ = fread_string_noheap( fp );
						if ( !preload
								&&   ch->pcdata->councilName_.length() > 0
								&& ( ch->pcdata->council = get_council( ch->pcdata->councilName_.c_str() )) == NULL )
						{
							sprintf( buf, "Warning: the council %s no longer exists, and herefore you no longer\n\rbelong to a council.\n\r",
									ch->pcdata->councilName_.c_str() );
							send_to_char( buf, ch );
							ch->pcdata->councilName_ = "";
						}
						fMatch = TRUE;
						break;
					}
				}

				KEY( "Class",	ch->Class,		fread_number( fp ) );
				break;

			case 'D':
				if ( ch->pcdata ) {
					KEY( "DGlory",       ch->pcdata->quest_deci, fread_number( fp ) );
				}
				KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
				KEY( "Deaf",	ch->deaf,		fread_number( fp ) );

				if ( !str_cmp( word, "Deity" ) && ch->pcdata )
				{
					ch->pcdata->deityName_ = fread_string_noheap( fp );

					if ( !preload
							&&   ch->pcdata->deityName_.length() > 0
							&& ( ch->pcdata->deity = get_deity( ch->pcdata->deityName_.c_str() )) == NULL )
					{
						sprintf( buf, "Warning: the deity %s no longer exists.\n\r",
								ch->pcdata->deityName_.c_str() );
						send_to_char( buf, ch );
						ch->pcdata->deityName_ = "";
						ch->pcdata->favor = 0;
					}
					fMatch = TRUE;
					break;
				}

				KEY( "Description",	ch->description_,	fread_string_noheap( fp ) );
				break;

				/* 'E' was moved to after 'S' */
			case 'F':
				if ( ch->pcdata ) {
					KEY( "Favor",	ch->pcdata->favor,	fread_number( fp ) );
					KEY( "Flags",	ch->pcdata->flags,	fread_number( fp ) );
				}
				break;

			case 'G':
				if ( ch->pcdata )  {
					KEY( "Glory",       ch->pcdata->quest_curr, fread_number( fp ) );
				}

				KEY( "Gold",	ch->gold,		fread_number( fp ) );

				/* temporary measure */
				if ( !str_cmp( word, "Guild" ) && ch->pcdata)
				{
					ch->pcdata->clanName_ = fread_string_noheap( fp );

					if ( !preload
							&&   ch->pcdata->clanName_.length() > 0
							&& ( ch->pcdata->clan = get_clan( ch->pcdata->clanName_.c_str() )) == NULL )
					{
						sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
								ch->pcdata->clanName_.c_str() );
						send_to_char( buf, ch );
						ch->pcdata->clanName_ = "";
					}
					fMatch = TRUE;
					break;
				}
				break;

			case 'H':
				if ( !str_cmp(word, "Helled") && ch->pcdata )
				{
					ch->pcdata->secReleaseDate = fread_number(fp);
					ch->pcdata->helledBy_ = fread_string_noheap(fp);

					if ( ch->pcdata->secReleaseDate < secCurrentTime)
					{
						ch->pcdata->helledBy_ = "";
						ch->pcdata->secReleaseDate = 0;
					}
					fMatch = TRUE;
					break;
				}

				KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );

				if ( ch->pcdata )
					KEY( "Homepage",	ch->pcdata->homepage,	fread_string_nohash( fp ) );

				if ( !str_cmp( word, "HpManaMove" ) )
				{
					ch->hit		= fread_number( fp );
					ch->max_hit	= fread_number( fp );
					ch->mana	= fread_number( fp );
					ch->max_mana	= fread_number( fp );
					ch->move	= fread_number( fp );
					ch->max_move	= fread_number( fp );
					fMatch = TRUE;
					break;
				}
				break;

			case 'I':
				if ( ch->pcdata )
					KEY( "IllegalPK",	ch->pcdata->illegal_pk,	fread_number( fp ) );
				KEY( "Immune",	ch->immune,		fread_number( fp ) );
				break;

			case 'K':
				if ( !str_cmp( word, "Killed" ) && ch->pcdata )
				{
					fMatch = TRUE;
					if ( killcnt >= MAX_KILLTRACK )
						bug( "fread_char: killcnt (%d) >= MAX_KILLTRACK", killcnt );
					else
					{
						ch->pcdata->killed[killcnt].vnum    = fread_number( fp );
						ch->pcdata->killed[killcnt++].count = fread_number( fp );
					}
				}
				break;

			case 'L':
				KEY( "Level",	ch->level,		fread_number( fp ) );
				KEY( "LongDescr",	ch->longDesc_,		fread_string_noheap( fp ) );
				if ( !str_cmp( word, "Languages" ) )
				{
					ch->speaks = fread_number( fp );
					ch->speaking = fread_number( fp );
					fMatch = TRUE;
				}
				break;

			case 'M':
				KEY( "Mentalstate", ch->mental_state,	fread_number( fp ) );
				KEY( "Mobinvis",	ch->mobinvis,		fread_number( fp ) );

				if ( ch->pcdata ) {
					KEY( "MGlory",      ch->pcdata->quest_accum,fread_number( fp ) );
					KEY( "MDeaths",	ch->pcdata->mdeaths,	fread_number( fp ) );
					KEY( "Minsnoop",	ch->pcdata->min_snoop,	fread_number( fp ) );
					KEY( "MKills",	ch->pcdata->mkills,	fread_number( fp ) );

					if ( !str_cmp( word, "MobRange" ) )
					{
						ch->pcdata->m_range_lo = fread_number( fp );
						ch->pcdata->m_range_hi = fread_number( fp );
						fMatch = TRUE;
					}
				}

				KEY("MaxColors", max_colors, fread_number(fp));

				break;

			case 'N':
				if ( !str_cmp( word, "Name" ) )
				{
					/*
					 * Name already set externally.
					 */
					fread_to_eol( fp );
					fMatch = TRUE;
					break;
				}
				break;

			case 'O':
				if ( ch->pcdata )
				{
					KEY( "Outcast_time", ch->pcdata->secOutcastTime, fread_number( fp ) );
					if ( !str_cmp( word, "ObjRange" ) )
					{
						ch->pcdata->o_range_lo = fread_number( fp );
						ch->pcdata->o_range_hi = fread_number( fp );
						fMatch = TRUE;
					}
				}
				break;

			case 'P':
				if ( ch->pcdata )
				{
					KEY( "Pagerlen",	ch->pcdata->pagerlen,	fread_number( fp ) );
					KEY( "Password",	ch->pcdata->pwd,	fread_string_nohash( fp ) );
					KEY( "PDeaths",	ch->pcdata->pdeaths,	fread_number( fp ) );
					KEY( "PKills",	ch->pcdata->pkills,	fread_number( fp ) );
					KEY( "Prompt",	ch->pcdata->prompt_,	fread_string_noheap( fp ) );
					KEY( "Prompt2",	ch->pcdata->prompt2_,	fread_string_noheap( fp ) );
				}

				KEY( "Played",	ch->played,		fread_number( fp ) );
				KEY( "Position",	ch->position,		fread_number( fp ) );
				KEY( "Practice",	ch->practice,		fread_number( fp ) );
				if (!str_cmp ( word, "PTimer" ) )
				{
					/* depreciated */
					fMatch = TRUE;
					break;
				}

				/*
				 * KSILYAN
				 */
				if (!str_cmp ( word, "PlayerThief" ) )
				{
					int Number;
					int c;
					char * buffer;

					if (file_ver <= 4)
					{
						buffer = fread_line(fp);

						for (uint x = 0; x <= strlen(buffer);x++)
						{
							c = buffer[x];
							if ( isdigit(c) )
							{
								for (uint y = 0; y < MAX_CITIES; y++)
								{
									if (x + (y*2) > strlen(buffer) )
										break;
									ch->pcdata->citythief[y] = buffer[x+(y*2)];
								}
								break;
							}
						}
					}
					else
					{
						Number = fread_number(fp);
						for (int x=0; x < Number; x++)
							ch->pcdata->CityWanted[x] = fread_number(fp);
					}

					fMatch = TRUE;
					break;
				}
				if ( !str_cmp(word, "PlayerWanted") )
				{
					int Number;
					int x;

					Number = fread_number(fp);
					for (x = 0; x < Number; x++)
					{
						ch->pcdata->CityWanted[x] = fread_number(fp);
					}

					fMatch = TRUE;
					break;
				}

				break;

			case 'R':
				KEY( "Race",        ch->race,		fread_number( fp ) );
				KEY( "Resistant",	ch->resistant,		fread_number( fp ) );
				if ( ch->pcdata )
				{
					KEY( "RecallRoom",  ch->pcdata->recall_room,fread_number( fp ) );
					KEY( "Rank",        ch->pcdata->rank_,	fread_string_noheap( fp ) );
					KEY( "Restore_time",ch->pcdata->secRestoreTime, fread_number( fp ) );
					KEY( "Rating",      ch->pcdata->rating, fread_number( fp ) );

					if ( !str_cmp( word, "RoomRange" ) )
					{
						ch->pcdata->r_range_lo = fread_number( fp );
						ch->pcdata->r_range_hi = fread_number( fp );
						fMatch = TRUE;
					}
				}

				if ( !str_cmp( word, "Room" ) )
				{
					Room * room = get_room_index( fread_number( fp ) );
					if ( room )
						ch->InRoomId = room->GetId();
					else
						ch->InRoomId = get_room_index( ROOM_VNUM_LIMBO )->GetId();
					fMatch = TRUE;
					break;
				}

				break;

			case 'S':
				KEY( "Sex",		ch->sex,		fread_number( fp ) );
				KEY( "Susceptible",	ch->susceptible,	fread_number( fp ) );

				if ( !str_cmp( word, "ShortDescr" ) )
				{
					ch->setShort( fread_string_noheap(fp) );
					fMatch = true;
					break;
				}

				if ( !str_cmp( word, "Scent" ) )
				{
					ch->setScent( fread_number(fp) );
					fMatch = true;
					break;
				}

				if ( !str_cmp( word, "SavingThrow" ) )
				{
					ch->saving_wand 	= fread_number( fp );
					ch->saving_poison_death = ch->saving_wand;
					ch->saving_para_petri 	= ch->saving_wand;
					ch->saving_breath 	= ch->saving_wand;
					ch->saving_spell_staff 	= ch->saving_wand;
					fMatch = TRUE;
					break;
				}

				if ( !str_cmp( word, "SavingThrows" ) )
				{
					ch->saving_poison_death = fread_number( fp );
					ch->saving_wand 	= fread_number( fp );
					ch->saving_para_petri 	= fread_number( fp );
					ch->saving_breath 	= fread_number( fp );
					ch->saving_spell_staff 	= fread_number( fp );
					fMatch = TRUE;
					break;
				}

				if ( !str_cmp( word, "Site" ) )
				{
					if ( !preload && !fCopyOver )
					{
						sprintf( buf, "Last connected from: %s\n\r", fread_word( fp ) );
						send_to_char( buf, ch );
					}
					else
						fread_to_eol( fp );

					fMatch = TRUE;

					if ( preload )
						word = "End";
					else
						break;
				}

				if ( !str_cmp( word, "Skill" ) && ch->pcdata )
				{
					int sn;
					int value;

					if ( preload )
						word = "End";
					else
					{
						value = fread_number( fp );

						sname = fread_word( fp );
						if ( file_ver < 3 )
							sn = skill_lookup( sname );
						else
							sn = bsearch_skill_exact( sname, gsn_first_skill, gsn_first_weapon-1 );

						if ( sn < 0 )
						{
							sprintf(sbuf, "Fread_char: unknown skill '%s'.", sname );
							bug(sbuf,0);
						}
						else
						{
							ch->pcdata->learned[sn] = value;
							/* Take care of people who have stuff they shouldn't     *
							 * Assumes class and level were loaded before. -- Altrag *
							 * Assumes practices are loaded first too now. -- Altrag */

							if ( ch->level < LEVEL_IMMORTAL )
							{
								if ( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
								{
#if 1 /* Testaur - allow forbidden skills */
									sprintf(buf,"fread_char: player %s has skill '%s'",
											NAME(ch),skill_table[sn]->name_.c_str());
									//log_string(buf);
									if (!strcmp(skill_table[sn]->name_.c_str(),"sneak") )
									{
										sprintf(buf,"fread_char: removing sneak from player");
										log_string(buf);
										ch->pcdata->learned[sn]=0;
									}
#else
									ch->pcdata->learned[sn] = 0;
									ch->practice++;
#endif
								}
							}

						}

						fMatch = TRUE;
						break;
					}
				}

				if ( !str_cmp( word, "Spell" ) && ch->pcdata )
				{
					int sn;
					int value;

					if ( preload )
						word = "End";
					else
					{
						value = fread_number( fp );
						sname = fread_word( fp );
						sn = bsearch_skill_exact( sname, gsn_first_spell, gsn_first_skill-1 );

						if ( sn < 0 )
						{
							sprintf(sbuf, "Fread_char: unknown spell '%s'.", sname );
							bug(sbuf,0);
						}
						else
						{
							ch->pcdata->learned[sn] = value;

							if ( ch->level < LEVEL_IMMORTAL )
							{
								if ( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
								{
#if 1 /* Testaur - allow forbidden spells (good for imm debugging!) */
									sprintf(buf,"fread_char: player %s has spell '%s'",
											NAME(ch),skill_table[sn]->name_.c_str());
									//log_string(buf);
#else
									ch->pcdata->learned[sn] = 0;
									ch->practice++;
#endif
								}
							}

						}
						fMatch = TRUE;
						break;
					}
				}
				if ( str_cmp( word, "End" ) )
					break;

			case 'E':
				if ( ch->pcdata )
					KEY("Email", ch->pcdata->email, fread_string_nohash(fp));

				if ( !str_cmp( word, "EntryMessage" ) )
				{
					string msg = fread_string_noheap(fp);
					ch->setMovementMessage(msg);
					fMatch = true;
					break;
				}

				if ( !str_cmp( word, "End" ) )
				{

					if ( ch->pcdata )
					{
						if (!ch->pcdata->pwd)         ch->pcdata->pwd       = str_dup ("");
						if (!ch->pcdata->bio)         ch->pcdata->bio       = STRALLOC("");
						if (!ch->pcdata->homepage)    ch->pcdata->homepage  = str_dup ("");

						killcnt = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_STONE_MASTER, MAX_KILLTRACK );

						if ( killcnt < MAX_KILLTRACK )
							ch->pcdata->killed[killcnt].vnum = 0;
					}

					ch->editor		= NULL;

					if ( file_ver < 6 )
						ch->played = 5 * ch->level * 7200;


					/* no good for newbies at all */
					if ( !IS_IMMORTAL( ch ) && !ch->speaking )
						ch->speaking = LANG_COMMON;

					if ( IS_IMMORTAL( ch ) )
					{
						ch->speaks = ~0;
						if ( ch->speaking == 0 )
							ch->speaking = ~0;
					}

					return;
				}
				KEY( "Exp",		ch->exp,		fread_number( fp ) );
				KEY( "Exitdescr",   ch->exitDesc_,         fread_string_noheap( fp ) );
				KEY( "Enterdescr",  ch->enterDesc_,        fread_string_noheap( fp ) );
				KEY( "Ethos",       ch->pcdata->ethos,      fread_number( fp ) );

				break;

			case 'T':
				if ( !str_cmp( word, "Tongue" ) && ch->pcdata )
				{
					int sn;
					int value;

					if ( preload )
						word = "End";
					else
					{
						value = fread_number( fp );
						sname = fread_word( fp );

						sn = bsearch_skill_exact( sname , gsn_first_tongue, gsn_top_sn-1 );

						if ( sn < 0 )
						{
							sprintf( sbuf,"Fread_char: unknown tongue '%s'.", sname );
							bug(sbuf, 0);
						}
						else
						{
							ch->pcdata->learned[sn] = value;
							if ( ch->level < LEVEL_IMMORTAL )
								if ( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
								{
									ch->pcdata->learned[sn] = 0;
									ch->practice++;
								}
						}
						fMatch = TRUE;
					}
					break;
				}
				KEY( "Trust", ch->trust, fread_number( fp ) );

				if ( !str_cmp( word, "Title" ) && ch->pcdata )
				{

					ch->pcdata->title_ = fread_string_noheap( fp );
					fMatch = TRUE;
	            	if(( isalpha(ch->pcdata->title_.c_str()[0]) || isdigit(ch->pcdata->title_.c_str()[0])) && ch->pcdata->title_.c_str()[1]!=':' )
	            	{
	               		sprintf( sbuf, " %s", ch->pcdata->title_.c_str() );
		           		ch->pcdata->title_ = sbuf;
	                }
					break;
				}

				break;

			case 'V':
				if ( !str_cmp( word, "Vnum" ) )
				{
					ch->pIndexData = get_mob_index( fread_number( fp ) );
					fMatch = TRUE;
					break;
				}
				KEY( "Version",	file_ver,		fread_number( fp ) );
				break;

			case 'W':
				if ( !str_cmp( word, "Weapon" ) && ch->pcdata )
				{
					int sn;
					int value;

					if ( preload )
						word = "End";
					else
					{
						value = fread_number( fp );
						sname = fread_word( fp );

						sn = bsearch_skill_exact( sname, gsn_first_weapon, gsn_first_tongue-1 );

						if ( sn < 0 )
						{
							sprintf(sbuf, "Fread_char: unknown weapon '%s'.", sname );
							bug(sbuf,0);
						}
						else
						{
							ch->pcdata->learned[sn] = value;

							if ( ch->level < LEVEL_IMMORTAL )
							{
								if ( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
								{
									ch->pcdata->learned[sn] = 0;
									ch->practice++;
								}
							}
						}
						fMatch = TRUE;
					}
					break;
				}
				KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
				if ( ch->pcdata )
					KEY( "WizInvis",	ch->pcdata->wizinvis,	fread_number( fp ) );
				break;
		}

		// KSILYAN: Make checks for player flags
		for (i = 0; i < MAX_CITIES; i++) {
			if ( ( ch->pcdata->citythief[i] != 0 ) && ( ch->pcdata->citythief[i] != 1 ) ) {
				ch->pcdata->citythief[i] = 0;
			}
		}

		if ( !fMatch )
		{
			sprintf( buf, "Fread_char: no match: %s", word );
			bug( buf, 0 );
		}
	}
}


void fread_obj( CHAR_DATA *ch, FILE *fp, sh_int os_type )
{
	char sbuf[MAX_INPUT_LENGTH];
	char *sname;

	Object *obj;
	const char *word;
	int iNest;
	bool fMatch;
	bool fNest;
	bool fVnum;
	ROOM_INDEX_DATA *room;

	obj = new Object();
	obj->count		= 1;
	obj->wear_loc	= -1;
	obj->weight 	= 1;

	fNest		= TRUE; 	/* Requiring a Nest 0 is a waste */
	fVnum		= TRUE;
	iNest		= 0;

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
				KEY("ActionDesc", obj->actionDesc_, fread_string_noheap(fp));
				if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
				{
					AFFECT_DATA *paf;
					int pafmod;

					CREATE( paf, AFFECT_DATA, 1 );
					if ( !str_cmp( word, "Affect" ) )
					{
						paf->type	= fread_number( fp );
					}
					else
					{
						int sn;
						sname=fread_word( fp );
						sn = skill_lookup( sname );
						if ( sn < 0 )
						{
							sprintf(sbuf, "Fread_obj: unknown skill '%s'.", sname );
							bug(sbuf, 0);
						}
						else
							paf->type = sn;
					}
					paf->duration	= fread_number( fp );
					pafmod		= fread_number( fp );
					paf->location	= fread_number( fp );
					paf->bitvector	= fread_number( fp );
					if ( paf->location == APPLY_WEAPONSPELL
						||	 paf->location == APPLY_WEARSPELL
						||	 paf->location == APPLY_REMOVESPELL )
						paf->modifier		= slot_lookup( pafmod );
					else
						paf->modifier		= pafmod;
					LINK(paf, obj->first_affect, obj->last_affect, next, prev );
					fMatch				= TRUE;
					break;
				}
				break;

			case 'C':
				KEY( "Cost",	obj->cost,		fread_number( fp ) );
				KEY( "Count",	obj->count, 	fread_number( fp ) );
				KEY( "Condition", obj->condition, fread_number( fp ) );
				break;

			case 'D':
				KEY( "Description", obj->longDesc_,	fread_string_noheap( fp ) );
				break;

			case 'E':
				KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
				KEY( "ExtraFlags2", obj->extra_flags_2, fread_number( fp ) );

				if ( !str_cmp( word, "ExtraDescr" ) )
				{
					ExtraDescData *ed;

					ed = new ExtraDescData;
					ed->keyword_ 	= fread_string_noheap( fp );
					ed->description_ 	= fread_string_noheap( fp );
					LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );
					fMatch				= TRUE;
				}

				if ( !str_cmp( word, "End" ) )
				{
					if ( !fNest || !fVnum )
					{
						bug( "Fread_obj: incomplete object.", 0 );
						delete obj;
						return;
					}
					else
					{
						sh_int wear_loc = obj->wear_loc;

						if ( obj->name_.length() == 0 )
							obj->name_ = obj->pIndexData->name_;
						if ( obj->longDesc_.length() == 0 )
							obj->longDesc_ = obj->pIndexData->longDesc_;
						if ( obj->shortDesc_.length() == 0 )
							obj->shortDesc_ = obj->pIndexData->shortDesc_;
						if ( obj->actionDesc_.length() == 0 )
							obj->actionDesc_ = obj->pIndexData->actionDesc_;

						// Ksilyan:
						if (obj->max_condition == 0)
						{
							obj->max_condition = obj->condition = obj->pIndexData->max_condition;
						}

						// Aiyan
						if ( obj->item_type == ITEM_WEAPON )
						{
							obj->value[OBJECT_WEAPON_DAMAGEMESSAGE] = weapontype_to_damagemsg(obj->value[OBJECT_WEAPON_WEAPONTYPE]);
						}

						if (obj->wear_loc == WEAR_SHEATHED_1 || obj->wear_loc == WEAR_SHEATHED_2)
						{
							// remove sheathed weapons, just in case they were saved there
							// before the new scabbard format appeared
							obj->wear_loc = -1;
							wear_loc = -1; // update wear_loc (set above)
						}

						LINK(obj, first_object, last_object, next, prev );
						if ( os_type != OS_CARRY_NOINC ) {
							obj->pIndexData->total_count += obj->count;
						}
						obj->pIndexData->count += obj->count;
						if ( !obj->serial )
						{
							cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
							obj->serial = obj->pIndexData->serial = cur_obj_serial;
						}
						if ( fNest )
							rgObjNest[iNest] = obj;
						numobjsloaded += obj->count;
						++physicalobjects;
						if ( file_ver > 1 || obj->wear_loc < -1
							||	 obj->wear_loc >= MAX_WEAR )
							obj->wear_loc = -1;
						/* Corpse saving. -- Altrag */
						if ( os_type == OS_CORPSE )
						{
							if ( !room )
							{
								bug( "Fread_obj: Corpse without room", 0);
								room = get_room_index(ROOM_VNUM_LIMBO);
							}
							obj = obj_to_room( obj, room );
						}
						else if ( iNest == 0 || rgObjNest[iNest] == NULL )
						{
							int slot;
							bool reslot = FALSE;

							if ( file_ver > 1
								&&	 wear_loc > -1
								&&	 wear_loc < MAX_WEAR )
							{
								int x;

								for ( x = 0; x < MAX_LAYERS; x++ )
									if ( !save_equipment[wear_loc][x] )
									{
										save_equipment[wear_loc][x] = obj;
										slot = x;
										reslot = TRUE;
										break;
									}
									if ( x == MAX_LAYERS )
										bug( "Fread_obj: too many layers %d", wear_loc );
							}
							obj = obj_to_char( obj, ch );

							if ( reslot )
								save_equipment[wear_loc][slot] = obj;
						}
						else
						{
							if ( rgObjNest[iNest-1] )
							{
								separate_obj( rgObjNest[iNest-1] );
								obj = obj_to_obj( obj, rgObjNest[iNest-1] );
							}
							else
								bug( "Fread_obj: nest layer missing %d", iNest-1 );
						}
						if ( fNest )
							rgObjNest[iNest] = obj;

						obj->skryptSetContainer(obj->pIndexData);
						return;
					}
				}
				break;

			case 'H':
				KEY( "ItemType",	obj->item_type, 	fread_number( fp ) );

#define HOLDER_KEY(text, destination) \
	if ( !str_cmp(word, (text)) ) \
	{ \
		char * buf; \
		buf = fread_string(fp); \
		if ( str_cmp(buf, "0") ) \
			destination.assign(buf); \
		STRFREE(buf); \
		fMatch = true; \
		break; \
	}

				HOLDER_KEY("Holder0Name", obj->Holders[0]->Name);
				HOLDER_KEY("Holder0Host", obj->Holders[0]->Host);
				HOLDER_KEY("Holder0Time", obj->Holders[0]->Time);
				HOLDER_KEY("Holder0Account", obj->Holders[0]->Account);

				HOLDER_KEY( "Holder1Name", obj->Holders[1]->Name);
				HOLDER_KEY( "Holder1Host", obj->Holders[1]->Host);
				HOLDER_KEY( "Holder1Time", obj->Holders[1]->Time);
				HOLDER_KEY( "Holder1Account", obj->Holders[1]->Account);

				break;

			case 'I':
				KEY( "ItemType",	obj->item_type, 	fread_number( fp ) );
				break;

			case 'L':
				{
					int tmp;
#ifdef USE_OBJECT_LEVELS
					KEY( "Level",	obj->level, 	fread_number( fp ) );
#else
					KEY("Level", tmp, fread_number(fp));
#endif
				}
				break;

			case 'M':
				KEY( "MaxCondition", obj->max_condition, fread_number( fp ) );
				break;

			case 'N':
				KEY( "Name",	obj->name_,		fread_string_noheap( fp ) );

				if ( !str_cmp( word, "Nest" ) )
				{
					iNest = fread_number( fp );
					if ( iNest < 0 || iNest >= MAX_NEST )
					{
						bug( "Fread_obj: bad nest %d.", iNest );
						iNest = 0;
						fNest = FALSE;
					}
					fMatch = TRUE;
				}
				break;

			/* matthew */
			case 'O':
				if ( !str_cmp( word, "ObjNPC_rvnum" ) )
				{
					if ( !obj->pObjNPC ) {
						CREATE(obj->pObjNPC, OBJ_NPC_DATA, 1);
					}

					obj->pObjNPC->r_vnum = fread_number(fp);
					fMatch = TRUE;
					break;
				}
				if ( !str_cmp( word, "ObjNPC_mvnum" ) )
				{
					if ( !obj->pObjNPC ) {
						CREATE(obj->pObjNPC, OBJ_NPC_DATA, 1);
					}

					obj->pObjNPC->m_vnum = fread_number(fp);
					fMatch = TRUE;
					break;
				}

			case 'R':
				KEY( "Room", room, get_room_index(fread_number(fp)) );

			case 'S':
				KEY( "ShortDescr",	obj->shortDesc_,	fread_string_noheap( fp ) );

				if ( !str_cmp( word, "Spell" ) )
				{
					int iValue;
					int sn;

					iValue = fread_number( fp );
					sname = fread_word( fp );
					sn	   = skill_lookup( sname );
					if ( iValue < 0 || iValue > 5 )
						bug( "Fread_obj: bad iValue %d.", iValue );
					else if ( sn < 0 )
					{
						sprintf(sbuf, "Fread_obj: unknown skill '%s'.", sname);
						bug(sbuf, 0);
					}
					else
						obj->value[iValue] = sn;
					fMatch = TRUE;
					break;
				}

				break;

			case 'T':
				KEY( "Timer",	obj->timer, 	fread_number( fp ) );
				break;

			case 'V':
				if ( !str_cmp( word, "Values" ) )
				{
					int x1,x2,x3,x4,x5,x6;
					bool sameshort, samelong;
					char *ln = fread_line( fp );

					sameshort = samelong = FALSE;

					x1=x2=x3=x4=x5=x6=0;
					sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
					/* clean up some garbage */
					if ( file_ver < 3 )
						x5=x6=0;

					obj->value[0]	= x1;
					obj->value[1]	= x2;
					obj->value[2]	= x3;
					obj->value[3]	= x4;
					obj->value[4]	= x5;
					obj->value[5]	= x6;
					/* Ksilyan: this doesn't work very well
					* for storerooms, so we're taking it out
					* and seeing what happens!
					*
					if (file_ver <= 3 )
					{
						switch (obj->item_type)
						{
							case ITEM_WEAPON:
								obj->condition = x1;
								obj->max_condition = obj->pIndexData->max_condition;
								break;
							case ITEM_ARMOR:
								obj->condition = x1;
								obj->max_condition = x2;
								break;
							case ITEM_CONTAINER:
								obj->condition = x4;
								obj->max_condition = obj->pIndexData->max_condition;
								break;
						}
					}
					else
					{
						if (obj->max_condition == 0)
							obj->max_condition = obj->condition;
					}*/

					/* KSILYAN- Keep up to date with prototypes
					 *	Compare item values with prototype's values. If they don't check,
					 *	and the item is not glorified (restring), then get new values.
					 */
					samelong = sameshort = false;
					if ( obj->pIndexData )
					{
						if ( obj->longDesc_ == obj->pIndexData->longDesc_ )
							samelong = TRUE;
						else
							samelong = FALSE;
					}
					else
						samelong = TRUE;

					if (obj->shortDesc_.length() > 0 && obj->pIndexData)
					{
						if ( obj->shortDesc_ != obj->pIndexData->shortDesc_ )
							sameshort = TRUE;
						else
							sameshort = FALSE;
					}
					else
						sameshort = TRUE;

					if (sameshort && samelong)
					{
						bool foundone;
						char buf[MAX_INPUT_LENGTH];
						int i;
						int oldvalue[6];

						switch(obj->item_type)
						{
							default:
								break;
							case ITEM_WEAPON:
							case ITEM_ARMOR:
							case ITEM_CONTAINER:
							case ITEM_QUIVER:
							case ITEM_PROJECTILE:
								obj->wear_flags = obj->pIndexData->wear_flags;
								foundone = FALSE;
								for (i = 0; i < 6; i++)
								{
									if (ITEM_CONTAINER && i == 1)
										continue;
									oldvalue[i] = obj->value[i];
									if (obj->value[i] != obj->pIndexData->value[i])
									{
										foundone = TRUE;
										obj->value[i] = obj->pIndexData->value[i];
									}
								}
								if (foundone)
								{
									if (ch && (ch != supermob) )
									{
										obj->max_condition = obj->pIndexData->max_condition;
										obj->item_type = obj->pIndexData->item_type;
										sprintf(buf, "OBJECT UPDATE: Player %s had an old item: %s. Updating... (%d %d %d %d %d %d) to (%d %d %d %d %d %d)",
											ch->getName().c_str(), obj->pIndexData->shortDesc_.c_str(),
											oldvalue[0],oldvalue[1],oldvalue[2],
											oldvalue[3],oldvalue[4],oldvalue[5],
											obj->pIndexData->value[0],
											obj->pIndexData->value[1],
											obj->pIndexData->value[2],
											obj->pIndexData->value[3],
											obj->pIndexData->value[4],
											obj->pIndexData->value[5]);
										log_string(buf);
									}
								}
								break;
						}
					}

					fMatch		= TRUE;
					break;
				}

				if ( !str_cmp( word, "Vnum" ) )
				{
					int vnum;

					vnum = fread_number( fp );
					/*	bug( "Fread_obj: bad vnum %d.", vnum );  */
					if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
						fVnum = FALSE;
					else
					{
						fVnum = TRUE;
						obj->cost = obj->pIndexData->cost;
						obj->weight = obj->pIndexData->weight;
						obj->item_type = obj->pIndexData->item_type;
						obj->wear_flags = obj->pIndexData->wear_flags;
						obj->extra_flags = obj->pIndexData->extra_flags;
						obj->extra_flags_2 = obj->pIndexData->extra_flags_2;
						obj->max_condition = obj->pIndexData->max_condition;

					}
					fMatch = TRUE;
					break;
				}
				break;

			case 'W':
				KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
				KEY( "WearLoc", obj->wear_loc,		fread_number( fp ) );
				KEY( "Weight",	obj->weight,		fread_number( fp ) );
				break;
		} // end of switch

		if ( !fMatch )
		{
			ExtraDescData *ed;
			AFFECT_DATA *paf;

			bug( "Fread_obj: no match.", 0 );
			bug( word, 0 );
			fread_to_eol( fp );
			while ( (ed=obj->first_extradesc) != NULL )
			{
				UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
				delete ed;
			}
			while ( (paf=obj->first_affect) != NULL )
			{
				UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
				DISPOSE( paf );
			}
			delete obj;
			return;
		}
	} // end of for loop
}

void set_alarm( long seconds )
{
	alarm( seconds );
}

#if 0
/* moved to last.c */
/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last(CHAR_DATA *ch, const char* argument)
{
    char buf [MAX_STRING_LENGTH];
    char arg [MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct stat fst;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Usage: last <playername>\n\r", ch );
	return;
    }
    strcpy( name, capitalize(arg) );
    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name );
    if ( stat( buf, &fst ) != -1 )
      sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
    else
      sprintf( buf, "%s was not found.\n\r", name );
   send_to_char( buf, ch );
}
#endif

void write_corpses( CHAR_DATA *ch, const char *name )
{
	OBJ_DATA *corpse;
	FILE *fp = NULL;

	/* Name and ch support so that we dont have to have a char to save their
	   corpses.. (ie: decayed corpses while offline) */
	if ( ch && IS_NPC(ch) )
	{
		/*    bug( "Write_corpses: writing NPC corpse.", 0 ); */
		return;
	}
	if ( ch )
		name = ch->getName().c_str();
	/* Go by vnum, less chance of screwups. -- Altrag */
	for ( corpse = first_object; corpse; corpse = corpse->next )
		if ( corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
				corpse->GetInRoom() &&
				!str_cmp(corpse->shortDesc_.c_str() +14, name) )
		{
			if ( !fp )
			{
				char buf[127];

				sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
				if ( !(fp = fopen(buf, "w")) )
				{
					bug( "Write_corpses: Cannot open file.", 0 );
					perror(buf);
					return;
				}
			}
			fwrite_obj(ch, corpse, fp, 0, OS_CORPSE);
		}
	if ( fp )
	{
		fprintf(fp, "#END\n\n");
		fclose(fp);
	}
	else
	{
		char buf[127];

		sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
		remove(buf);
	}
	return;
}

void load_corpses( void )
{
  DIR *dp;
  struct dirent *de;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  extern int falling;

  if ( !(dp = opendir(CORPSE_DIR)) )
  {
    bug( "Load_corpses: can't open CORPSE_DIR", 0);
    perror(CORPSE_DIR);
    return;
  }

  falling = 1; /* Arbitrary, must be >0 though. */
  while ( (de = readdir(dp)) != NULL )
  {
    if ( de->d_name[0] != '.' )
    {
      sprintf(strArea, "%s%s", CORPSE_DIR, de->d_name );
      fprintf(stderr, "Corpse -> %s\n", strArea);
      if ( !(fpArea = fopen(strArea, "r")) )
      {
        perror(strArea);
        continue;
      }
      for ( ; ; )
      {
        char letter;
        char *word;

        letter = fread_letter( fpArea );
        if ( letter == '*' )
        {
          fread_to_eol(fpArea);
          continue;
        }
        if ( letter != '#' )
        {
          bug( "Load_corpses: # not found.", 0 );
          break;
        }
        word = fread_word( fpArea );
        if ( !str_cmp(word, "CORPSE" ) )
          fread_obj( NULL, fpArea, OS_CORPSE );
        else if ( !str_cmp(word, "OBJECT" ) )
          fread_obj( NULL, fpArea, OS_CARRY );
        else if ( !str_cmp( word, "END" ) )
          break;
        else
        {
          bug( "Load_corpses: bad section.", 0 );
          break;
        }
      }
      fclose(fpArea);
    }
  }
  fpArea = NULL;
  strcpy(strArea, "$");
  closedir(dp);
  falling = 0;
  return;
}

