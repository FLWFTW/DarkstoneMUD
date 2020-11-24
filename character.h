



#ifndef __CHARACTER_H_
#define __CHARACTER_H_

#include "mud.h"
#include "managed_objects.hpp"
#include "InputHandler.h"
#include "memory.h"

#include <list>
#include <set>

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */



class Character;
extern tPointerOwner<Character> CharacterMap; // character.cpp

typedef tId<Character> idCharacter;

// forward declarations
class PlayerConnection;
class Scent;
typedef tId<Object> idObject;
typedef tId<Room> idRoom;
typedef tId<SocketGeneral> idSocket;
struct PlayerData;

class Character : public tManagedObject<Character>, public InputHandler, public cabSkryptable
{
private:
	std::list<std::string> IncomingLines; // the lines that are waiting to be processed
	std::string LastCommand; // the last command typed by the player
	int RepeatTimes; // how many times the player typed the same line

	std::list<idSocket> SnoopedBy; // Who's snooping this character.

	std::string SnoopBuffer; // buffer created for snooping

	bool ReadyForPrompt;

	short WaitTime; // in clock-ticks

	idSocket ConnectionId; // connection tied to this character

	idCharacter VictimCharId; //!< who I'm attacking
	std::set<idCharacter> AttackedBy; //!< A list of people who are attacking me.

	shared_str  name_;
	shared_str  shortDesc_; // for mobs, primarily
	shared_str stageName_; //!< A temporary short-desc replacement - is not saved   - Ksilyan sep-13-2004

	std::string movementMsgName_; //!< The name of the movement message being used by this character.

	// AIYAN - SCENT SYSTEM - 28/03/05
	//  -ksilyan, 2005-09-07: made private, added getter
	int scentId_;
	
public:
	Character();
	virtual ~Character();


	////////////////////////////
	// Input/Output Functions //
	////////////////////////////

	virtual void ProcessLine(const std::string line, SocketGeneral * socket ); // line to be interpreted/handled

	inline virtual void sendText(const std::string & text, bool color = true) // pass along text.c_str() - auxiliary output
	{
		if (!ConnectionId)
			return;
		if (text.length() == 0)
			return;
		this->sendText(text.c_str(), color);
	}
        inline virtual void sendText(const shared_str & text, bool color = true)
        {
            this->sendText(text.c_str(), color);
        }
	virtual void sendText(const char * text, bool color = true); // pass string through color translator - primary output

	virtual void OutputBufferEmptied();

	virtual void SendPrompt(); // send a prompt to the character

	PlayerConnection * GetConnection(); // look up connection attached to this character
	void SetConnection(PlayerConnection * d); // set connection reference to d's ID

	////////////////////
	// Snoop commands //
	////////////////////

	void StartSnoopedBy(PlayerConnection * connection);
	void StopSnoopedBy(PlayerConnection * connection);
	bool IsSnooped();
	std::string SnoopedByNames();
	void SendCommandToSnoopers(const char * name, const char * text);
	void SendOutputToSnoopers(const char * name, const char * text);

	///////////////////
	// Wait commands //
	///////////////////

	void AddWait(short howMuch); // add wait in ticks
	short GetWait(); // returns wait in ticks
	void AddWaitSeconds(short howMuch); // add wait in seconds
	short GetWaitSeconds(); // returns wait in seconds

	////////////////////////
	// Effects and Spells //
	////////////////////////

	bool IsAffected(AFFECT_DATA * affect); // is this structure in the list?

	std::list<SpellMemory*> spellMemory_; // the list of spells cast on me and by me

	short BarricadeDir; // the direction this character is barricading

        ////////////
        // Quests //
        ////////////

        bool isQuestActive(const shared_str & name);
        bool isQuestSucceeded(const shared_str & name);
        bool isQuestFailed(const shared_str & name);
        bool isQuestTerminated(const shared_str & name);

        

	///////////
	// Stats //
	///////////

	short getStr() const; // current str
	short getInt() const; // current int
	short getWis() const; // current wis
	short getDex() const; // current dex
	short getCon() const; // current con
	short getCha() const; // current cha
	short getLck() const; // current luck

