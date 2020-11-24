

/***************************************************************\
*                                                               *
*                           KSILYAN                             *
*                                                               *
*****************************************************************
*                                                               *
* This is the code for using the banking system. Players go to  *
* a "banker" mob, and withdraw and deposit money.               *
*                                                               *
* This also has the gem code in it.                             *
*                                                               *
\***************************************************************/

#include "mud.h"
#include "db_public.h"
#include "stored_objs.h"
#include <stdlib.h>
#include <string.h>

// Forward declarations
void do_say(CHAR_DATA *ch, const char* argument);
void do_tell(CHAR_DATA *ch, const char* argument);
void do_whisper(CHAR_DATA *ch, const char* argument);
void do_emote(CHAR_DATA * ch, const char* argument);
void do_save(CHAR_DATA *ch, const char* argument);

void do_bank(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA * mob;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
					

	if (IS_NPC(ch))
	{
		send_to_char( "Only players can use the bank!\n\r", ch);
		return;
	}

	/* Check for mob with act->banker */
	for ( mob = ch->GetInRoom()->first_person; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_BANKER ) )
			break;
	}
	if ( mob == NULL )
	{
		send_to_char( "There is no bank teller here to work with.\n\r", ch );
		return;
	}

	if (argument[0] == '\0')
	{
		sprintf(buf, "mutter %s", ch->getName().c_str());
		interpret(mob, buf);
		do_say(mob, "Don't waste my time! What kind of transaction do you wish to make?");
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_prefix(arg1, "balance"))
	{
		if (ch->pcdata->bank_gold == 0)
		{
			sprintf(buf, "%s %s, you have no money stored with us!", ch->getName().c_str(), ch->getName().c_str());
		}
		else
		{
			sprintf(buf, "%s You currently have %d gold stored in our bank.", ch->getName().c_str(), ch->pcdata->bank_gold);
		}
		do_tell(mob, buf);
		return;
	}
	if (!str_prefix(arg1, "deposit"))
	{
		int amount;
		if (is_number(arg2))
		{
			amount = atoi(arg2);

			if (amount > ch->gold)
			{
				sprintf(buf, "%s You don't have that much money, %s.", ch->getName().c_str(), ch->getName().c_str());
				do_tell(mob, buf);
				return;
			}
			else if (amount <= 0)
			{
				send_to_char("Please enter a positive number.\n\r",ch);
				return;
			}
			else
			{
				if (ch->gold - amount < 25)
				{
					sprintf(buf, "%s You don't have enough money to pay our 25 gold fee, %s.", ch->getName().c_str(), ch->getName().c_str());
					do_tell(mob, buf);
					return;
				}
				ch->gold -= amount + 25;
				ch->pcdata->bank_gold += amount;
				sprintf(buf, "$n takes %d gold %s from $N and deposits them into a safe.",
					amount, (amount > 1 ? "coins" : "coin"));
				act( AT_SOCIAL, buf, mob, NULL, ch, TO_NOTVICT);
				sprintf(buf, "$n takes %d gold %s from you and deposits them into a safe.",
					amount, (amount > 1 ? "coins" : "coin"));
				act( AT_SOCIAL, buf, mob, NULL, ch, TO_VICT);
				do_save(ch, "");
				return;
			}
		}
		else
		{
			send_to_char("Please enter a number when depositing gold.\n\r",ch);
			return;
		}
	}
	if (!str_prefix(arg1, "withdraw"))
	{
		int amount;
		if (is_number(arg2))
		{
			amount = atoi(arg2);

			if (amount > ch->pcdata->bank_gold)
			{
				sprintf(buf, "%s You don't have that much money stored, %s.", ch->getName().c_str(), ch->getName().c_str());
				do_tell(mob, buf);
				return;
			}
			else if (amount <= 0)
			{
				send_to_char("Please enter a positive number.\n\r",ch);
				return;
			}
			else
			{
				int fee;

				fee = amount/50;
				if (fee < 1) fee = 1;
				amount -= fee;
				ch->gold += amount;
				ch->pcdata->bank_gold -= amount + fee;
				sprintf(buf, "$n takes %d gold %s from a safe and gives them to $N.",
					amount, (amount > 1 ? "coins" : "coin"));
				act( AT_SOCIAL, buf, mob, NULL, ch, TO_NOTVICT);
				sprintf(buf, "$n takes %d gold %s from a safe and gives them to you.",
					amount, (amount > 1 ? "coins" : "coin"));
				act(AT_SOCIAL, buf, mob, NULL, ch, TO_VICT);
				do_save(ch, "");
				return;
			}
		}
		else
		{
			send_to_char("Please enter a number when withdrawing gold.\n\r",ch);
			return;
		}
	}
	if ( !str_prefix(arg1, "transfer"))
	{
		char destName[MAX_INPUT_LENGTH];
		int amount;

		argument = one_argument(argument, destName);

		if ( !is_number(arg2) )
		{
			ch->sendText("Please enter an amount to transfer.\n\r");
			return;
		}

		// Search for the player
		Character * dest = get_char_world(ch, destName);

		if ( !dest )
		{
			ch->sendText(string("I couldn't find '") + destName + "'.\n\r");
			return;
		}

		if ( IS_NPC(dest) )
		{
			ch->sendText("You can't transfer funds to an NPC!\n\r");
			return;
		}

		
		amount = atoi(arg2);

		if (amount > ch->pcdata->bank_gold)
		{
			sprintf(buf, "%s You don't have that much money stored, %s.", ch->getName().c_str(), ch->getName().c_str());
			do_tell(mob, buf);
			return;
		}
		else if (amount <= 0)
		{
			send_to_char("Please enter a positive number.\n\r",ch);
			return;
		}
		else
		{
			int fee;

			fee = amount/50;
			if (fee < 1) fee = 1;
			amount -= fee;
			
			dest->pcdata->bank_gold += amount;
			ch->pcdata->bank_gold -= amount + fee;
			
			sprintf(buf, "$n takes %d gold %s from a safe and puts them in another safe.",
					amount, (amount > 1 ? "coins" : "coin"));

			act( AT_SOCIAL, buf, mob, NULL, ch, TO_ROOM);
			
			do_save(ch, "");
			do_save(dest, "");
			
			return;
		}

	}

}

