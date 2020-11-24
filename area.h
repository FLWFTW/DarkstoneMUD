

#ifndef __AREA_H_
#define __AREA_H_

#include "skrypt/skrypt_public.h"

// Forward declarations
typedef struct reset_data RESET_DATA;

/* Random room stuff */
#define NO_RANDOM_DESCRIPTION   0
#define RANDOM_PLAINS           1
#define RANDOM_FOREST           2
#define RANDOM_HILL             3
#define RANDOM_MOUNTAIN         4
#define RANDOM_DESERT           5
#define RANDOM_SWAMP			6

#define MIN_RANDOM_DESCRIPTION_TYPE 1
#define MAX_RANDOM_DESCRIPTION_TYPE 6
#define MAX_RANDOM_DESCRIPTIONS 6

/*
 * Area definition.
 * Like rooms, areas have their own
 * skrypt containers.
 */
class Area : public cabSkryptable
{
public:
	Area();
	virtual ~Area();

	//////////////////
	// Skrypt Handling
	//////////////////

	SkryptContainer * AreaSkryptContainer; // Skrypt container

	// Send a Skrypt event.
	virtual void skryptSendEvent(const string & eventName, list<Argument*> & arguments);

	////////////////////////
	// Code object interface
	////////////////////////
	virtual const string codeGetClassName(); // Return the name of the class.
	virtual const string codeGetBasicInfo(); // Return a short summary of the object.
	virtual const string codeGetFullInfo(); // Return a complete summary of the object.

	////////////
	// Old stuff
	////////////

    AREA_DATA *		next;
    AREA_DATA *		prev;
    AREA_DATA *		next_sort;
    AREA_DATA *		prev_sort;
    RESET_DATA *	first_reset;
    RESET_DATA *	last_reset;
    char *		name;
    char *		filename;
    int                 flags;
    short              status;  /* h, 8/11 */
    short		age;
    short		nplayer;
    short		reset_frequency;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    int         low_m_vnum;
    int  		hi_m_vnum;
    int			low_soft_range;
    int			hi_soft_range;
    int			low_hard_range;
    int			hi_hard_range;
    char *		author; /* Scryn */
    char *              resetmsg; /* Rennard */
    RESET_DATA *	last_mob_reset;
    RESET_DATA *	last_obj_reset;
    short		max_players;
    int			mkills;
    int			mdeaths;
    int			pkills;
    int			pdeaths;
    int			gold_looted;
    int			illegal_pk;
    int			high_economy;
    int			low_economy;

	// Ksilyan:
    int random_description_counts[MAX_RANDOM_DESCRIPTION_TYPE+1];

	char * random_descriptions[MAX_RANDOM_DESCRIPTION_TYPE + 1][MAX_RANDOM_DESCRIPTIONS];
	char * random_night_descriptions[MAX_RANDOM_DESCRIPTION_TYPE + 1][MAX_RANDOM_DESCRIPTIONS];

	time_t secInstallDate;
   
   int version;
};



#endif // include guard