	short getBaseStr() const; // base str
	short getBaseInt() const; // base int
	short getBaseWis() const; // base wis
	short getBaseDex() const; // base dex
	short getBaseCon() const; // base con
	short getBaseCha() const; // base cha
	short getBaseLck() const; // base luck

	short getLuckBonus() const; // the luck bonus: luck-13

	short getAC() const;
	short getHitRoll() const;
	short getDamRoll() const;

	short getSkillProficiency(short skillNumber) const;

	int GetLevelGainHP();
	int GetLevelGainMana();
	int GetLevelGainMove();

	sh_int		hit;
	sh_int		max_hit;
	sh_int		mana;
	sh_int		max_mana;
	sh_int		move;
	sh_int		max_move;

	int BaseMaxHp;
	int BaseMaxMana;
	int BaseMaxMove;

	////////////
	// Status //
	////////////

	bool isOutside() const;
	inline bool isInside() const { return !this->isOutside(); }

	void setName(const shared_str  & name) { name_ = name; }
	const shared_str & getName() const { return name_; }
	
	void setShort(const shared_str & str) { shortDesc_ = str; }
	/*
	 * Method: getShort
	 *
	 * For a mob, returns short desc - for a player returns name.
	 * In all cases, if useStageName == true, returns stage name if it exists.
         *
         * Parameters:
         *  useStageName - set to true if the stage name should be used; false
         *                 if the mob's real name should be used. Default: true.
         *
         *
         * Returns:
         *   The short name of the character.
	 */
	const shared_str & getShort(bool useStageName = true) const;

	const shared_str & getBamfIn() const;
	const shared_str & getBamfOut() const;

	void setLong(const std::string & str) { longDesc_ = str; }
	const shared_str & getLong() const { return longDesc_; }

	const shared_str & getStageName() const { return stageName_; }
	void setStageName(const std::string & name) { stageName_ = name; }
	bool hasStageName() const { return stageName_ != ""; }

	std::string getMovementMessage() const { return movementMsgName_; }
	void setMovementMessage(const std::string & msg) { movementMsgName_ = msg; }

	void setScent(int scent) { scentId_ = scent; }
	void setScent(const Scent * scent);
	int getScentId() const { return scentId_; }
	Scent * getScent();

	////////////
	// Combat //
	////////////

	Character * GetVictim();
	bool IsAttackedBy(idCharacter ch);

	bool IsAttacking(); // am I attacking anybody?
	bool IsFighting(); // am I fighting? (attacking and/or attacked)

	void StartAttacking(Character * victim); // engage combat with victim

	void StopAllFights(); // immediately remove me from any fights I'm involved in
	void StopAttacking(); // stop attacking target
	void StopAttacked(); // all people attacking me stop attacking

	uint GetNumFighting(); // get number of people I'm fighting	(attacking/attacked), without counting anyone twice

	uint GetTimesKilled(Character * victim); // how many times I have killed victim

	void GainAccumulatedXp();

	/////////////
	// Rolling //
	/////////////

	// A positive modifier is helpful, a negative modifier is not
	bool ChanceRoll(short successChance, short modifier = 0);
	bool ChanceRollSkill(short skillNumber, short modifier = 0);
	bool ChanceRollAttribute(short successChance, short attributeScore, short modifier = 0);

	////////////////
	// ID Lookups //
	////////////////

	Character * GetMaster();
	Character * GetLeader();
	Character * GetReplyToChar();
	Character * GetRetellToChar();
	Character * GetSwitchedChar();
	Character * GetMount(); // the character's mount
	Character * GetDestinationChar(); // for mpwalking


	Object * GetSittingOn(); // object the character is sitting on
	Object * GetQuestNote();

	Room * GetInRoom(); // look up room this player is in
	Room * GetWasInRoom();

	// data members
	idCharacter MasterId;
    idCharacter LeaderId;
	idCharacter ReplyToCharId;
	idCharacter RetellToCharId;
	idCharacter SwitchedCharId;
	idCharacter MountId;
	idCharacter DestinationCharId; // mpwalking target

	idObject SittingOnId;
	idObject QuestNoteId;

	idRoom InRoomId;
	idRoom WasInRoomId;