/*
 _   __             __ __     
| | / /___ _ __ __ / // /_ ___
| |/ // _ `// // // // __/(_-<
|___/ \_,_/ \_,_//_/ \__//___/
                              
*/

void do_vault(Character * ch, const char* argument)
{

	// Make sure that the person in question is a player!
	if ( IS_NPC(ch) || !ch->pcdata )
	{
		ch->sendText("Only players can have vaults!\n\r");
		return;
	}

	ch->sendText("Function currently unavailable\n\r");

#if 0

	Character * banker;

	/* Check for mob with act->banker */
	for ( banker = ch->GetInRoom()->first_person; banker; banker = banker->next_in_room )
	{
		if ( IS_NPC(banker) && IS_SET(banker->act, ACT_BANKER ) )
			break;
	}
	if ( banker == NULL )
	{
		send_to_char( "There is no bank teller here to work with.\n\r", ch );
		return;
	}

	if ( !argument || strlen(argument) == 0)
	{
		char buf[MAX_INPUT_LENGTH];

		sprintf(buf, "%s Do you want to see your vault contents, add an object to it, or retrieve something?", ch->getName().c_str());
		do_whisper(banker, buf);
		return;
	}

	char arg1[MAX_INPUT_LENGTH];

	// First, load up the locker if we need to.
	if ( !ch->pcdata->Vault )
	{
		ch->pcdata->Vault = MasterDatabase->GetVault(ch->getName().c_str());
		ch->pcdata->secVaultLastAccessed = secCurrentTime;
	}

	argument = one_argument(argument, arg1);

	if ( !str_cmp(arg1, "show") )
	{
		if ( !ch->pcdata->Vault )
		{
			string tellBuf;
			tellBuf = string(ch->getName().c_str()) + " Your vault is empty.";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}
		
		string tellBuf = string(ch->getName().c_str()) + " According to our records, your vault contains:";
		do_whisper(banker, (char *) tellBuf.c_str());
		ch->pcdata->Vault->ShowToCharacter(ch);

		return;
	}
	else if ( !str_cmp(arg1, "add") || !str_cmp(arg1, "deposit") || !str_cmp(arg1, "put") )
	{
		int fee = 100;

		if ( ch->gold < fee )
		{
			string tellBuf = string(ch->getName().c_str()) + " You don't have enough money to pay our fee.";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}
		
		if ( !ch->pcdata->Vault )
		{
			// The vault doesn't exist, so we need to create it.
			ch->pcdata->Vault = new ObjectStorage();
		}
		
		if ( ch->pcdata->Vault->ListSize() >= 5 )
		{
			// Hardcoded limit of 5 objects - for now.
			string tellBuf = string(ch->getName().c_str()) + " Your vault is full already.";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}
	
		char arg2[MAX_INPUT_LENGTH];
		argument = one_argument(argument, arg2);

		Object * obj;
		obj = get_obj_carry(ch, arg2);
		if (!obj)
		{
			string tellBuf = string(ch->getName().c_str()) + " You don't seem to have that object.";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}
		if ( obj->wear_loc != -1 )
		{
			string tellBuf = string(ch->getName().c_str()) + " You have to remove " + string(obj->short_descr) + " before I can store it!";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}
		if ( obj->first_content )
		{
			interpret(banker, string("mutter " + string(ch->getName().c_str())).c_str() );
			string tellBuf = string(ch->getName().c_str()) + " Don't you read policy? You have to empty " +
				string(obj->short_descr) + " before I can store it!";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}

		separate_obj(obj);
		obj_from_char(obj);
		ch->pcdata->Vault->AddObject(obj);

		char actBuf[MAX_STRING_LENGTH];
		
		sprintf(actBuf, "$n gives %s to $N, who puts it into the vault.", obj->short_descr );
		act( AT_SOCIAL, actBuf, ch, NULL, banker, TO_ROOM);
		sprintf(actBuf, "You give %s to $N, who puts it into the vault.", obj->short_descr );
		act( AT_SOCIAL, actBuf, ch, NULL, banker, TO_CHAR);
		
		sprintf(actBuf, "$n takes %d gold %s from $N and puts them in the bank coffers.",
				fee, (fee > 1 ? "coins" : "coin") );
		act( AT_SOCIAL, actBuf, banker, NULL, ch, TO_NOTVICT);
		sprintf(actBuf, "$n takes %d gold %s from you and puts them in the bank coffers.",
				fee, (fee > 1 ? "coins" : "coin") );
		act( AT_SOCIAL, actBuf, banker, NULL, ch, TO_VICT);

		// Save the vault.
		MasterDatabase->SaveVault(ch->getName().c_str(), ch->pcdata->Vault);
		ch->gold -= fee;

		return;
	}
	else if ( !str_cmp(arg1, "get") || !str_cmp(arg1, "retrieve") || !str_cmp(arg1, "withdraw") )
	{
		int fee = 100;

		if ( !ch->pcdata->Vault )
		{
			// No vault, no can't get objects.
			string tellBuf = string(ch->getName().c_str()) + " You can't retrieve items from an empty vault!";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}

		if ( ch->gold < fee )
		{
			string tellBuf = string(ch->getName().c_str()) + " You don't have enough money to pay our fee.";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}
		
		char arg2[MAX_INPUT_LENGTH];
		argument = one_argument(argument, arg2);
		if ( !is_number(arg2) )
		{
			string tellBuf = string(ch->getName().c_str()) + " Give me the number of the object you want me to retrieve.";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}

		short whichOne = atoi(arg2);

		do_emote(banker, "walks off to the bank vault.");

		Object * obj;
		obj = ch->pcdata->Vault->GetObject(whichOne);

		if (!obj)
		{
			do_emote(banker, "returns empty-handed.");
			string tellBuf = string(ch->getName().c_str()) + " I can't seem to find that in your vault.";
			do_whisper(banker, (char *) tellBuf.c_str());
			return;
		}

		obj_to_char(obj, ch);

		do_emote( banker, (char *) string("returns holding " + string(obj->short_descr) + ".").c_str() );

		string tellBuf = string(ch->getName().c_str()) + " Here is your " + string( myobj(obj) ) + ", " + string(ch->getName().c_str()) + ".";
		do_whisper(banker, (char *) tellBuf.c_str());

		char actBuf[MAX_STRING_LENGTH];
		sprintf(actBuf, "$n takes %d gold %s from $N and puts them in the bank coffers.",
				fee, (fee > 1 ? "coins" : "coin") );
		act( AT_SOCIAL, actBuf, banker, NULL, ch, TO_NOTVICT);
		sprintf(actBuf, "$n takes %d gold %s from you and puts them in the bank coffers.",
				fee, (fee > 1 ? "coins" : "coin") );
		act( AT_SOCIAL, actBuf, banker, NULL, ch, TO_VICT);

		ch->gold -= fee;

		// Save the vault.
		MasterDatabase->SaveVault(ch->getName().c_str(), ch->pcdata->Vault);
	}
	else
	{
		ch->SendText("Usage: vault show | add <object> | get <number>\n\r");
		return;
	}
#endif
}

