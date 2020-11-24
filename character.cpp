

#include "globals.h"
#include "character.h"
#include "connection.h"
#include "mud.h"
#include "World.h"
#include "commands.h"

#include "stored_objs.h"
#include "utility_objs.hpp"

#include "ScentController.h"
#include "Scent.h"

tPointerOwner<Character> CharacterMap;

mob_index_data::mob_index_data()
{
	next = next_sort = NULL;
	spec_fun = NULL;
	pShop = NULL;
	pStable = NULL;
	train = NULL;
	rShop = NULL;
	mudprogs = NULL;

    progtypes = 0;
    vnum = 0;
    count = killed = sex = level = 0;
    act = affected_by = 0;
    alignment = 0;
    mobthac0 = 0;		/* Unused */
    ac = 0;
    hitnodice = hitsizedice = hitplus = 0;
	damnodice = damsizedice = damplus = numattacks = 0;
    gold = exp = xflags = 0;
    resistant = immune = susceptible = 0;
    attacks = defenses = speaks = speaking = 0;
    position = defposition = height = weight = race = Class = hitroll = damroll = 0;
    perm_str = perm_int = perm_wis = perm_dex = perm_con = perm_cha = perm_lck = 0;
    saving_poison_death = saving_wand = saving_para_petri = saving_breath = saving_spell_staff = 0;
	
	HomeVnum = ScentId = 0;
}


Character::Character()
	: tManagedObject<Character> (CharacterMap)
{
	next = prev = next_in_room = prev_in_room = NULL;
	hunting = fearing = hating = NULL;
	spec_fun = NULL;
	mpact = NULL;
	pIndexData = NULL;
	first_affect = last_affect = NULL;
	pnote = NULL;
	first_carrying = last_carrying = NULL;
	pcdata = NULL;
	last_cmd = prev_cmd = NULL;
	dest_buf = spare_ptr = NULL;
	editor = NULL;
	first_timer = last_timer = NULL;

	inter_page = 0;
	inter_type = 0;
	inter_editing = NULL;
	inter_editing_vnum = -1;
	inter_substate = 0;

	mpactnum = tempnum = mpscriptpos = 0;
	/*num_fighting =*/ substate = sex = race = level = trust = 0;
	Class = 3;
	played = 0;
	secLogonTime = secCurrentTime;
	secSaveTime = 0;
	timer = WaitTime = 0;
	hit = max_hit = BaseMaxHp = 20;
	mana = max_mana = move = max_move = BaseMaxMana = BaseMaxMove = 100;
	practice = numattacks = 0;
	gold = exp = accumulated_exp = act = affected_by =  carry_number = carry_weight = 0;
	xflags = resistant = immune = susceptible = attacks = defenses = 0;
	speaks = speaking = LANG_COMMON;
	saving_poison_death = saving_wand = saving_para_petri = saving_breath = saving_spell_staff = 0;
	alignment = mobthac0 = hitroll = damroll = hitplus = damplus = defposition = wimpy = 0;
	barenumdie = 1;
	baresizedie = 4;
	deaf = 0;
	height = 72;
	weight = 180;
	perm_str = perm_int = perm_dex = perm_wis = perm_con = perm_cha = perm_lck = 13;
	mod_str = mod_int = mod_dex = mod_wis = mod_con = mod_cha = mod_lck = 0;
	mental_state = emotional_state = 0;
	pagelen = 24;
	scentId_ = 0;
	
	retran = 0; regoto = 0; mobinvis = 0; vnum_destination = 0;
	mount_vnum = 0; HomeVnum = 0;
	armor = 100;
	position = POS_STANDING;

	ReadyForPrompt = false;

	spellMemory_.clear();

	BarricadeDir = -1; // none

	movementMsgName_ = "default";
}

