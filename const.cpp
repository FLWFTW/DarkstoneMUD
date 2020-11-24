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
 *			     Mud constants module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

/* undef these at EOF */
#define AM 95
#define AC 95
#define AT 85
#define AW 85
#define AV 95
#define AD 95
#define AR 90
#define AA 95

const int is_inside[SECT_MAX] = {
    1,  /* SECT_INSIDE */
    0,  /* SECT_CITY   */
    0,  /* SECT_FIELD  */
    0,  /* SECT_FOREST */
    0,  /* SECT_HILLS  */
    0,  /* SECT_MOUNTAIN */
    0,  /* SECT_WATER_SWIM */
    0,  /* SECT_WATER_NOSWIM */
    1,  /* SECT_UNDERWATER */
    0,  /* AIR */
    0,  /* DESERT */
    1,  /* DUNNO */
    1,  /* OCEANFLOOR */
    1,  /* UNDERGROUND */
    1,  /* ROADS */
};

/*
 * Race table.
 */
const	struct	race_type	race_table	[MAX_RACE]	=
{
    /*  race name	   DEF_AFF      st dx ws in cn ch lk hp mn re su 	RESTRICTION                                                                         LANGUAGE */
    {
	"Human",		0,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	0,										    LANG_COMMON
    },

    {
	"Elf",	 AFF_DETECT_MAGIC,     -1, 2, 0, 1,-1, 1, 0, -2,10, 0, 0,	0,										    LANG_ELVEN
    },

    {
	"Dwarf",     AFF_INFRARED,	1, 0, 1, 0, 2,-1, 0, 6,-6, 0, 0,	(1 << CLASS_MAGE),								    LANG_DWARVEN
    },

    {
	"Halfling",     AFF_SNEAK,     -2, 1, 0, 0, 1, 0, 0, -3,10, 0, 0,	(1 << CLASS_MAGE) | (1 << CLASS_DRUID) | (1 << CLASS_CLERIC) | (1 << CLASS_AUGURER),			    LANG_HALFLING
    },

    {
	"Pixie",       AFF_FLYING,     -4, 3, 0,-1,-2, 3, 0, -5,40, 0, 0,
					(1 << CLASS_VAMPIRE)
					| (1 << CLASS_WARRIOR)
					| (1 << CLASS_AUGURER),
					LANG_PIXIE
    },

    {
	"_Vampire_",   AFF_INFRARED,	1, 0, 0, 0, 2,-2, 0, 0, 0, 0, 0, 0,												    LANG_COMMON
    },

    /*  race name	   DEF_AFF      st dx ws in cn ch lk hp mn re su 	RESTRICTION									    LANGUAGE */
    {
	"Half-Ogre",   AFF_INFRARED,    2,-3,-1,-2, 3,-4, 0, 5,-8, 0, 0,
					(1 << CLASS_MAGE)
					| (1 << CLASS_CLERIC)
					| (1 << CLASS_DRUID)
					| (1 << CLASS_AUGURER)
					| (1 << CLASS_RANGER) , LANG_OGRE
    },


    {
	"Half-Orc",    AFF_INFRARED,    1,-2,-2,-1, 2,-4, 0, 6,-9, 0, 0,
		  			(1 << CLASS_MAGE)
		  			| (1 << CLASS_CLERIC)
		  			| (1 << CLASS_DRUID)
					| (1 << CLASS_AUGURER)
		  			| (1 << CLASS_RANGER) , LANG_ORCISH
    },
    {/* Name       Afects          Str Dex Wis Int Con Cha Lck Hit Mna Res Sus*/
	"Half-Troll",  AFF_INFRARED,    3, -2, -2,  1,  2, -5,  0,  7, -9,  0,  0,                      /* Class restrictions */
					(1 << CLASS_MAGE)
					| (1 << CLASS_CLERIC)
					| (1 << CLASS_DRUID)
					| (1 << CLASS_RANGER),
/* Language */
					LANG_TROLLISH
    },

    {
	"Half-Elf",    AFF_INFRARED,   -1, 1, 0, 1,-1, 1, 0, 3, 3, 0, 0,
					(1 << CLASS_AUGURER),
					LANG_ELVEN
    },

    {
	"Gith",        AFF_DETECT_INVIS,    1, 2,-1, 1,-2,-5, 1, 4,20, 0, 0,
					(1 << CLASS_DRUID)
					| (1 << CLASS_RANGER),
					LANG_GITH
    },

    {
        "Glandar",     AFF_DETECT_MAGIC,   -3, 0, 1, 2,-1, 0, 0,-1,25, 0, 0,
                                        (1 << CLASS_WARRIOR)
                                        | (1 << CLASS_KNIGHT),
                                        LANG_ELVEN
    },

    {
        "Gnome",       AFF_INFRARED,        0, 0, 0, 3, 2,-2,-2, 1,-5, 0, 0, 0,
                                        LANG_DWARVEN
    },

    {
        "Katrin",      AFF_INFRARED,       -1, 3, 0, 0,-1, 0, 1, 0,-5, 0, 0, 0,
                                        LANG_KATRIN
    },

    {
        "Arayan",        0,                 1, 1, 0, 0, 0,-2, 0, 1,-5, 0, 0, 0,
                                        LANG_COMMON
    },

    {
        "Haran",         0,                 4, 0,-2,-3, 2,-1, 0, 1,-9, 0, 0, 0,
                                        LANG_COMMON
    },

    {
        "Eyan",          0,                -2, 0, 0, 3, 0, 0, 0, 0, 5, 0, 0, 0,
                                        LANG_COMMON
    },

    {
        "Minotaur",      0,                 4,-1,-2,-2, 3,-1, 0, 2,-9, 0, 0,
                                        (1 << CLASS_MAGE)
                                        | (1 << CLASS_BARD)
                                        | (1 << CLASS_DRUID),
                                        LANG_COMMON
    },

    {
        "Eldari",        0,                -1, 0, 1, 1,-2, 1, 0, 0,15, 0, 0, 0,
                                        LANG_COMMON
    },
    {/* Name,    Affects        Str Dex Wis Int Con Char Lck Hit Mna Res, Sus */
        "Troll", AFF_INFRARED,   6, -4, -2,  2,  4, -10,  0, 14, -18,  0,  0,
     /* Class restrictions */
        ~( (1 << CLASS_WARRIOR) | (1 << CLASS_KNIGHT) ),
        LANG_TROLLISH
    }
};