void do_showvault(Character * character, const char* argument)
{
	/*
	string result;
	result = MasterDatabase->ShowVault(argument);

	character->SendText(result, false);*/

	character->sendText("Function currently unavailable\n\r");
}

/*
  ___ _ ___  __ _   ___
 / _ `// -_)/  ' \ (_-<
 \_, / \__//_/_/_//___/
/___/                  

*/

#define NUMBER_OF_GEMS 68
#define BASE_GEM_VNUM 98304

OBJ_INDEX_DATA * GemIndex[NUMBER_OF_GEMS];

int GetGemWorth(OBJ_DATA * gem)
{
	int value = 0;

	value = gem->pIndexData->value[OBJECT_GEM_VALUE]
	        * (gem->value[OBJECT_GEM_SIZE] + gem->value[OBJECT_GEM_QUALITY])
	        / 200;

	return UMAX(value, 1);
}

/* GemDropChance
 * Chance to drop a gem based on mob's level.
 */

int GemDropChance(CHAR_DATA * ch)
{
	return 5;
	//return (int) ch->level * 0.2;
}


/* WorthToGem
 * Takes a worth and gives a gem.
 */

OBJ_INDEX_DATA * WorthToGem(int Worth)
{
	short Index;
	int BaseValue;

	BaseValue = GemIndex[0]->value[OBJECT_GEM_VALUE];

	if      (Worth <= BaseValue * 5)
		Index = number_range(0, 8);
	else if (Worth <= BaseValue * 10)
		Index = number_range(9, 14);
	else if (Worth <= BaseValue * 26)
		Index = number_range(15, 29);
	else if (Worth <= BaseValue * 83)
		Index = number_range(30, 44);
	else if (Worth <= BaseValue * 320)
		Index = number_range(45, 59);
	else
		Index = number_range(60, NUMBER_OF_GEMS - 1);

	return GemIndex[Index];
}