Character::~Character()
{
	OBJ_DATA *obj;
	AFFECT_DATA *paf;
	TIMER *timer;
	MPROG_ACT_LIST *mpact, *mpact_next;
	
	if ( !this )
	{
		bug( "Free_char: null ch!" );
		return;
	}
	
	if ( this->ConnectionId != 0 )
		bug( "Free_char: char still has descriptor." );
	
	while ( (obj = this->last_carrying) != NULL )
	{
		extract_obj( obj, FALSE );
	}
	
	while ( (paf = this->last_affect) != NULL )
		affect_remove( this, paf );
	
	while ( (timer = this->first_timer) != NULL )
		extract_timer( this, timer );
	
	if ( this->editor )
		stop_editing( this );
	
	if ( this->inter_editing )
		DISPOSE( this->inter_editing );
	
	stop_hunting( this );
	stop_hating ( this );
	stop_fearing( this );
	//free_fight	( this );
	
	StopAllFights();
	
	if ( this->pnote )
		free_note( this->pnote );
	
	if ( this->pcdata )
	{
		DISPOSE( this->pcdata->pwd	);	/* no hash */
		STRFREE( this->pcdata->bio	); 
		DISPOSE( this->pcdata->homepage	);	/* no hash */
		DISPOSE( this->pcdata->email		);	/* No hash */
		if ( this->pcdata->subprompt )
			STRFREE( this->pcdata->subprompt );
		if ( this->pcdata->Vault )
			DeleteMemoryAllocation(this->pcdata->Vault);
		this->pcdata->Vault = NULL;
		delete this->pcdata;
	}

	for ( mpact = this->mpact; mpact; mpact = mpact_next )
	{
		mpact_next = mpact->next;
		DISPOSE( mpact->buf );
		DISPOSE( mpact		);
	}

	// say bye to whoever's snooping this character
	itorSocketId itor;
	for (itor = SnoopedBy.begin(); itor != SnoopedBy.end(); itor++)
	{
		PlayerConnection * connection = this->GetConnection();

		if (!connection)
			continue;

		string result;
		result.assign("Your victim, " );
		result.append(this->getName().c_str());
		result.append(", has left the game.\n\r");
		connection->SendText( result );
		connection->CancelSnoop(this);
	}
}

void Character::sendText(const char * text, bool color)
{
	//bool hasOutput;

	PlayerConnection * d = this->GetConnection();

	if (!d)
		return; // no point in doing anything if text won't go anywhere
	if (strlen(text) == 0)
		return; // no point in doing anything if there is no text
	

	/*
	 * If we have output at the beginning, and then after sending,
	 * we no longer have output - i.e. we emptied the buffer - 
	 * then send a prompt.
	 */
	//hasOutput = true;

	SnoopBuffer.append(text);

	if (!color)
		d->SendText(text); // if there's no color just forward it on
	else
	{
		const char *colstr;
		const char *prevstr = text;
		char colbuf[20];
		int ln;
		
		while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
		{
			if (colstr > prevstr)
				d->SendText(prevstr, (colstr-prevstr));
			ln = make_color_sequence(colstr, colbuf, d);
			if ( ln < 0 )
			{
				prevstr = colstr+1;
				break;
			}
			else if ( ln > 0 )
				d->SendText(colbuf, ln);
			prevstr = colstr+2;
		}
		if ( *prevstr )
			d->SendText(prevstr);
	}

	// next time the buffer empties, send a prompt
	// (prompts are sent directly to the connection,
	//   so it won't trigger another ReadyForPrompt)
	if ( !d->InCommand )
		ReadyForPrompt = true;
}

/*
 * OutputBufferEmptied
 * Called by connection, when the output
 * buffer is emptied.
 */
void Character::OutputBufferEmptied()
{
	if (ReadyForPrompt)
	{
		ReadyForPrompt = false;
		SendPrompt();
	}

	if (SnoopBuffer.length() > 0)
	{
		// Send text to snoopers.
		this->SendOutputToSnoopers(this->getName().c_str(), SnoopBuffer.c_str());
		SnoopBuffer.assign("");
	}
}