const char *	const	npc_race	[MAX_NPC_RACE] =
{
"human", "elf", "dwarf", "halfling", "pixie", "vampire", "half-ogre",
"half-orc", "half-troll", "half-elf", "gith", "glandar", "gnome", "katrin", "arayan", "haran",
"eyan", "minotaur", "eldari", "troll", "ant", "ape", "baboon", "bat", "bear", "bee",
"beetle", "boar", "bugbear", "cat", "dog", "dragon", "ferret", "fly",
"gargoyle", "gelatin", "ghoul", "gnoll", "gnome", "goblin", "golem",
"gorgon", "harpy", "hobgoblin", "kobold", "lizardman", "locust",
"lycanthrope", "balrog", "mold", "mule", "neanderthal", "ooze", "orc",
"rat", "rustmonster", "shadow", "shapeshifter", "shrew", "shrieker",
"skeleton", "slime", "snake", "spider", "stirge", "thoul", "troglodyte",
"undead", "wight", "wolf", "worm", "zombie", "bovine", "canine", "feline",
"porcine", "mammal", "rodent", "avis", "reptile", "amphibian", "fish",
"crustacean", "insect", "spirit", "magical", "horse", "animal", "humanoid",
"monster", "god", "troll", "ogre"
};


const char *	const	npc_class	[MAX_NPC_CLASS] =
{
"mage", "cleric", "thief", "warrior", "vampire", "druid", "ranger",
"augurer", "gypsy", "bard", "knight", "pc11", "pc12", "pc13", "pc14", "pc15",
"pc16", "pc17", "pc18", "pc19",
"baker", "butcher", "blacksmith", "mayor", "king", "queen"
};

/*
 * Attribute name tables. This is the prototype:
 *
 *       STR       INT       WIS       DEX       CON       CHA       LCK
 * 3-5   Feeble    Brainless Moronic   Maladroit Frail     Monstrous Ill-fated
 * 6-8   Faint     Stupid    Daft      Clumsy    Sickly    Horrible  Adversive
 * 9-11  Weak      Slow      Foolish   Awkward   Delicate  Ugly      Unlucky
 * 12-14 Average   Average   Average   Average   Average   Average   Average
 * 15-17 Strong    Rational  Sensible  Nimble    Sound     Charming  Lucky
 * 18-19 Powerful  Bright    Wise      Agile     Healthy   Suave     Fortunate
 * 20-21 Intense   Shrewd    Sagacious Very Fast Vigorous  Magnetic  Auspicious
 * 22-23 Legendary Legendary Legendary Legendary Legendary Legendary Legendary
 * 24+   Godlike   Godlike   Godlike   Godlike   Godlike   Godlike   Godlike
 *
 * Dexterity:
 * Nimble-Quick, light, or agile in movement or action
 *
 * Luck:
 * Adversity-hardship, luckless
 * Prosperity-luck; good luck, run of luck;
 *
 * 26.06.00 -- Warp
 */

const char* const str_names [9] =
{
   "Feeble", "Faint", "Weak", "Average", "Strong",
   "Powerful", "Intense", "Legendary", "Godlike"
};

const char* const int_names [9] =
{
   "Brainless", "Stupid", "Slow", "Average", "Rational",
   "Bright", "Shrewd", "Legendary", "Godlike"
};

const char* const wis_names [9] =
{
   "Moronic", "Daft", "Foolish", "Average", "Sensible",
   "Wise", "Sagacious", "Legendary", "Godlike"
};