/* RandomGemDrop
 * Returns a gem object if a mob drops one.
 * Otherwise, NULL.
 */

OBJ_DATA * RandomGemDrop(CHAR_DATA * ch)
{
	int Chance;
	int Percent;
	int ChanceBonus;
	int MinWorth, MaxWorth;
	int FinalWorth;

	OBJ_INDEX_DATA * GemIndex;
	OBJ_DATA * Gem;
	int DifferenceFactor;

	//char buf[MAX_STRING_LENGTH];

	Chance = GemDropChance(ch);
	Percent = number_percent();

	if ( Percent > Chance )
		return NULL;
	
	ChanceBonus = Chance - Percent;

	MinWorth = 10;
	MaxWorth = (ch->level + (2 * ChanceBonus)) * ch->level;

	FinalWorth = number_range(MinWorth, MaxWorth);
	GemIndex = WorthToGem(FinalWorth);
	DifferenceFactor = 100 * FinalWorth / GemIndex->value[OBJECT_GEM_VALUE];

	Gem = create_object(GemIndex, 0);
	Gem->value[OBJECT_GEM_QUALITY] = number_range(DifferenceFactor/2, DifferenceFactor);
	Gem->value[OBJECT_GEM_SIZE] = (2 * DifferenceFactor) - Gem->value[OBJECT_GEM_QUALITY];

	/*sprintf(buf, "Base worth: %d. Final worth: %d. Quality: %d. Size: %d. Vnum: %s.\n\r",
	        GemIndex->value[OBJECT_GEM_VALUE], FinalWorth, Gem->value[OBJECT_GEM_QUALITY],
	        Gem->value[OBJECT_GEM_SIZE], vnum_to_dotted(Gem->pIndexData->vnum));
	log_string(buf);*/

	return Gem;
}



void do_setgembase(CHAR_DATA * ch, const char* argument)
{
	int base = 0;

	if ( !is_number(argument) )
	{
		send_to_char("Please enter a valid number as an argument.\n\r", ch);
		return;
	}

	base = atoi(argument);

	if (base < 1)
	{
		send_to_char("Please enter a positive number as an argument.\n\r", ch);
		return;
	}

	PopulateGemIndex();
	InitGemValues(base);
	send_to_char("Done.\n\r", ch);
}