void Character::SendPrompt()
{
	PlayerConnection * d = this->GetConnection();

	if ( !d )
		return;

	if (d && gGameRunning)
	{
		if ( d->ConnectedState == CON_PLAYING )
		{
			d->InPrompt = true;
			if ( IS_SET(this->act, PLR_BLANK) /*&& Connection->InCommand*/ )
				d->SendText("\n\r", 2 );
			
			if ( IS_SET(this->act, PLR_PROMPT) )
				display_prompt(d);

			d->InPrompt = false;

			/*if ( IS_SET(ch->act, PLR_TELNET_GA) )
				SendText( go_ahead_str );*/
		}
	}
}


void Character::ProcessLine(const string line, SocketGeneral * socket )
{
	if (line.length() == 0)
		return;

	//printf("Character object received input.\n\r");

	PlayerConnection * d = this->GetConnection();

	if (d)
	{
		if (d->ConnectedState == CON_PLAYING)
			interpret(this, line.c_str());
		else if (d->ConnectedState == CON_EDITING)
			edit_buffer( this, (char *) line.c_str() );
	}
	else
		interpret(this, line.c_str());

	/*
	 * Bust a prompt.
	 */
	if (d && d->InCommand)
	{
	    SendPrompt();
	}
}


const shared_str & Character::getShort(bool useStageName)
const
{
	if ( useStageName && stageName_.length() > 0 )
		return stageName_;

	if ( IS_NPC(this) )
		return shortDesc_;
	else
		return name_;
}

const shared_str & Character::getBamfIn() const
{
	// cannot initialize these statically -- see known issues of shared_str doc
	static shared_str * defaultBamfIn = NULL;
		
	if ( pcdata == NULL || pcdata->bamfIn_.length() == 0 )
	{
		if ( defaultBamfIn == NULL )
			defaultBamfIn = new shared_str("$n appears in a swirling mist.");

		return *defaultBamfIn;
	}
	else
		return pcdata->bamfIn_;
}

const shared_str & Character::getBamfOut() const
{
	// cannot initialize these statically -- see known issues of shared_str doc
	static shared_str * defaultBamfOut = NULL;
		
	if ( pcdata == NULL || pcdata->bamfOut_.length() == 0 )
	{
		if ( defaultBamfOut == NULL )
			defaultBamfOut = new shared_str("$n leaves in a swirling mist.");

		return *defaultBamfOut;
	}
	else
		return pcdata->bamfOut_;
}



//////////////////////////////////////////////////////
// SNOOPING
//////////////////////////////////////////////////////

void Character::StartSnoopedBy(PlayerConnection * connection)
{
	if ( !connection )
		return;

	SnoopedBy.push_back( connection->GetId() );

	PlayerConnection * d = this->GetConnection();

	if( d && d->Account && d->Account->flags & ACCOUNT_NOTIFY)
	{
		char buf[MAX_INPUT_LENGTH];
		sprintf(buf,"\r\n~~You feel as though %s is watching your every move.~~\r\n", NAME(connection->GetCharacter()) );
		sendText(buf, false);
	}
}

void Character::StopSnoopedBy(PlayerConnection * connection)
{
	if ( !connection )
		return;

	if ( listContains( SnoopedBy, connection->GetId() ) )
	{
		SnoopedBy.remove( connection->GetId() );
	}

	PlayerConnection * d = this->GetConnection();

	if ( d && d->Account && d->Account->flags & ACCOUNT_NOTIFY)
	{
		d->SendText("\r\n~~You no longer feel watched.~~\r\n");
	}
}

bool Character::IsSnooped()
{
	return ( SnoopedBy.size() > 0 );
}

string Character::SnoopedByNames()
{
	string result;
	itorSocketId itor;
	for (itor = SnoopedBy.begin(); itor != SnoopedBy.end(); itor++)
	{
		PlayerConnection * connection = (PlayerConnection*) SocketMap[*itor];

		result.append( NAME( connection->GetOriginalCharacter() ) );
		result.append(" ");
	}

	return result;
}