	///////////////////////////
	// Code object interface //
	///////////////////////////
	virtual const std::string codeGetClassName() const; // Return the name of the class.
	virtual const std::string codeGetBasicInfo() const; // Return a short summary of the object.
	virtual const std::string codeGetFullInfo() const;  // Return a complete summary of the object.


	// Send a Skrypt event.
	virtual void skryptSendEvent(const std::string & eventName, std::list<Argument*> & arguments);


	//////////////////////
	// Old data members //
	// (from mud.h)     //
	//////////////////////

	Character *		next;
	Character *		prev;
	Character *		next_in_room;
	Character *		prev_in_room;
	FIGHT_DATA *	fiiighting;
	HuntHateFear *		hunting;
	HuntHateFear *		fearing;
	HuntHateFear *		hating;
	SPEC_FUN *		spec_fun;
	MPROG_ACT_LIST *	mpact;
	int 		mpactnum;
	sh_int		mpscriptpos;
	MOB_INDEX_DATA *	pIndexData;
  
	AFFECT_DATA *	first_affect;
	AFFECT_DATA *	last_affect;
	NoteData * 	pnote;
	Object * first_carrying;
	Object * last_carrying;
	PlayerData *		pcdata;
	DO_FUN *		last_cmd;
	DO_FUN *		prev_cmd;	/* mapping */
	void *		dest_buf;
	void *		spare_ptr;
	int 		tempnum;
	EDITOR_DATA *	editor;
	TIMER	*	first_timer;
	TIMER	*	last_timer;
	shared_str	longDesc_;
	shared_str  description_;
	//sh_int		num_fighting;
	sh_int		substate;
	sh_int		sex;
	sh_int		Class;
	sh_int		race;
	sh_int		level;
	sh_int		trust;
	int 		played;
	time_t		secLogonTime; // in seconds
	time_t		secSaveTime; // in seconds
	sh_int		timer;
	sh_int		practice;
	sh_int		numattacks;
	int 		gold;
	int 		exp;
	int 		accumulated_exp;
	int 		act;
	int 		affected_by;
	int 		carry_weight;
	int 		carry_number;
	int 		xflags;
	int 		resistant;
	int 		immune;
	int 		susceptible;
	int 		attacks;
	int 		defenses;
	int 		speaks;
	int 		speaking;
	sh_int		saving_poison_death;
	sh_int		saving_wand;
	sh_int		saving_para_petri;
	sh_int		saving_breath;
	sh_int		saving_spell_staff;
	sh_int		alignment;
	sh_int		barenumdie;
	sh_int		baresizedie;
	sh_int		mobthac0;
	sh_int		hitroll;
	sh_int		damroll;
	sh_int		hitplus;
	sh_int		damplus;
	sh_int		position;
	sh_int		defposition;
	sh_int		height;
	sh_int		weight;
	sh_int		armor;
	sh_int		wimpy;
	int 		deaf;
	sh_int		perm_str;
	sh_int		perm_int;
	sh_int		perm_wis;
	sh_int		perm_dex;
	sh_int		perm_con;
	sh_int		perm_cha;
	sh_int		perm_lck;
	sh_int		mod_str;
	sh_int		mod_int;
	sh_int		mod_wis;
	sh_int		mod_dex;
	sh_int		mod_con;
	sh_int		mod_cha;
	sh_int		mod_lck;
	sh_int		mental_state;		/* simplified */
	sh_int		emotional_state;	/* simplified */
	int 		pagelen;						/* BUILD INTERFACE */
	sh_int		inter_page; 					/* BUILD INTERFACE */
	sh_int		inter_type; 					/* BUILD INTERFACE */
	char		*inter_editing; 				/* BUILD INTERFACE */
	int 		inter_editing_vnum; 			/* BUILD INTERFACE */
	sh_int		inter_substate; 				/* BUILD INTERFACE */
	int 		retran;
	int 		regoto;
	sh_int		mobinvis;	/* Mobinvis level SB */
	int 			 vnum_destination;
   
	shared_str enterDesc_;
	shared_str exitDesc_;
	int 		mount_vnum;

	//Ksilyan: mobs only.
	int HomeVnum;
};

typedef std::list<idCharacter>::iterator itorCharacterId;

#endif