void do_worth(CHAR_DATA * ch, const char* argument)
{
	int worth;
	OBJ_DATA * gem = NULL;
	CHAR_DATA * mob = NULL;

	if (!argument || strlen(argument) == 0)
	{
		send_to_char("Check the worth of what?\n\r", ch);
		return;
	}

	gem = get_obj_carry(ch, argument);

	for (mob = ch->GetInRoom()->first_person; mob; mob = mob->next)
	{
		if (IS_NPC(mob) && IS_SET(mob->act, ACT_GEMDEALER))
			break;
	}

	if (!mob || !IS_NPC(mob) || !IS_SET(mob->act, ACT_GEMDEALER) )
	{
		send_to_char("There is no gem dealer here to work with.\n\r", ch);
		return;
	}

	if (!gem)
	{
		send_to_char("You don't have that gem.\n\r", ch);
		return;
	}
	
	worth = GetGemWorth(gem);

	set_char_color(AT_TELL, ch);
	ch_printf(ch, "%s tells you, ", capitalize(mob->getShort().c_str()));
	ch_printf(ch, "'%s is worth %d.'\n\r", capitalize(gem->shortDesc_.c_str()), worth);

	return;
}

void InitGems(int base)
{
	PopulateGemIndex();
	InitGemValues(base);
	log_string("Gems loaded.");
}

void PopulateGemIndex()
{
	int i;

	for (i = 0; i < NUMBER_OF_GEMS; i++)
	{
		GemIndex[i] = get_obj_index( BASE_GEM_VNUM + i );
		if ( GemIndex[i] == NULL )
		{
			bug("Could not load gem of index %d (%s)!", BASE_GEM_VNUM + i, vnum_to_dotted(BASE_GEM_VNUM+i));
		}
	}
}