void Character::SendCommandToSnoopers(const char * name, const char * text)
{
	itorSocketId itor;
	for (itor = SnoopedBy.begin(); itor != SnoopedBy.end(); itor++)
	{
		PlayerConnection * connection = (PlayerConnection*) SocketMap[*itor];

		connection->SendText(name);
		connection->SendText(": ", 2);
		connection->SendText(text);
		connection->SendText("\n\r", 2);
	}
}

void Character::SendOutputToSnoopers(const char * name, const char * text)
{
	itorSocketId itor;
	for (itor = SnoopedBy.begin(); itor != SnoopedBy.end(); itor++)
	{
		PlayerConnection * connection = (PlayerConnection*) SocketMap[*itor];

		connection->SendText(name);
		connection->SendText("% ", 2);
		connection->SendText(text);
		connection->SendText("\n\r", 2);
	}
}

void Character::AddWait(short howMuch)
{
	WaitTime += howMuch;
	if (WaitTime < 0)
		WaitTime = 0;
}
void Character::AddWaitSeconds(short howMuch)
{
	WaitTime += howMuch * (1000 / FRAME_TIME);
	if (WaitTime < 0)
		WaitTime = 0;
}
short Character::GetWait()
{
	return WaitTime;
}
short Character::GetWaitSeconds()
{
	return (short) (WaitTime / (1000 / FRAME_TIME));
}

/*
=================================
   ____ ___ ___           __     
  / __// _// _/___  ____ / /_ ___
 / _/ / _// _// -_)/ __// __/(_-<
/___//_/ /_/  \__/ \__/ \__//___/
                                 
    __    ____           __ __   
 __/ /_  / __/___  ___  / // /___
/_  __/ _\ \ / _ \/ -_)/ // /(_-<
 /_/   /___// .__/\__//_//_//___/
           /_/                   
=================================
*/

bool Character::IsAffected(AFFECT_DATA * affect)
{
	AFFECT_DATA * paf;
	for ( paf = this->first_affect; paf != NULL; paf = paf->next)
	{
		if (paf == affect)
			return true;
	}
	return false;
}

/*
===============================
  _____           __        __ 
 / ___/__  __ _  / /  ___ _/ /_
/ /__/ _ \/  ' \/ _ \/ _ `/ __/
\___/\___/_/_/_/_.__/\_,_/\__/ 
                                  
===============================
*/

bool Character::IsAttackedBy(idCharacter ch)
{
	return ( AttackedBy.find(ch) != AttackedBy.end() );
}

bool Character::IsFighting()
{
	return ( IsAttacking() || AttackedBy.size() > 0 );
}

bool Character::IsAttacking()
{
	return GetVictim() != NULL;
}

