

#include "area.h"


Area::Area()
{
	AreaSkryptContainer = NULL;
	
	next = prev = next_sort = prev_sort = NULL;
	
	first_reset = last_reset = NULL;
	name = filename = NULL;
	flags = 0;
	status = age = nplayer = reset_frequency = 0;
	
	low_r_vnum = hi_r_vnum = low_o_vnum = hi_o_vnum = low_m_vnum = hi_m_vnum = 0;
	low_soft_range = hi_soft_range = low_hard_range = hi_hard_range = 0;

	author = resetmsg = NULL;
	last_mob_reset = last_obj_reset = NULL;
	
	max_players = 0;
	mkills = mdeaths = pkills = pdeaths = 0;
	gold_looted = illegal_pk = high_economy = low_economy = 0;
	
	// Ksilyan:
	int i;
	for ( i = 0; i < MAX_RANDOM_DESCRIPTION_TYPE + 1; i++)
	{
		random_description_counts[i] = 0;
		
		int j;
		for ( j = 0; j < MAX_RANDOM_DESCRIPTIONS; j++ )
		{
			random_descriptions[i][j] = NULL;
			random_night_descriptions[i][j] = NULL;
		}
	}
	
	secInstallDate = 0;
	
	version = 0;
}

Area::~Area()
{
	// do nothing
}

void Area::skryptSendEvent(const string & eventName, list<Argument*> & arguments)
{
	if ( !this->skryptContainer_ )
		return;

	ArgumentArea argMe(this);
	skryptContainer_->SendEvent(eventName, argMe, arguments, this);
}


const string Area::codeGetClassName()
{
	return "Area";
}

const string Area::codeGetBasicInfo()
{
	return "Area " + string(this->name) + " (" + string(this->filename) + ")";
}
const string Area::codeGetFullInfo()
{
	return "Area " + string(this->name) + " (" + string(this->filename) + ")";
}