void InitGemValues(int base)
{
	// quartz crystal
	if (GemIndex[0])
	GemIndex[0]->value[OBJECT_GEM_VALUE] = base;

	// blue topaz
	if (GemIndex[1])
	GemIndex[1]->value[OBJECT_GEM_VALUE] = (int) (GemIndex[0]->value[OBJECT_GEM_VALUE] * 1.5);

	// clear topaz
	if (GemIndex[2])
	GemIndex[2]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 2;

	// clear sapphire
	if (GemIndex[3])
	GemIndex[3]->value[OBJECT_GEM_VALUE] = (int) (GemIndex[0]->value[OBJECT_GEM_VALUE] * 2.5);

	// green adventurite stone
	if (GemIndex[4])
	GemIndex[4]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 3;

	// garnet
	if (GemIndex[5])
	GemIndex[5]->value[OBJECT_GEM_VALUE] = (int) (GemIndex[0]->value[OBJECT_GEM_VALUE] * 3.5);

	// black jasper
	if (GemIndex[6])
	GemIndex[6]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 4;

	// red jasper
	if (GemIndex[7])
	GemIndex[7]->value[OBJECT_GEM_VALUE] = (int) (GemIndex[0]->value[OBJECT_GEM_VALUE] * 4.5);

	// yellow jasper
	if (GemIndex[8])
	GemIndex[8]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 5;

	// bloodstone
	if (GemIndex[9])
	GemIndex[9]->value[OBJECT_GEM_VALUE] = (int) (GemIndex[0]->value[OBJECT_GEM_VALUE] * 5.5);

	// blue cordierite
	if (GemIndex[10])
	GemIndex[10]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 6;

	// carnerlian quartz
	if (GemIndex[11])
	GemIndex[11]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 7;

	// cat’s eye quartz
	if (GemIndex[12])
	GemIndex[12]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 8;

	// banded sardonyx stone
	if (GemIndex[13])
	GemIndex[13]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 9;

	// black onyx
	if (GemIndex[14])
	GemIndex[14]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 10;

	// blue spinel
	if (GemIndex[15])
	GemIndex[15]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 11;

	// pink spinel
	if (GemIndex[16])
	GemIndex[16]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 12;

	// red spinel
	if (GemIndex[17])
	GemIndex[17]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 13;

	// violet spinel
	if (GemIndex[18])
	GemIndex[18]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 14;

	// citrine quartz
	if (GemIndex[19])
	GemIndex[19]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 15;

	// rose quartz
	if (GemIndex[20])
	GemIndex[20]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 16;

	// amber tourmaline
	if (GemIndex[21])
	GemIndex[21]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 17;

	// black tourmaline
	if (GemIndex[22])
	GemIndex[22]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 18;

	// blue tourmaline
	if (GemIndex[23])
	GemIndex[23]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 19;

	// clear tourmaline
	if (GemIndex[24])
	GemIndex[24]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 20;

	// green tourmaline
	if (GemIndex[25])
	GemIndex[25]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 21;

	// pink tourmaline
	if (GemIndex[26])
	GemIndex[26]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 22;

	// amethyst
	if (GemIndex[27])
	GemIndex[27]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 23;

	// bright green chrysoberyl
	if (GemIndex[28])
	GemIndex[28]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 24;

	// golden beryl gem
	if (GemIndex[29])
	GemIndex[29]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 26;

	// fire opal
	if (GemIndex[30])
	GemIndex[30]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 30;

	// lapis lazuli
	if (GemIndex[31])
	GemIndex[31]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 34;

	// green garnet
	if (GemIndex[32])
	GemIndex[32]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 40;

	// aquamarine gem
	if (GemIndex[33])
	GemIndex[33]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 45;

	// turquoise stone
	if (GemIndex[34])
	GemIndex[34]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 48;

	// pink rhosocrosite stone
	if (GemIndex[35])
	GemIndex[35]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 50;

	// deathstone
	if (GemIndex[36])
	GemIndex[36]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 52;

	// dreamstone
	if (GemIndex[37])
	GemIndex[37]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 55;

	// moonstone
	if (GemIndex[38])
	GemIndex[38]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 58;

	// smoky topaz
	if (GemIndex[39])
	GemIndex[39]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 60;

	// pink topaz
	if (GemIndex[40])
	GemIndex[40]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 64;

	// golden topaz
	if (GemIndex[41])
	GemIndex[41]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 65;

	// green malachite
	if (GemIndex[42])
	GemIndex[42]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 68;

	// white opal
	if (GemIndex[43])
	GemIndex[43]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 75;

	// yellow sapphire
	if (GemIndex[44])
	GemIndex[44]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 83;

	// violet sapphire
	if (GemIndex[45])
	GemIndex[45]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 85;

	// black pearl
	if (GemIndex[46])
	GemIndex[46]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 90;

	// pink pearl
	if (GemIndex[47])
	GemIndex[47]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 95;

	// green sapphire
	if (GemIndex[48])
	GemIndex[48]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 110;

	// star sapphire
	if (GemIndex[49])
	GemIndex[49]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 115;

	// star ruby
	if (GemIndex[50])
	GemIndex[50]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 125;

	// sunstone
	if (GemIndex[51])
	GemIndex[51]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 150;

	// starstone
	if (GemIndex[52])
	GemIndex[52]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 160;

	// chameleon agate
	if (GemIndex[53])
	GemIndex[53]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 185;

	// emerald
	if (GemIndex[54])
	GemIndex[54]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 200;

	// rough diamond
	if (GemIndex[55])
	GemIndex[55]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 225;

	// star emerald
	if (GemIndex[56])
	GemIndex[56]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 250;

	// shard of dragonmist crystal
	if (GemIndex[57])
	GemIndex[57]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 275;

	// blue diamond shard
	if (GemIndex[58])
	GemIndex[58]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 300;

	// diamond
	if (GemIndex[59])
	GemIndex[59]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 320;

	// shard of golden firestone
	if (GemIndex[60])
	GemIndex[60]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 320;

	// blue diamond
	if (GemIndex[61])
	GemIndex[61]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 325;

	// swirling colorstone
	if (GemIndex[62])
	GemIndex[62]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 335;

	// redbrim opal
	if (GemIndex[63])
	GemIndex[63]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 330;

	// jet black diamond
	if (GemIndex[64])
	GemIndex[64]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 340;

	// dragonmist crystal
	if (GemIndex[65])
	GemIndex[65]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 340;

	// perfect sphere of shadowglass
	if (GemIndex[66])
	GemIndex[66]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 345;

	// orb of red firestone
	if (GemIndex[67])
	GemIndex[67]->value[OBJECT_GEM_VALUE] = GemIndex[0]->value[OBJECT_GEM_VALUE] * 350;
}