void Character::StartAttacking(Character * victim)
{
	/* Limit attackers -Thoric */
	if ( victim->GetNumFighting() > max_fight(victim) )
	{
		this->sendText( "There are too many people fighting for you to join in.\n\r" );
		return;
	}

	VictimCharId = victim->GetId();

	// Sets only keep one copy of the data, so don't need to worry about already being there
	victim->AttackedBy.insert( this->GetId() );

	if ( in_arena(this) && !(IS_NPC(this) && IS_NPC(victim)) && (victim->GetVictim() != this))
	{
		char buf[MAX_STRING_LENGTH];

		sprintf(buf, "%s snarls in rage as %s throws %sself against %s.", 
				NAME(victim), 
				SEX_HE(victim), 
				SEX_HIM(victim), 
				NAME(this));
		talk_channel(this, buf, CHANNEL_ARENA, "");
	}

	/* Testaur - log (non-arena) fights against players */
	/* note: victim is actually the aggressor,  ch is the one attacked! */

	if(!in_arena(this) && !IS_NPC(this) && ( victim->GetVictim() != this))
	{
		/* some player has been attacked for real... */
		char buf[MAX_STRING_LENGTH];
		char *ptr;
		CHAR_DATA *whodunit;

		/* whodunit? */
		whodunit=NULL;
		if(IS_NPC(victim))
		{
			/* see if this NPC is acting on some player's orders */
			if(IS_AFFECTED( victim , AFF_CHARM ) && victim->GetMaster() != NULL )
			{
				if (!IS_NPC( victim->GetMaster() ))
				{
					whodunit = victim->GetMaster();
					sprintf( buf, "PK: %s's charmed %s", NAME(whodunit), NAME(victim));
				}
			}
		}
		else
		{
			/* a player did it. */
			whodunit=victim;
			sprintf( buf, "PK: %s", NAME(whodunit));
		}

		if( whodunit )  /* a player was the instigator */
		{
			for(ptr=buf; *ptr ; ptr++)
				;
			/* now determine legality of attack */
			if( whodunit->level - this->level > 5 )
			{/* illegal attack */
				sprintf(ptr, " unfairly");
			}
			else
			{
				if( this->level - whodunit->level > 5 )
				{/* suicidal attack */
					sprintf(ptr, " foolishly");
				}
			}
			while(*ptr)
				ptr++;

			sprintf( ptr, " attacks %s", NAME(this) );
			log_string(buf);
			to_channel(buf,CHANNEL_MONITOR,"Monitor",0);      
		}
	} /* end of log section */

	if ( IS_AFFECTED(this, AFF_SLEEP) )
		affect_strip( this, gsn_sleep );

	this->position = POS_FIGHTING;
	
	if ( victim->GetSwitchedChar() != NULL && IS_AFFECTED(victim->GetSwitchedChar(), AFF_POSSESS) )
	{
		send_to_char( "You are disturbed!\n\r", victim->GetSwitchedChar() );
		do_return( victim->GetSwitchedChar(), "" );
	}
	
	return;
}

void Character::StopAllFights()
{
	StopAttacking();
	StopAttacked();
}

void Character::StopAttacking()
{
	Character * victim = GetVictim();
	if ( !victim )
		return;
	
	victim->AttackedBy.erase( this->GetId() );
	VictimCharId = 0;
	
	
	if ( GetMount() )
		position = POS_MOUNTED;
	else
		position = POS_STANDING;

	if ( IS_AFFECTED(this, AFF_BERSERK) )
	{
		affect_strip(this, gsn_berserk);
		set_char_color(AT_WEAROFF, this);
		send_to_char(skill_table[gsn_berserk]->msgOff_.c_str(), this);
        send_to_char("\n\r", this);
	}
	
	update_pos(this);
}

void Character::StopAttacked()
{
	// 2005-10-24 - iterate over a local copy because iteration over set
	// while elements are being removed is a bad no-no           Ksilyan
	set<idCharacter> localCopy = AttackedBy;
	
	set<idCharacter>::iterator it;

	for ( it = localCopy.begin(); it != localCopy.end(); it++ )
	{
		// inform each attacker that they aren't attacking anymore
		Character * attacker = CharacterMap[*it];
		
		// Make sure attacker is valid
		if ( attacker == NULL )
			continue;
		
		attacker->StopAttacking();
	}

	// Shouldn't need to do this, but, well, just in case...
	AttackedBy.clear();
}

uint Character::GetNumFighting()
{
	// Get number of people attacking me
	uint total = AttackedBy.size();

	// Check if victim is attacking me ... if so, don't count twice
	if ( GetVictim() != NULL && AttackedBy.find( GetVictim()->GetId() ) == AttackedBy.end() )
		total++;

	return total;
}

uint Character::GetTimesKilled(Character * victim)
{
	// NPCs don't track kills
	if ( IS_NPC(this) )
		return 0;
	
	// Don't track kills of a player
	if ( !IS_NPC(victim) )
		return 0;

	int vnum = victim->pIndexData->vnum;
	short track = URANGE(2, ((this->level+3) * MAX_KILLTRACK)/LEVEL_HERO_MAX, MAX_KILLTRACK );

	for ( short x = 0; x < track; x++ )
	{
		if ( this->pcdata->killed[x].vnum == vnum )
			return this->pcdata->killed[x].count;
		else if ( this->pcdata->killed[x].vnum == 0 )
			break; // last entry, stop searching
	}

	// Not found
	return 0;
}