const char* const dex_names [9] =
{
   "Maladroit", "Clumsy", "Awkward", "Average", "Nimble",
   "Agile", "Very Fast", "Legendary", "Godlike"
};

const char* const con_names [9] =
{
   "Sickly", "Frail", "Delicate", "Average", "Sound",
   "Healthy", "Vigorous", "Legendary", "Godlike"
};

const char* const cha_names [9] =
{
   "Monstrous", "Horrible", "Ugly", "Average", "Charming",
   "Suave", "Magnetic", "Legendary", "Godlike"
};

const char* const lck_names [9] =
{
   "Ill-fated", "Adversive", "Unlucky", "Average", "Lucky",
   "Fortunate", "Auspicious", "Legendary", "Godlike"
};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0,  95,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 125, 11 },
    {  0,  0, 140, 12 },
    {  0,  0, 155, 13 }, /* 13  */
    {  0,  1, 170, 14 },
    {  1,  1, 185, 15 }, /* 15  */
    {  1,  2, 200, 16 },
    {  2,  3, 220, 22 },
    {  2,  4, 250, 25 }, /* 18  */
    {  3,  5, 400, 30 },
    {  3,  6, 500, 35 }, /* 20  */
    {  4,  7, 600, 40 },
    {  5,  7, 700, 45 },
    {  6,  8, 800, 50 },
    {  8, 10, 900, 55 },
    { 10, 12, 999, 60 }  /* 25   */
};



const	struct	int_app_type	int_app		[26]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 85 },
    { 99 }	/* 25 */
};



const	struct	wis_app_type	wis_app		[26]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 2 },
    { 2 },	/* 10 */
    { 2 },
    { 2 },
    { 2 },
    { 2 },
    { 3 },	/* 15 */
    { 3 },
    { 4 },
    { 5 },	/* 18 */
    { 5 },
    { 5 },	/* 20 */
    { 6 },
    { 6 },
    { 6 },
    { 6 },
    { 7 }	/* 25 */
};



const	struct	dex_app_type	dex_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};



const	struct	con_app_type	con_app		[26]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 }    /* 25 */
};


const	struct	cha_app_type	cha_app		[26]		=
{
    { - 60 },   /* 0 */
    { - 50 },   /* 1 */
    { - 50 },
    { - 40 },
    { - 30 },
    { - 20 },   /* 5 */
    { - 10 },
    { -  5 },
    { -  1 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    1 },
    {    5 },   /* 15 */
    {   10 },
    {   20 },
    {   30 },
    {   40 },
    {   50 },   /* 20 */
    {   60 },
    {   70 },
    {   80 },
    {   90 },
    {   99 }    /* 25 */
};

/* Have to fix this up - not exactly sure how it works (Scryn) */
const	struct	lck_app_type	lck_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


/*
 * Liquid properties.
 * Used in #OBJECT section of area file.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    { "water",				"clear",				{  0, 1, 10 } },  	/*  0 */
    { "beer",				"amber",				{  3, 2,  5 } },
    { "wine",				"rose",					{  5, 2,  5 } },
    { "ale",				"brown",				{  2, 2,  5 } },
    { "dark ale",			"dark",					{  1, 2,  5 } },

    { "whisky",				"golden",				{  6, 1,  4 } },  	/*  5 */
    { "lemonade",			"pink",					{  0, 1,  8 } },
    { "firebreather",		"boiling",				{ 10, 0,  0 } },
    { "local specialty",	"everclear",			{  3, 3,  3 } },
    { "slime mold juice",	"green",				{  0, 4, -8 } },

    { "milk",				"white",				{  0, 3,  6 } },  	/* 10 */
    { "tea",				"tan",					{  0, 1,  6 } },
    { "coffee",				"black",				{  0, 1,  6 } },
    { "blood",				"red",					{  0, 2, -1 } },
    { "salt water",			"clear",				{  0, 1, -2 } },

    { "cola",				"cherry",				{  0, 1,  5 } },  	/* 15 */
    { "mead",				"honey color",			{  4, 2,  5 } },  	/* 16 */
    { "grog",				"thick brown",			{  3, 2,  5 } },   	/* 17 */
    { "poison",         	"nasty green acidic", 	{  0, 0,  0 } }, 	/* 18 */
    { "ambrosia",       	"golden", 				{ 10, 0, -5 } }		/* 19 */
};

const char *	const	attack_table	[MAX_DAM_TYPES+1] =
{
    "hit",
    "slice",  "stab",    "slash",           "whip",        "claw",
    "blast",  "pound",   "crush",           "grep",        "bite",
    "pierce", "suction", "long arrow",      "short arrow", "bolt",
    "dart",   "stone",   "throwing dagger", "throwing spear"
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n
#define LI LEVEL_IMMORTAL

#undef AM
#undef AC
#undef AT
#undef AW
#undef AV
#undef AD
#undef AR
#undef AA

#undef LI