void Character::GainAccumulatedXp()
{
	if ( accumulated_exp == 0 )
		return;

	ch_printf(this, "You receive %d experience points!\r\n", accumulated_exp);
	gain_exp(this, accumulated_exp, false);
}

/*
=====================================
   ____ __         __         ____
  / __// /_ ___ _ / /_ ___   / __/___
 _\ \ / __// _ `// __/(_-<   > _/_ _/
/___/_\__/ \_,_/_\__//___/  |_____/
  / _ \ ___   / // /___
 / , _// _ \ / // /(_-<
/_/|_| \___//_//_//___/

=====================================
*/

short Character::getSkillProficiency(short skillNumber) const
{
	if ( IS_NPC(this) )
		return UMAX(100, this->level * 2);
	else
		return this->pcdata->learned[skillNumber];
}


inline short GetMax(const Character * ch, short attribute)
{
	if ( !ch )
		return 0;

	if ( IS_NPC(ch) || class_table[ch->Class]->attr_prime == attribute )
		return 25;
	else
		return 20;
}

short Character::getStr() const
{
	short max;
	max = GetMax(this, APPLY_STR);

	return URANGE ( 3, this->perm_str + this->mod_str, max);
}
short Character::getBaseStr() const
{
	return perm_str;
}

short Character::getInt() const
{
	short max;
	max = GetMax(this, APPLY_INT);

	return URANGE ( 3, this->perm_int + this->mod_int, max);
}
short Character::getBaseInt() const
{
	return perm_int;
}

short Character::getWis() const
{
	short max;
	max = GetMax(this, APPLY_WIS);

	return URANGE ( 3, this->perm_wis + this->mod_wis, max);
}
short Character::getBaseWis() const
{
	return perm_wis;
}

short Character::getDex() const
{
	short max;
	max = GetMax(this, APPLY_DEX);

	return URANGE ( 3, this->perm_dex + this->mod_dex, max);
}
short Character::getBaseDex() const
{
	return perm_dex;
}

short Character::getCon() const
{
	short max;
	max = GetMax(this, APPLY_CON);

	return URANGE ( 3, this->perm_con + this->mod_con, max);
}
short Character::getBaseCon() const
{
	return perm_con;
}

short Character::getCha() const
{
	short max;
	max = GetMax(this, APPLY_CHA);

	return URANGE ( 3, this->perm_cha + this->mod_cha, max);
}
short Character::getBaseCha() const
{
	return perm_cha;
}

short Character::getLck() const
{
	short max;
	max = GetMax(this, APPLY_LCK);

	return URANGE ( 3, this->perm_lck + this->mod_lck, max);
}
short Character::getBaseLck() const
{
	return perm_lck;
}

short Character::getLuckBonus() const
{
	return this->getLck() - 13;
}

short Character::getAC() const
{
	short result;

	result = this->armor;
	if ( IS_AWAKE(this) )
		result += dex_app[this->getDex()].defensive;
	result += VAMP_AC(this);

	return result;
}

short Character::getHitRoll() const
{
	short result = this->hitroll;

	result += str_app[this->getStr()].tohit;

	result += ( 2 - abs(this->mental_state / 10) );

	// Cap out the hitroll at 15.
	if ( result > 15 )
		result = 15;

	/* Knight class mounted hitroll bonus */
	#define MNTED_KNIGHT_BONUS	2

	if((this->Class == CLASS_KNIGHT) && (this->position == POS_MOUNTED))
		result += MNTED_KNIGHT_BONUS;

	return result;
}

short Character::getDamRoll() const
{
	short result = this->damroll;

	result += str_app[this->getStr()].todam;

	if ( this->mental_state > 5 && this->mental_state < 15 )
		// player is slightly stimulated, so up the damroll
		result++;

	if ( result <= 15 )
		// keep the value
		;
	else
		result = 15;

	return result;
}


/*
 * GetChanceModifier: centralized location
 * for determining a character's general
 * "luckiness", with factors such as deity
 * alignment, clan alignment, etc.
 *
 * A positive result is helpful, a negative
 * result is not.
 *     -Ksilyan
 */

short GetChanceModifier(Character * ch)
{
	short result = 0;

	// penalty for differences between ch's alignment, and ch's clan's alignment
	// Note that neutral characters have smaller maximum penalties as they can
	// only be off by 1,000 points (instead of 2,000)
	if ( IS_CLANNED(ch) )
		result -= abs(ch->pcdata->clan->alignment - ch->alignment) / 100;
  

	if ( IS_DEVOTED( ch ) )
	{
		// penalty for differences between ch's alignment, and ch's deity's alignment
		result -= abs(ch->pcdata->deity->alignment - ch->alignment) / 100;

		result += ch->pcdata->favor / 200;
	}

	/* Mental state bonus/penalty:  Your mental state is a ranged value with
	 * zero (0) being at a perfect mental state (bonus of 2).
	 * negative values would reflect how sedated one is, and
	 * positive values would reflect how stimulated one is.
	 * In most circumstances you'd do best at a perfectly balanced state.
	 */
	
	result += (10 - abs(ch->mental_state)) / 5;
	// examples:
	// ms == +- 100 --> -18
	// ms ==      0 --> +2
	// ms == +-  50 --> -8

	return result;
}

/*
 * Scryn, standard luck check 2/2/96
 * translated to C++, Ksilyan, Sep-2-03
 */
bool Character::ChanceRoll(short successChance, short modifier)
{
	if ( number_percent() - GetChanceModifier(this) - modifier <= successChance )
		return true;
	else
		return false;
}

bool Character::ChanceRollSkill(short skillNumber, short modifier)
{
	int successChance = this->getSkillProficiency(skillNumber);

	if (number_percent() - GetChanceModifier(this) - modifier <= successChance )
		return true;
	else
		return false;
}

/* Scryn, standard luck check + consideration of 1 attrib 2/2/96*/ 
// translated to C++ by Ksilyan, Sep-2-03

bool Character::ChanceRollAttribute(short successChance, short attributeScore, short modifier)
{	
	if (number_percent() - GetChanceModifier(this) - modifier - (attributeScore - 13) <= successChance )
		return true;
	else
		return false;
	
}



int Character::GetLevelGainHP()
{
	int gain = con_app[getBaseCon()].hitp + number_range(
		class_table[Class]->hp_min,
		class_table[Class]->hp_max );

	return MAX(gain, 1);
}
int Character::GetLevelGainMana()
{
	int gain  = class_table[Class]->fMana
		? number_range(2, (2*getBaseInt()+getBaseWis())/8)
		: 0;

	return MAX(gain, 0);

}
int Character::GetLevelGainMove()
{
	int gain = number_range( 5, (getBaseCon()+getBaseDex())/4 );
	return MAX(gain, 10);
}

/*
================================
   ____ __         __
  / __// /_ ___ _ / /_ __ __ ___
 _\ \ / __// _ `// __// // /(_-<
/___/ \__/ \_,_/ \__/ \_,_//___/

================================
*/

bool Character::isOutside() const
{
	// Can't use normal GetInRoom because that's not const.
	Room * room = RoomMap[InRoomId];

	if ( !room )
	{
		gTheWorld->LogBugString( "ERROR: isOutside: bad InRoomId! " + this->codeGetBasicInfo() );
		return false;
	}

	return ( !IS_SET(room->room_flags, ROOM_INDOORS) &&
		!is_inside[room->sector_type] );
}

void Character::setScent(const Scent * scent)
{
	scentId_ = scent->getID();
}

Scent * Character::getScent()
{
	if ( scentId_ == 0 )
		return NULL;

	Scent * scent = ScentController::instance()->findScent( scentId_ );

	if ( scent == NULL )
	{
		ostringstream msg;
		msg << "ERROR: Character::getScent(): bad scentId '" << scentId_ << "' for character " << getName().str();
		gTheWorld->LogBugString( msg.str() );
		scentId_ = 0;
		return NULL;
	}

	return scent;
}

/*
=======================================
   ____    __
  /  _/___/ /
 _/ / / _  /
/___/ \_,_/
   __              __
  / /  ___  ___   / /__ __ __ ___   ___
 / /__/ _ \/ _ \ /  '_// // // _ \ (_-<
/____/\___/\___//_/\_\ \_,_// .__//___/
                           /_/
=======================================
*/

// lookups will return null if not found

#define C_CHARACTER_LOOKUP(name, type, idName, mapName) \
type Character::name() \
{ \
	if ( !idName ) \
		return NULL; /* don't bother looking up */ \
	else \
	{ \
		type result = mapName[this->idName]; \
		if ( result ) \
			return result; \
		else \
		{ \
			gTheWorld->LogCodeString( \
				"Character::" #name "() invalid " #idName " reference! Char: " + this->codeGetBasicInfo() ); \
			this->idName = 0; \
			return NULL; \
		} \
	} \
}
// #arg preprocessor operator means: make this argument a string ("arg")

C_CHARACTER_LOOKUP(GetInRoom, Room *, InRoomId, RoomMap)
C_CHARACTER_LOOKUP(GetWasInRoom, Room *, WasInRoomId, RoomMap)


C_CHARACTER_LOOKUP(GetMount, Character *, MountId, CharacterMap)
C_CHARACTER_LOOKUP(GetMaster, Character *, MasterId, CharacterMap)
C_CHARACTER_LOOKUP(GetLeader, Character *, LeaderId, CharacterMap)
C_CHARACTER_LOOKUP(GetReplyToChar, Character *, ReplyToCharId, CharacterMap)
C_CHARACTER_LOOKUP(GetRetellToChar, Character *, RetellToCharId, CharacterMap)
C_CHARACTER_LOOKUP(GetSwitchedChar, Character *, SwitchedCharId, CharacterMap)
C_CHARACTER_LOOKUP(GetDestinationChar, Character *, DestinationCharId, CharacterMap)
C_CHARACTER_LOOKUP(GetVictim, Character *, VictimCharId, CharacterMap)


C_CHARACTER_LOOKUP(GetSittingOn, Object *, SittingOnId, ObjectMap)
C_CHARACTER_LOOKUP(GetQuestNote, Object *, QuestNoteId, ObjectMap)

PlayerConnection * Character::GetConnection()
{
	if ( ConnectionId == 0 )
		return NULL;
	else
	{
		PlayerConnection * result = (PlayerConnection *) SocketMap[ConnectionId];
		if ( result )
			return result;
		else
		{
			gTheWorld->LogCodeString("Character::GetConnection() invalid connection reference! Char: " + this->codeGetBasicInfo() );
			return NULL;
		}
	}
}

void Character::SetConnection(PlayerConnection * d)
{
	if ( !d )
		ConnectionId = 0;
	else
		ConnectionId = d->GetId();
}

/*
=================================
   ____ __                    __ 
  / __// /__ ____ __ __ ___  / /_
 _\ \ /  '_// __// // // _ \/ __/
/___//_/\_\/_/   \_, // .__/\__/ 
                /___//_/         
=================================
*/


void Character::skryptSendEvent(const string & eventName, list<Argument*> & arguments)
{
	if ( !this->skryptContainer_ )
		return;

	ArgumentMobile argMe(this);
	skryptContainer_->SendEvent(eventName, argMe, arguments, this);
}


////////////////////////
// Code object interface
////////////////////////

const string Character::codeGetBasicInfo() const
{
	string result;
	if ( IS_NPC(this) )
		result = this->shortDesc_.str() + "(" + this->name_.str() + ")";
	else
		result = this->name_.str();

	return result;
}

const string Character::codeGetFullInfo() const
{
	string result;
	if ( IS_NPC(this) )
		result = this->shortDesc_.str() + "(" + this->name_.str() + ")";
	else
		result = this->name_.str();

	return result;
}

const string Character::codeGetClassName() const
{
	return "Character";
}
