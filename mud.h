/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG (C) 1994, 1995, 1996 by Derek Snider                 |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Narn, Scryn, Haus, Swordbearer,   |   / ' ' \   *
 * Altrag, Grishnakh and Tricops                              |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			    Main mud header file			    *
 ****************************************************************************/

#ifndef __MUD_H_
#define __MUD_H_

#include "globals.h" // Ksilyan's property

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifdef unix
	#include <sys/cdefs.h>
	#include <sys/time.h>
#else
	#include <time.h>
	#include <winsock2.h>
#endif

// Shared string types
#include "shared_str.h"


#include "skrypt/skrypt_public.h"

#include "memory.h"
#include "utility.h"

#include <string>
#include <list>
#include <map>

// Lua includes
extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}


using namespace std;

class MemoryManager;
extern list<MemoryManager*> AllocatedMemory;


/* #include <malloc_dbg.h> */

typedef	int				ch_ret;
typedef	int				obj_ret;

long GetMillisecondsTime();

#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 false
#endif

#if	!defined(TRUE)
#define TRUE	 true
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

#if	defined(_AIX)
	#if	!defined(const)
		#define const
	#endif
	typedef int				sh_int;
	typedef int				bool;
	#define unix
#else
	typedef short 			sh_int;
	//typedef unsigned char			bool;
#endif

/*
 * Structure types.
 */
typedef class Account    ACCOUNT_DATA;
typedef struct	affect_data		AFFECT_DATA;
typedef class Area AREA_DATA;
typedef struct  auction_data            AUCTION_DATA; /* auction data */

struct account_ban_data;
typedef struct  account_ban_data ACCOUNT_BAN_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	extracted_char_data	EXTRACT_CHAR_DATA;
typedef class Character CHAR_DATA;
typedef struct	fighting_data		FIGHT_DATA;

// anyone who needs to use descriptors needs to include the
// right file.
//typedef struct	descriptor_data		DESCRIPTOR_DATA;
class PlayerConnection;

typedef struct	help_data		HELP_DATA;
typedef struct	menu_data		MENU_DATA;
typedef class mob_index_data MOB_INDEX_DATA;
typedef struct	comment_data		COMMENT_DATA;
typedef struct	board_data		BOARD_DATA;
typedef class	Object		OBJ_DATA;

class ObjectStorage;

typedef class obj_index_data OBJ_INDEX_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	map_index_data		MAP_INDEX_DATA;   /* maps */
typedef struct	map_data		MAP_DATA;   /* maps */
typedef class Room ROOM_INDEX_DATA;
typedef struct  arena_data              ARENA_DATA; /* Info about the current arena. */
typedef struct  arena_rank_data ARENA_RANK_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct  stable_data     STABLE_DATA;  /* matthew -- the stable shop */
typedef struct	repairshop_data		REPAIR_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	hour_min_sec		HOUR_MIN_SEC;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  tourney_data            TOURNEY_DATA;
typedef struct	mob_prog_data		MPROG_DATA;
typedef struct	mob_prog_act_list	MPROG_ACT_LIST;
typedef	struct	editor_data		EDITOR_DATA;
typedef struct	teleport_data		TELEPORT_DATA;
typedef struct	timer_data		TIMER;
typedef struct  godlist_data		GOD_DATA;
typedef struct	system_data		SYSTEM_DATA;
typedef	struct	smaug_affect		SMAUG_AFF;
typedef struct  who_data                WHO_DATA;
typedef	struct	social_type		SOCIALTYPE;
typedef	struct	cmd_type		CMDTYPE;
typedef	struct	killed_data		KILLED_DATA;
typedef struct  train_data      TRAIN_DATA;
typedef struct  train_list_data TRAIN_LIST_DATA;
typedef struct	wizent			WIZENT;

/* matthew -- structure to hold note animals */
typedef struct  obj_npc_data   OBJ_NPC_DATA;

/* Warp: struct to hold auctio nrooms so we don't need to loop all rooms */
typedef struct  auction_room  AUCTION_ROOM;

/*
 * Function types.
 */
typedef	void	DO_FUN		( CHAR_DATA *ch, const char *argument );
typedef bool	SPEC_FUN	( CHAR_DATA *ch );
typedef ch_ret	SPELL_FUN	( int sn, int level, CHAR_DATA *ch, void *vo );

#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
#define BV32        ((int64)1 << 32) /* only use 32+ for long bvs */
#define BV33        ((int64)1 << 33) /* only use 32+ for long bvs */
#define BV34        ((int64)1 << 34) /* only use 32+ for long bvs */
#define BV35        ((int64)1 << 35) /* only use 32+ for long bvs */
#define BV36        ((int64)1 << 36) /* only use 32+ for long bvs */
#define BV37        ((int64)1 << 37) /* only use 32+ for long bvs */
#define BV38		((int64)1 << 38) /* only use 32+ for long bvs */

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096  /* buf */
//#define MAX_INPUT_LENGTH	 1024  /* arg */
#define MAX_INBUF_SIZE		 1024
#define MAX_BUFFER_LINES         100
#define MAX_BUFFER_COLS          100
#define MAX_BUFFER_SIZE          10000

#define HASHSTR			 /* use string hashing */

#define	MAX_LAYERS		 8	/* maximum clothing layers */
#define MAX_NEST	       100	/* maximum container nesting */

#define MAX_KILLTRACK		25	/* track mob vnums killed */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH	       500000
#define MIN_EXP_WORTH		   20

#define MAX_REXITS		   20	/* Maximum exits allowed in 1 room */
#define MAX_SKILL		  350
#define MAX_CLASS           	   11   /* Increased to 11 for Knight  - Tarin*/
#define MAX_NPC_CLASS		   26
#define MAX_RACE		   25   /* Lots of new races - Tarin*/
#define MAX_NPC_RACE		   94
#define MAX_CLAN		   50
#define MAX_DEITY		   50
#define MAX_CPD			    4   /* Maximum council power level difference */
#define	MAX_HERB		   20

#define MAX_LEVEL		          65
#define LEVEL_LOTD               (MAX_LEVEL -  0)
#define LEVEL_STONE_MASTER       (MAX_LEVEL -  1)
#define LEVEL_STONE_ADEPT        (MAX_LEVEL -  2)
#define LEVEL_STONE_INITIATE     (MAX_LEVEL -  3)
#define LEVEL_STONE_SEEKER       (MAX_LEVEL -  4)
#define LEVEL_ARTIFICER          (MAX_LEVEL -  5)
#define LEVEL_TINKERER           (MAX_LEVEL -  6)
#define LEVEL_CREATOR            (MAX_LEVEL -  7)
#define LEVEL_ENGINEER           (MAX_LEVEL -  8)
#define LEVEL_WANDERER           (MAX_LEVEL -  9)
#define LEVEL_IMMORTAL           (LEVEL_WANDERER)
#define LEVEL_HERO_MAX           (MAX_LEVEL - 10)
#define LEVEL_HERO_MIN           (MAX_LEVEL - 14)

#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK		  (70 * PULSE_PER_SECOND)
#define PULSE_AREA		  (60 * PULSE_PER_SECOND)
#define PULSE_AUCTION             (10 * PULSE_PER_SECOND)
#define PULSE_ASAVE       (1200*PULSE_PER_SECOND)

/*
 * Command logging types.
 */
typedef enum
{
  LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM, LOG_ALL
} log_types;

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
  rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
  rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
  rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
  rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE,
  rAVOIDED, /* Added by Ksilyan to fix small feed bug... */
  rSPELL_ALREADY_AFFECTED, /* Added by Ksilyan for object wear spells */
  rCHAR_AND_OBJ_EXTRACTED = 128,
  rERROR = 255
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2

/* defines for new do_who */
#define WT_MORTAL 0
#define WT_AVATAR 1
#define WT_IMM    2

/*
 * Auction room support structure -- Warp
 */
struct auction_room {
   ROOM_INDEX_DATA* room;
   AUCTION_ROOM* next;
};
/*
 * do_who output structure -- Narn
 */
struct who_data
{
  WHO_DATA *prev;
  WHO_DATA *next;
  char *text;
  char *text2;
  char *text3;
  char *text4;
  char *text5;
  char *text6;
  int  color;
  int  color2;
  int clan_color;
  int council_color;
  int  type;
};

/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    BAN_DATA *	prev;
    char     *	site;
    char     *	ban_time;
};

/*
 * Time and weather stuff.
 */
typedef enum
{
  SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
  SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct hour_min_sec
{
  int hour;
  int min;
  int sec;
  int manual;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
};



/*
 * Structure used to build wizlist
 */
struct	wizent
{
    WIZENT *		next;
    WIZENT *		last;
    char *		name;
    sh_int		level;
};


/*
 * Connected state for a channel.
 */
/*typedef enum {
  CON_GET_EMAIL = -50,
  CON_GET_NEW_EMAIL,
  CON_GET_OLD_PASSWORD,
  CON_GET_MAIN_MENU_CHOICE,
  CON_GET_NEW_PASSWORD,
  CON_CONFIRM_NEW_PASSWORD,
  CON_GET_NEW_ACCOUNT,
  CON_GET_CHAR_NAME,
  CON_GET_NAME,
  CON_CONFIRM_NEW_NAME,
  CON_GET_NEW_SEX,
  CON_GET_NEW_CLASS,
  CON_GET_ADD_NAME,
  CON_READ_MOTD,
  CON_GET_NEW_RACE,
  CON_GET_ADD_PASS,
  CON_CONFIRM_ADD_PASS,
  CON_GET_EMULATION,
  CON_GET_WANT_RIPANSI,
  CON_TITLE,
  CON_PRESS_ENTER,
  CON_WAIT_1,
  CON_WAIT_2,
  CON_WAIT_3,
  CON_ACCEPTED,
  CON_READ_IMOTD,
  CON_COPYOVER_RECOVER,
  CON_DELETE_PROMPT,
  CON_DELETE_PROMPT_2,

  CON_PLAYING = 0,
  CON_EDITING
} connection_types;*/


/*
 * Character substates
 */
typedef enum
{
  SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_OBJ_SHORT, SUB_OBJ_LONG,
  SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC, SUB_ROOM_EXTRA,
  SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, SUB_HELP_EDIT,
  SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD, SUB_RESTRICTED,
  SUB_DEITYDESC, SUB_OBJ_DESC, SUB_AREA_RANDOM,
  SUB_MSKRYPT_EDIT, SUB_OSKRYPT_EDIT, SUB_RSKRYPT_EDIT,
  /* timer types ONLY below this point */
  SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
/*struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	prev;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
    int			port;
    int			descriptor;
    sh_int		connected;
    sh_int		idle;
    sh_int		lines;
    sh_int		scrlen;
    bool		fcommand;
    char		inbuf		[MAX_INBUF_SIZE];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    unsigned long	outsize;
    unsigned int outtop;
    char *		pagebuf;
    unsigned long	pagesize;
    unsigned int			pagetop;
    char *		pagepoint;
    char		pagecmd;
    char		pagecolor;
    int			auth_inc;
    int			auth_state;
    char		abuf[ 256 ];
    int			auth_fd;
    char *		user;
    int 		atimes;
    int			newstate;
    unsigned char	prevcolor;

    ACCOUNT_DATA *account;
	bool    mxp;   // player using MXP flag
};*/



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};

struct	cha_app_type
{
    sh_int	charm;
};

struct  lck_app_type
{
    sh_int	luck;
};

/* the races */
#define RACE_HUMAN	    0
#define RACE_ELF            1
#define RACE_DWARF          2
#define RACE_HALFLING       3
#define RACE_PIXIE          4
#define RACE_VAMPIRE        5
#define RACE_HALF_OGRE      6
#define RACE_HALF_ORC       7
#define RACE_HALF_TROLL     8
#define RACE_HALF_ELF       9
#define RACE_GITH           10
#define RACE_GLANDAR        11
#define RACE_GNOME          12
#define RACE_KATRIN         13
#define RACE_ARAYAN         14
#define RACE_HARAN          15
#define RACE_EYAN           16
#define RACE_MINOTAUR       17
#define RACE_ELDARI         18

/* npc races */
#define	RACE_DRAGON	    31

#define CLASS_NONE	   -1 /* For skill/spells according to guild */
#define CLASS_MAGE	    0
#define CLASS_CLERIC	    1
#define CLASS_THIEF	    2
#define CLASS_WARRIOR	    3
#define CLASS_VAMPIRE	    4
#define CLASS_DRUID	    5
#define CLASS_RANGER	    6
#define CLASS_AUGURER	    7 /* 7-7-96 SB */
#define CLASS_GYPSY         8
#define CLASS_BARD          9
#define CLASS_KNIGHT        10
/*
#define CLASS_WEREWOLF	    8
#define CLASS_LYCANTHROPE   9
#define CLASS_LICH	    10
*/

/*
 * Languages -- Altrag
 */
#define LANG_COMMON      BV00  /* Human base language */
#define LANG_ELVEN       BV01  /* Elven base language */
#define LANG_DWARVEN     BV02  /* Dwarven base language */
#define LANG_PIXIE       BV03  /* Pixie/Fairy base language */
#define LANG_OGRE        BV04  /* Ogre base language */
#define LANG_ORCISH      BV05  /* Orc base language */
#define LANG_TROLLISH    BV06  /* Troll base language */
#define LANG_RODENT      BV07  /* Small mammals */
#define LANG_INSECTOID   BV08  /* Insects */
#define LANG_MAMMAL      BV09  /* Larger mammals */
#define LANG_REPTILE     BV10  /* Small reptiles */
#define LANG_DRAGON      BV11  /* Large reptiles, Dragons */
#define LANG_SPIRITUAL   BV12  /* Necromancers or undeads/spectres */
#define LANG_MAGICAL     BV13  /* Spells maybe?  Magical creatures */
#define LANG_GOBLIN      BV14  /* Goblin base language */
#define LANG_GOD         BV15  /* Clerics possibly?  God creatures */
#define LANG_ANCIENT     BV16  /* Prelude to a glyph read skill? */
#define LANG_HALFLING    BV17  /* Halfling base language */
#define LANG_CLAN	 BV18  /* Clan language */
#define LANG_GITH	 BV19  /* Gith Language */
#define LANG_KATRIN      BV20  /* Katrin Language */
#define LANG_UNKNOWN        0  /* Anything that doesnt fit a category */
#define VALID_LANGS    ( LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_PIXIE  \
		       | LANG_OGRE | LANG_ORCISH | LANG_TROLLISH | LANG_GOBLIN \
		       | LANG_HALFLING | LANG_GITH | LANG_KATRIN )
/* 18 Languages */

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_WORLD        4
#define TO_TARG         5

/*
 * Real action "TYPES" for act.
 */
#define AT_BLACK	    0
#define AT_BLOOD	    1
#define AT_DGREEN           2
#define AT_ORANGE	    3
#define AT_DBLUE	    4
#define AT_PURPLE	    5
#define AT_CYAN	  	    6
#define AT_GREY		    7
#define AT_DGREY	    8
#define AT_RED		    9
#define AT_GREEN	   10
#define AT_YELLOW	   11
#define AT_BLUE		   12
#define AT_PINK		   13
#define AT_LBLUE	   14
#define AT_WHITE	   15
#define AT_BLINK	   16

#define AT_WHITE_BLINK	   AT_WHITE + AT_BLINK
#define AT_RED_BLINK	   AT_RED + AT_BLINK


#define AT_PLAIN	17
#define AT_ACTION	18
#define AT_SAY		19
#define AT_GOSSIP	20
#define AT_YELL		21
#define AT_TELL		22
#define AT_HIT		23
#define AT_HITME	24
#define AT_IMMORT	25
#define AT_HURT		26
#define AT_FALLING	27
#define AT_DANGER	28
#define AT_MAGIC	29
#define AT_CONSIDER	30
#define AT_REPORT	31
#define AT_POISON	32
#define AT_SOCIAL	33
#define AT_DYING	34
#define AT_DEAD		35
#define AT_SKILL	36
#define AT_CARNAGE	37
#define AT_DAMAGE	38
#define AT_FLEE		39
#define AT_RMNAME	40
#define AT_RMDESC	41
#define AT_OBJECT	42
#define AT_PERSON	43
#define AT_LIST		44
#define AT_BYE		45
#define AT_GOLD		46
#define AT_GTELL	47
#define AT_NOTE		48
#define AT_HUNGRY	49
#define AT_THIRSTY	50
#define	AT_FIRE		51
#define AT_SOBER	52
#define AT_WEAROFF	53
#define AT_EXITS	54
#define AT_SCORE	55
#define AT_RESET	56
#define AT_LOG		57
#define AT_DIEMSG	58
#define AT_WARTALK      59
#define AT_ARENA        60
#define AT_MUSE         61
#define AT_THINK        62
#define AT_AVTALK       63
#define AT_OOC          64
#define AT_PRAY         65
#define AT_CLAN         66
#define AT_COUNCIL      67
#define AT_ORDER        68
#define AT_EVTALK       69
#define AT_SGTALK       70
#define AT_NIGHT        71
#define MAX_PCCOLORS    72

extern sh_int const default_set [MAX_PCCOLORS];


#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 33 // base resistance

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA * prev;
    sh_int	level;
    char *	keyword;
    char *	text;
};

struct arena_data
{
   bool         open;
   int          min_level;
   int          max_level;
   char         opened_by[25];
   AREA_DATA*   pArea;
   ROOM_INDEX_DATA * pSRoom;
};

struct arena_rank_data
{
	sh_int		Rank;
	char		Name[MAX_INPUT_LENGTH];
};

/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    SHOP_DATA * prev;			/* Previous shop in list	*/
    int		keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};

struct   train_data
{
  TRAIN_DATA *    next;
  TRAIN_DATA *    prev;
  TRAIN_LIST_DATA *   first_in_list;
  TRAIN_LIST_DATA *   last_in_list;

  int vnum;      /* vnum of the mob */
  int _class;     /* -1 == any */
  int alignment; /* -1 Any, 0 Evil, 1 Neutral, 2 Good */
  int max_level; /* -1 == any */
  int min_level; /* -1 == any */
  int base_cost; /* base cost for a practice (skill cost will be added) */
  int gain_cost; /* How much does it cost to gain a level? */
};

struct   train_list_data
{
   TRAIN_LIST_DATA *    next;
    TRAIN_LIST_DATA *      prev;

    int sn;         /* Contains sn of Skill/Spell, used for strcmp! */

    int     max;        /*  max train percentage for a particular spell */
    int     cost;     /* individual charge to train on this */

};

struct stable_data
{
	STABLE_DATA * next;
	STABLE_DATA * prev;

	int keeper;

	sh_int stable_cost;
	sh_int unstable_cost;

	sh_int close_hour;
	sh_int open_hour;
};

#define MAX_FIX		3
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct	repairshop_data
{
    REPAIR_DATA * next;			/* Next shop in list		*/
    REPAIR_DATA * prev;			/* Previous shop in list	*/
    int		  keeper;		/* Vnum of shop keeper mob	*/
    sh_int	  fix_type [MAX_FIX];	/* Item types shop will fix	*/
    sh_int	  profit_fix;		/* Cost multiplier for fixing	*/
    sh_int	  shop_type;		/* Repair shop type		*/
    sh_int	  open_hour;		/* First opening hour		*/
    sh_int	  close_hour;		/* First closing hour		*/
};


/* Mob program structures */
struct  act_prog_data
{
    struct act_prog_data *next;
    void *vo;
};

struct	mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *	     buf;
    CHAR_DATA *      ch;
    Object *	     obj;
    void *	     vo;
};

struct	mob_prog_data
{
    MPROG_DATA * next;
    int64     type;
    bool	 triggered;
    int		 resetdelay;
    char *	 arglist;
    char *	 comlist;
};

extern bool	MOBtrigger;

/*
 * Per-class stuff.
 */
struct	ClassType
{
	// Constructor
	ClassType();

	shared_str whoName_;		/* Name for 'who'		*/
	sh_int	attr_prime;		/* Prime attribute		*/
	int		weapon;			/* First weapon			*/
	int		guild;			/* Vnum of guild room		*/
	sh_int	skill_adept;		/* Maximum skill level		*/
	sh_int	thac0_00;		/* Thac0 for level  0		*/
	sh_int	thac0_32;		/* Thac0 for level 32		*/
	sh_int	hp_min;			/* Min hp gained on leveling	*/
	sh_int	hp_max;			/* Max hp gained on leveling	*/
	bool	fMana;			/* Class gains mana on level	*/
	sh_int	exp_base;		/* Class base exp		*/
};

/* race dedicated stuff */
struct	race_type
{
    char 	race_name	[16];	/* Race name			*/
    int		affected;		/* Default affect bitvectors	*/
    sh_int	str_plus;		/* Str bonus/penalty		*/
    sh_int	dex_plus;		/* Dex      "			*/
    sh_int	wis_plus;		/* Wis      "			*/
    sh_int	int_plus;		/* Int      "			*/
    sh_int	con_plus;		/* Con      "			*/
    sh_int	cha_plus;		/* Cha      "			*/
    sh_int	lck_plus;		/* Lck 	    "			*/
    sh_int      hit;
    sh_int      mana;
    sh_int      resist;
    sh_int      suscept;
    int		class_restriction;	/* Flags for illegal classes	*/
    int         language;               /* Default racial language      */
};

typedef enum {
CLAN_PLAIN, CLAN_VAMPIRE, CLAN_WARRIOR, CLAN_DRUID, CLAN_MAGE, CLAN_CELTIC,
CLAN_THIEF, CLAN_CLERIC, CLAN_UNDEAD, CLAN_CHAOTIC, CLAN_NEUTRAL, CLAN_LAWFUL,
CLAN_NOKILL, CLAN_ORDER, CLAN_GUILD } clan_types;

typedef enum { GROUP_CLAN, GROUP_COUNCIL, GROUP_GUILD } group_types;


struct	ClanData
{
	// Constructor
	ClanData();

	ClanData * next;		/* next clan in list			*/
	ClanData * prev;		/* previous clan in list		*/
	shared_str	filename_;	/* Clan filename			*/
	shared_str	name_;		/* Clan name				*/
	shared_str	motto_;		/* Clan motto				*/
	shared_str	description_;	/* A brief description of the clan	*/
	shared_str	deity_;		/* Clan's deity				*/
	shared_str	leader_;		/* Head clan leader			*/
	shared_str	number1_;	/* First officer			*/
	shared_str	number2_;	/* Second officer			*/
	int		pkills;		/* Number of pkills on behalf of clan	*/
	int		pdeaths;	/* Number of pkills against clan	*/
	int		mkills;		/* Number of mkills on behalf of clan	*/
	int		mdeaths;	/* Number of clan deaths due to mobs	*/
	int		illegal_pk;	/* Number of illegal pk's by clan	*/
	int		score;		/* Overall score			*/
	sh_int	color;		/* Color for who display */
	sh_int	clan_type;	/* See clan type defines		*/
	sh_int	favour;		/* Deities favour upon the clan		*/
	sh_int	strikes;	/* Number of strikes against the clan	*/
	sh_int	members;	/* Number of clan members		*/
	sh_int	alignment;	/* Clan's general alignment		*/
	int		board;		/* Vnum of clan board			*/
	int		clanobj1;	/* Vnum of first clan obj (ring)	*/
	int		clanobj2;	/* Vnum of second clan obj (shield)	*/
	int		clanobj3;	/* Vnum of third clan obj (weapon)	*/
	int		recall;		/* Vnum of clan's recall room		*/
	int		storeroom;	/* Vnum of clan's store room		*/
	int		guard1;		/* Vnum of clan guard type 1		*/
	int		guard2;		/* Vnum of clan guard type 2		*/
	int		Class;		/* For guilds				*/
};

struct CouncilData
{
	// Constructor
	CouncilData();

    CouncilData * next;	/* next council in list			*/
    CouncilData * prev;	/* previous council in list		*/
    char *	filename;	/* Council filename			*/
    shared_str name_;		/* Council name				*/
    char *	description;	/* A brief description of the council	*/
    shared_str head_;		/* Council head 			*/
    char *	powers;		/* Council powers			*/
    sh_int	members;	/* Number of council members		*/
    int		board;		/* Vnum of council board		*/
    int		meeting;	/* Vnum of council's meeting room	*/
};

struct	DeityData
{
	// Constructor
	DeityData();

    DeityData * next;
    DeityData * prev;
    char *	filename;
    shared_str	name_;
    char *	description;
    sh_int	alignment;
    sh_int	worshippers;
    sh_int	scorpse;
    sh_int	sdeityobj;
    sh_int	savatar;
    sh_int	srecall;
    sh_int	flee;
    sh_int	flee_npcrace;
    sh_int	flee_npcfoe;
    sh_int	kill;
    sh_int	kill_magic;
    sh_int	kill_npcrace;
    sh_int	kill_npcfoe;
    sh_int	sac;
    sh_int	bury_corpse;
    sh_int	aid_spell;
    sh_int	aid;
    sh_int	backstab;
    sh_int	steal;
    sh_int	die;
    sh_int	die_npcrace;
    sh_int	die_npcfoe;
    sh_int	spell_aid;
    sh_int	dig_corpse;
    int		race;
    int		Class;
    int		element;
    int		sex;
    int		avatar;
    int		deityobj;
    int		affected;
    int		npcrace;
    int		npcfoe;
    int		suscept;
};


struct tourney_data
{
    int    open;
    int    low_level;
    int    hi_level;
};


/* matthew */
/* Simple right now */
struct obj_npc_data
{
	int         r_vnum;     /* What vnum will it work at? */
	int         m_vnum;     /* The vnum of the mob */
};

/*
 * Data structure for notes.
 */
struct NoteData
{
	// Constructor
	NoteData();

	NoteData * next;
	NoteData * prev;
	shared_str sender_;
	char *	date;
	char *	to_list;
	char *	subject;
	int         voting;
	char *	yesvotes;
	char *	novotes;
	char *	abstentions;
	char *	text;
};

struct	board_data
{
    BOARD_DATA * next;			/* Next board in list		   */
    BOARD_DATA * prev;			/* Previous board in list	   */
    NoteData *  first_note;		/* First note on board		   */
    NoteData *  last_note;		/* Last note on board		   */
    char *	 note_file;		/* Filename to save notes to	   */
    char *	 read_group;		/* Can restrict a board to a       */
    char *	 post_group;		/* council, clan, guild etc        */
    char *	 extra_readers;		/* Can give read rights to players */
    char *       extra_removers;        /* Can give remove rights to players */
    int		 board_obj;		/* Vnum of board object		   */
    sh_int	 num_posts;		/* Number of notes on this board   */
    sh_int	 min_read_level;	/* Minimum level to read a note	   */
    sh_int	 min_post_level;	/* Minimum level to post a note    */
    sh_int	 min_remove_level;	/* Minimum level to remove a note  */
    sh_int	 max_posts;		/* Maximum amount of notes allowed */
    int          type;                  /* Normal board or mail board? */
};


/* Ksilyan:
 * The memory of spells cast,
 * so that when a player quits
 * the game, they all disappear.
 * HAH HAHA HAH!!!
 */

class SpellMemory
{
public:
	SpellMemory() { Caster = Target = NULL; Spell = NULL; }

	SpellMemory(CHAR_DATA * caster, CHAR_DATA * target, AFFECT_DATA * spell)
		{	Caster = caster; Target = target; Spell = spell;	}

	CHAR_DATA * Caster;
	CHAR_DATA * Target;
	AFFECT_DATA * Spell;
};

/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    AFFECT_DATA *	prev;
    sh_int		type;
    int                 duration;
    sh_int		location;
    int			modifier;
    int			bitvector;
    sh_int      level;
};


/*
 * A SMAUG spell
 */
struct	smaug_affect
{
    SMAUG_AFF *		next;
    char *		duration;
    sh_int		location;
    char *		modifier;
    int			bitvector;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   		445
#define MOB_VNUM_VAMPIRE	   		593
#define MOB_VNUM_ANIMATED_CORPSE   	5
#define MOB_VNUM_POLY_WOLF	   		10
#define MOB_VNUM_POLY_MIST	   		11
#define MOB_VNUM_POLY_BAT	   		12
#define MOB_VNUM_POLY_HAWK	   		13
#define MOB_VNUM_POLY_CAT	   		14
#define MOB_VNUM_POLY_DOVE	   		15
#define MOB_VNUM_POLY_FISH	   		16
#define MOB_VNUM_HORSE         		515

#define SOCIAL_AGGRESSIVE   		BV00
#define SOCIAL_SEX          		BV01
#define SOCIAL_SHOCKING     		BV02
#define SOCIAL_FRIENDLY     		BV03
#define SOCIAL_MALE_ONLY    		BV04
#define SOCIAL_FEMALE_ONLY  		BV05
#define SOCIAL_NEUTER_ONLY  		BV06
#define SOCIAL_GOOD_ONLY    		BV07
#define SOCIAL_EVIL_ONLY    		BV08
#define SOCIAL_NEUTRAL_ONLY 		BV09
#define SOCIAL_REVULSION    		BV10
#define SOCIAL_MOB_ONLY     		BV11
#define SOCIAL_PC_ONLY      		BV12
#define SOCIAL_SLEEPABLE    		BV13
#define SOCIAL_IMMORTAL     		BV14
#define SOCIAL_GLOBAL       		BV15
#define SOCIAL_LOG          		BV16
#define SOCIAL_GLOBALECHO   		BV17
#define SOCIAL_COMMUNICABLE 		BV18
#define MAX_SOCIAL_TYPES    		19


/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		 			BV00		/* Auto set for mobs	*/
#define ACT_SENTINEL				BV01		/* Stays in one room	*/
#define ACT_SCAVENGER		 		BV02		/* Picks up objects	*/
#define ACT_BANKER					BV03	/* Banker mob */
#define ACT_STAYSECTOR       		BV04 // will wander, but not to a room with different sector
#define ACT_AGGRESSIVE		 		BV05		/* Attacks PC's		*/
#define ACT_STAY_AREA		 		BV06		/* Won't leave area	*/
#define ACT_WIMPY		 			BV07		/* Flees when hurt	*/
#define ACT_PET			 			BV08		/* Auto set for pets	*/
#define ACT_TRAIN		 			BV09		/* Mob does not award EXP	*/
#define ACT_PRACTICE				BV10		/* Can practice PC's	*/
#define ACT_IMMORTAL		 		BV11		/* Cannot be killed	*/
#define ACT_INVISIBLE_TO_PLAYERS 	BV12   /* Cannot be seen by non-imms  */
#define ACT_POLYSELF		 		BV13
#define ACT_SAFE             		BV14       // Cannot be attacked
#define ACT_GUARDIAN		 		BV15		// Protects master
#define ACT_RUNNING		 			BV16		/* Hunts quickly	*/
#define ACT_NOWANDER		 		BV17		/* Doesn't wander	*/
#define ACT_MOUNTABLE		 		BV18		/* Can be mounted	*/
#define ACT_MOUNTED		 			BV19		/* Is mounted		*/
#define ACT_SCHOLAR              	BV20           /* Can teach languages  */
#define ACT_SECRETIVE		 		BV21		/* actions aren't seen	*/
#define ACT_POLYMORPHED		 		BV22		/* Mob is a ch		*/
#define ACT_MOBINVIS		 		BV23		/* Like wizinvis	*/
#define ACT_NOASSIST		 		BV24		/* Doesn't assist mobs	*/
#define ACT_SUV              		BV25
#define ACT_NOASTRAL         		BV26       // players cannot astral to this target
#define ACT_NOREGEN          		BV27       /* Mob does not regen HP */
#define ACT_GEMDEALER        		BV28       /* Mob will buy/sell gems */
#define ACT_NOGEMDROP        		BV29       /* Mob will not drop gems */
#define ACT_PROTOTYPE		 		BV30		/* A prototype mob	*/
#define ACT_INANIMATE        		BV31
/*
 * The inanimate mob is a "fake" mob - basically,
 * it is an object with hitpoints. This means that
 * it cannot fight back, it does not "die" (it is
 * destroyed), it cannot be healed, and is not
 * affected by spells that normally affect living
 * creatures. The reason we needed this flag was to
 * create a door "mob".
 *      -Ksilyan
 */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		  			BV00
#define AFF_INVISIBLE		 	 	BV01
#define AFF_DETECT_EVIL		  		BV02
#define AFF_DETECT_INVIS	  		BV03
#define AFF_DETECT_MAGIC	  		BV04
#define AFF_DETECT_HIDDEN	  		BV05
#define AFF_EARTHMELD		  		BV06		/* Unused	*/
#define AFF_SANCTUARY		  		BV07
#define AFF_FAERIE_FIRE		  		BV08
#define AFF_INFRARED		  		BV09
#define AFF_CURSE		  			BV10
#define AFF_ROCKMELD		 		BV11		/* Unused	*/
#define AFF_POISON		  			BV12
#define AFF_PROTECT		  			BV13
#define AFF_PARALYSIS				BV14
#define AFF_SNEAK		  			BV15
#define AFF_HIDE		  			BV16
#define AFF_SLEEP		  			BV17
#define AFF_CHARM		  			BV18
#define AFF_FLYING		  			BV19
#define AFF_PASS_DOOR			 	BV20
#define AFF_FLOATING		  		BV21
#define AFF_TRUESIGHT		  		BV22
#define AFF_DETECTTRAPS		  		BV23
#define AFF_SCRYING	          		BV24
#define AFF_FIRESHIELD	          	BV25
#define AFF_SHOCKSHIELD	          	BV26
#define AFF_HAUS1                 	BV27
#define AFF_ICESHIELD  		  		BV28
#define AFF_POSSESS		  			BV29
#define AFF_BERSERK		  			BV30
#define AFF_AQUA_BREATH		  		BV31

/* 31 aff's (1 left.. :P) */
/* make that none - ugh - time for another field? :P */
/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  			BV00
#define RIS_COLD		  			BV01
#define RIS_ELECTRICITY		  		BV02
#define RIS_ENERGY		  			BV03
#define RIS_BLUNT		  			BV04
#define RIS_PIERCE		  			BV05
#define RIS_SLASH		  			BV06
#define RIS_ACID		  			BV07
#define RIS_POISON		  			BV08
#define RIS_DRAIN		  			BV09
#define RIS_SLEEP		  			BV10
#define RIS_CHARM		  			BV11
#define RIS_HOLD		  			BV12
#define RIS_NONMAGIC				BV13
#define RIS_PLUS1		  			BV14
#define RIS_PLUS2		  			BV15
#define RIS_PLUS3		  			BV16
#define RIS_PLUS4		  			BV17
#define RIS_PLUS5		  			BV18
#define RIS_PLUS6		  			BV19
#define RIS_MAGIC		  			BV20
#define RIS_PARALYSIS				BV21
/* 21 RIS's*/

/*
 * Attack types
 */
#define ATCK_BITE		  			BV00
#define ATCK_CLAWS		  			BV01
#define ATCK_TAIL		  			BV02
#define ATCK_STING		  			BV03
#define ATCK_PUNCH		  			BV04
#define ATCK_KICK		  			BV05
#define ATCK_TRIP		  			BV06
#define ATCK_BASH		  			BV07
#define ATCK_STUN		  			BV08
#define ATCK_GOUGE		  			BV09
#define ATCK_BACKSTAB		  		BV10
#define ATCK_FEED		  			BV11
#define ATCK_DRAIN		  			BV12
#define ATCK_FIREBREATH		  		BV13
#define ATCK_FROSTBREATH	  		BV14
#define ATCK_ACIDBREATH		  		BV15
#define ATCK_LIGHTNBREATH	  		BV16
#define ATCK_GASBREATH		  		BV17
#define ATCK_POISON		  			BV18
#define ATCK_NASTYPOISON	  		BV19
#define ATCK_GAZE		  			BV20
#define ATCK_BLINDNESS		  		BV21
#define ATCK_CAUSESERIOUS	  		BV22
#define ATCK_EARTHQUAKE		  		BV23
#define ATCK_CAUSECRITICAL	  		BV24
#define ATCK_CURSE		  			BV25
#define ATCK_FLAMESTRIKE	  		BV26
#define ATCK_HARM		  			BV27
#define ATCK_FIREBALL		  		BV28
#define ATCK_COLORSPRAY		  		BV29
#define ATCK_WEAKEN		  			BV30
#define ATCK_SPIRALBLAST	  		BV31
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * Defense types
 */
#define DFND_PARRY		  			BV00
#define DFND_DODGE		  			BV01
#define DFND_HEAL		  			BV02
#define DFND_CURELIGHT		  		BV03
#define DFND_CURESERIOUS	  		BV04
#define DFND_CURECRITICAL	  		BV05
#define DFND_DISPELMAGIC	  		BV06
#define DFND_DISPELEVIL		  		BV07
#define DFND_SANCTUARY		  		BV08
#define DFND_FIRESHIELD		  		BV09
#define DFND_SHOCKSHIELD	  		BV10
#define DFND_SHIELD		  			BV11
#define DFND_BLESS		  			BV12
#define DFND_STONESKIN				BV13
#define DFND_TELEPORT		  		BV14
#define DFND_MONSUM1		  		BV15
#define DFND_MONSUM2		  		BV16
#define DFND_MONSUM3		  		BV17
#define DFND_MONSUM4		  		BV18
#define DFND_DISARM		  			BV19
#define DFND_ICESHIELD 		  		BV20
#define DFND_GRIP		  			BV21
/* 21 def's */

/*
 * Body parts
 */
#define PART_HEAD		  			BV00
#define PART_ARMS		  			BV01
#define PART_LEGS		  			BV02
#define PART_HEART		  			BV03
#define PART_BRAINS		  			BV04
#define PART_GUTS		  			BV05
#define PART_HANDS		  			BV06
#define PART_FEET		  			BV07
#define PART_FINGERS				BV08
#define PART_EAR		  			BV09
#define PART_EYE		  			BV10
#define PART_LONG_TONGUE		  	BV11
#define PART_EYESTALKS		  		BV12
#define PART_TENTACLES		  		BV13
#define PART_FINS		  			BV14
#define PART_WINGS		  			BV15
#define PART_TAIL		  			BV16
#define PART_SCALES		  			BV17
/* for combat */
#define PART_CLAWS		  			BV18
#define PART_FANGS		  			BV19
#define PART_HORNS		  			BV20
#define PART_TUSKS		  			BV21
#define PART_TAILATTACK		  		BV22
#define PART_SHARPSCALES	  		BV23
#define PART_BEAK		  			BV24

#define PART_HAUNCH		  			BV25
#define PART_HOOVES		  			BV26
#define PART_PAWS		  			BV27
#define PART_FORELEGS		  		BV28
#define PART_FEATHERS		  		BV29

/*
 * Autosave flags
 */
#define SV_DEATH		  			BV00
#define SV_KILL			  			BV01
#define SV_PASSCHG		  			BV02
#define SV_DROP			  			BV03
#define SV_PUT			  			BV04
#define SV_GIVE			  			BV05
#define SV_AUTO			  			BV06
#define SV_ZAPDROP		  			BV07
#define SV_AUCTION		  			BV08
#define SV_GET			  			BV09
#define SV_RECEIVE		  			BV10
#define SV_IDLE			  			BV11
#define SV_BACKUP		  			BV12

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  			BV01
#define PIPE_LIT		  			BV02
#define PIPE_HOT		  			BV03
#define PIPE_DIRTY		  			BV04
#define PIPE_FILTHY		  			BV05
#define PIPE_GOINGOUT				BV06
#define PIPE_BURNT		  			BV07
#define PIPE_FULLOFASH				BV08

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		  			BV11
#define SF_EARTH		  			BV12
#define SF_AIR			  			BV13
#define SF_ASTRAL		  			BV14
#define SF_AREA			  			BV15  /* is an area spell		*/
#define SF_DISTANT		  			BV16  /* affects something far away	*/
#define SF_REVERSE		  			BV17
#define SF_SAVE_HALF_DAMAGE	  		BV18  /* save for half damage		*/
#define SF_SAVE_NEGATES		  		BV19  /* save negates affect		*/
#define SF_ACCUMULATIVE		  		BV20  /* is accumulative		*/
#define SF_RECASTABLE		  		BV21  /* can be refreshed		*/
#define SF_NOSCRIBE		  			BV22  /* cannot be scribed		*/
#define SF_NOBREW		  			BV23  /* cannot be brewed		*/
#define SF_GROUPSPELL		  		BV24  /* only affects group members	*/
#define SF_OBJECT		  			BV25	/* directed at an object	*/
#define SF_CHARACTER		  		BV26  /* directed at a character	*/
#define SF_SECRETSKILL		  		BV27	/* hidden unless learned	*/

#if 0 /* Testaur - eliminate PKSENSITIVE flag */
#define SF_PKSENSITIVE            	BV28  /* more difficult for pvp       */
#endif
#define SF_PERSISTANT		  		BV28	/* Not easily dispellable	*/

#define SF_STOPONFAIL		  		BV29	/* stops spell on first failure */
#define SF_SPECIALTRAINERS        	BV30  /* only practicable at trainers that specify it */

typedef enum { SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
	       SS_BREATH, SS_SPELL_STAFF } save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)

typedef enum {
    DAM_HIT,    DAM_SLICE,  DAM_STAB,    DAM_SLASH,   DAM_WHIP,
    DAM_CLAW,   DAM_BLAST,  DAM_POUND,   DAM_CRUSH,   DAM_GREP,
    DAM_BITE,   DAM_PIERCE, DAM_SUCTION, DAM_LARROW,  DAM_SARROW,
    DAM_BOLT,   DAM_DART,   DAM_STONE,   DAM_TDAGGER, DAM_TSPEAR
} damage_types;

#define MAX_DAM_TYPES        DAM_TSPEAR
#define FIRST_PROJECTILE_DAM DAM_LARROW

typedef enum { SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
	       SD_POISON, SD_DRAIN } spell_dam_types;

typedef enum { SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
	       SA_DIVINATE, SA_OBSCURE, SA_CHANGE } spell_act_types;

typedef enum { SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum { SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
	       SC_LIFE, SC_DEATH, SC_ILLUSION } spell_class_types;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum { SEX_NEUTRAL, SEX_MALE, SEX_FEMALE } sex_types;

typedef enum {
  TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART,    TRAP_TYPE_POISON_NEEDLE,
  TRAP_TYPE_POISON_DAGGER,  TRAP_TYPE_POISON_ARROW,   TRAP_TYPE_BLINDNESS_GAS,
  TRAP_TYPE_SLEEPING_GAS,   TRAP_TYPE_FLAME,	      TRAP_TYPE_EXPLOSION,
  TRAP_TYPE_ACID_SPRAY,	    TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
  TRAP_TYPE_SEX_CHANGE } trap_types;

#define MAX_TRAPTYPE		   		TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   		BV00
#define TRAP_OBJ	      	   		BV01
#define TRAP_ENTER_ROOM		   		BV02
#define TRAP_LEAVE_ROOM		   		BV03
#define TRAP_OPEN		   			BV04
#define TRAP_CLOSE		   			BV05
#define TRAP_GET		   			BV06
#define TRAP_PUT		   			BV07
#define TRAP_PICK		   			BV08
#define TRAP_UNLOCK		   			BV09
#define TRAP_N			   			BV10
#define TRAP_S			   			BV11
#define TRAP_E	      		   		BV12
#define TRAP_W	      		   		BV13
#define TRAP_U	      		   		BV14
#define TRAP_D	      		   		BV15
#define TRAP_EXAMINE		   		BV16
#define TRAP_NE			   			BV17
#define TRAP_NW			   			BV18
#define TRAP_SE			   			BV19
#define TRAP_SW			   			BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      	2
#define OBJ_VNUM_MONEY_SOME	      	3

#define OBJ_VNUM_CORPSE_NPC	     	10
#define OBJ_VNUM_CORPSE_PC	     	11
#define OBJ_VNUM_SEVERED_HEAD	    12
#define OBJ_VNUM_TORN_HEART	     	13
#define OBJ_VNUM_SLICED_ARM	     	14
#define OBJ_VNUM_SLICED_LEG	     	15
#define OBJ_VNUM_SPILLED_GUTS	    16
#define OBJ_VNUM_BLOOD		     	17
#define OBJ_VNUM_BLOODSTAIN	     	18
#define OBJ_VNUM_SCRAPS		     	19
#define OBJ_VNUM_MUSHROOM	     	20
#define OBJ_VNUM_LIGHT_BALL	     	21
#define OBJ_VNUM_SPRING		     	22

#define OBJ_VNUM_SLICE		     	24
#define OBJ_VNUM_SHOPPING_BAG	    25

#define OBJ_VNUM_MAGIC_FIRE		 	30
#define OBJ_VNUM_TRAP		     	31
#define OBJ_VNUM_PORTAL		     	32
#define OBJ_VNUM_BLACK_POWDER	    33
#define OBJ_VNUM_SCROLL_SCRIBING    34
#define OBJ_VNUM_FLASK_BREWING      35
#define OBJ_VNUM_NOTE		     	36

#define OBJ_VNUM_FIRE            	39

/* Academy eq */
#define OBJ_VNUM_NEWBIE_DAGGER    	4600
#define OBJ_VNUM_NEWBIE_SHIELD    	4601
#define OBJ_VNUM_NEWBIE_BOOTS     	4602
#define OBJ_VNUM_NEWBIE_GLOVES    	4603

#define OBJ_VNUM_SCROLL_SCRIBING2   8017

/* Ores - Zoie */
#define OBJ_VNUM_TIN_ORE    		11171
#define OBJ_VNUM_COPPER_ORE    		11172
#define OBJ_VNUM_IRON_ORE    		11173
#define OBJ_VNUM_LEAD_ORE    		11174
#define OBJ_VNUM_SILVER_ORE    		11175
#define OBJ_VNUM_GOLD_ORE    		11176
#define OBJ_VNUM_MITHRIL_ORE    	11177

/* Ingots - Zoie */

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
  ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
  ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION, ITEM_WORN, ITEM_FURNITURE,
  ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER, ITEM_NOTE, ITEM_DRINK_CON,
  ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN, ITEM_BOAT,
  ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL, ITEM_BLOOD,
  ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON, ITEM_HERB,
  ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
  ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_COMPONENT, ITEM_MATCH,
  ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER, ITEM_TINDER,
  ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
  ITEM_PROJECTILE, ITEM_QUIVER, ITEM_SCABBARD, ITEM_SHOVEL, ITEM_SALVE,
  ITEM_MEAT, ITEM_GEM, ITEM_ORE
} item_types;


#define MAX_ITEM_TYPE		     	ITEM_ORE

typedef enum
{
OLD_ITEM_NONE, OLD_ITEM_LIGHT, OLD_ITEM_SCROLL, OLD_ITEM_WAND, OLD_ITEM_STAFF, OLD_ITEM_WEAPON,
OLD_ITEM_FIREWEAPON, OLD_ITEM_MISSILE, OLD_ITEM_TREASURE, OLD_ITEM_ARMOR, OLD_ITEM_POTION,
OLD_ITEM_WORN, OLD_ITEM_FURNITURE, OLD_ITEM_TRASH, OLD_ITEM_OLDTRAP, OLD_ITEM_CONTAINER,
OLD_ITEM_NOTE, OLD_ITEM_DRINK_CON, OLD_ITEM_KEY, OLD_ITEM_FOOD, OLD_ITEM_MONEY, OLD_ITEM_PEN,
OLD_ITEM_BOAT, OLD_ITEM_CORPSE_NPC, OLD_ITEM_CORPSE_PC, OLD_ITEM_FOUNTAIN, OLD_ITEM_PILL,
OLD_ITEM_BLOOD, OLD_ITEM_BLOODSTAIN, OLD_ITEM_SCRAPS, OLD_ITEM_PIPE, OLD_ITEM_HERB_CON,
OLD_ITEM_HERB, OLD_ITEM_INCENSE, OLD_ITEM_FIRE, OLD_ITEM_BOOK, OLD_ITEM_SWITCH, OLD_ITEM_LEVER,
OLD_ITEM_PULLCHAIN, OLD_ITEM_BUTTON, OLD_ITEM_DIAL, OLD_ITEM_RUNE, OLD_ITEM_RUNEPOUCH,
OLD_ITEM_MATCH, OLD_ITEM_TRAP, OLD_ITEM_MAP, OLD_ITEM_PORTAL, OLD_ITEM_PAPER,
OLD_ITEM_TINDER, OLD_ITEM_LOCKPICK, OLD_ITEM_SPIKE, OLD_ITEM_DISEASE, OLD_ITEM_OIL, OLD_ITEM_FUEL,
OLD_ITEM_SHORT_BOW, OLD_ITEM_LONG_BOW, OLD_ITEM_CROSSBOW, OLD_ITEM_PROJECTILE, OLD_ITEM_QUIVER,
OLD_ITEM_SHOVEL, OLD_ITEM_SALVE, OLD_ITEM_MEAT, OLD_ITEM_MISSILE_WEAPON
} old_item_types;

/* How cooked is the food? */
#define COOKED_DRIPPING 			0 /* Dripping Blood */
#define COOKED_RAW      			1 /* Some blood */
#define COOKED_RARE     			2 /* Edible, but slight mentalstate mix */
#define COOKED_MEDIUM   			3 /* Normal, no problems eating it */
#define COOKED_WELLDONE 			4 /* Yum, Yum */
#define COOKED_CHARED   			5 /* Na, you can't eat that... */



/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW					BV00
#define ITEM_HUM					BV01
#define ITEM_DARK					BV02
#define ITEM_LOYAL					BV03
#define ITEM_EVIL					BV04
#define ITEM_INVIS					BV05
#define ITEM_MAGIC					BV06
#define ITEM_NODROP					BV07
#define ITEM_BLESS					BV08
#define ITEM_ANTI_GOOD				BV09
#define ITEM_ANTI_EVIL				BV10
#define ITEM_ANTI_NEUTRAL			BV11
#define ITEM_NOREMOVE				BV12
#define ITEM_INVENTORY				BV13
#define ITEM_ANTI_MAGE				BV14
#define ITEM_ANTI_THIEF	    		BV15
#define ITEM_ANTI_WARRIOR			BV16
#define ITEM_ANTI_CLERIC			BV17
#define ITEM_ORGANIC				BV18
#define ITEM_METAL					BV19
#define ITEM_DONATION				BV20
#define ITEM_CLANOBJECT				BV21
#define ITEM_CLANCORPSE				BV22
#define ITEM_ANTI_VAMPIRE			BV23
#define ITEM_ANTI_DRUID	       		BV24
#define ITEM_HIDDEN					BV25
#define ITEM_POISONED				BV26
#define ITEM_COVERING				BV27
#define ITEM_DEATHROT				BV28
#define ITEM_BURIED       			BV29    /* item is underground */
#define ITEM_PROTOTYPE				BV30
#define ITEM_DEITY          		BV31    /* damnit -- no more */

#define MAX_O_FLAGS 				31

/*
 * KSILYAN
 * Extra flags (2).
 * Needed more flags for items...
 */
#define ITEM_INVISIBLE_TO_PLAYERS	BV00 // an item that is invis to players, but visible to avs and Imms
#define ITEM_COMPONENT_CONTAINER	BV01 // this is for "instant access" component containers- pouches, cloaks, etc.
#define ITEM_COMMUNICATION_DEVICE	BV02 // this, when worn, allows player to be on comm network.
#define ITEM_NEVER_BREAKS 			BV03 // this item will never reduce to scraps, nor have its max condition go down.
#define ITEM_NO_LOCATE 				BV04 // this item cannot be located
#define ITEM_RARE 					BV05 // This item is a rare

#define MAX_O_FLAGS_2 				5

/* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING				BV00
#define ITEM_BACKSTABBER  			BV01
#define ITEM_BANE					BV02
#define ITEM_LOYAL					BV03
#define ITEM_HASTE					BV04
#define ITEM_DRAIN					BV05
#define ITEM_LIGHTNING_BLADE  		BV06

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP						BV00
#define TRIG_UNLOCK					BV01
#define TRIG_LOCK					BV02
#define TRIG_D_NORTH				BV03
#define TRIG_D_SOUTH				BV04
#define TRIG_D_EAST					BV05
#define TRIG_D_WEST					BV06
#define TRIG_D_UP					BV07
#define TRIG_D_DOWN					BV08
#define TRIG_DOOR					BV09
#define TRIG_CONTAINER				BV10
#define TRIG_OPEN					BV11
#define TRIG_CLOSE					BV12
#define TRIG_PASSAGE				BV13
#define TRIG_OLOAD					BV14
#define TRIG_MLOAD					BV15
#define TRIG_TELEPORT				BV16
#define TRIG_TELEPORTALL			BV17
#define TRIG_TELEPORTPLUS			BV18
#define TRIG_DEATH					BV19
#define TRIG_CAST					BV20
#define TRIG_FAKEBLADE				BV21
#define TRIG_RAND4					BV22
#define TRIG_RAND6					BV23
#define TRIG_TRAPDOOR				BV24
#define TRIG_ANOTHEROOM				BV25
#define TRIG_USEDIAL				BV26
#define TRIG_ABSOLUTEVNUM			BV27
#define TRIG_SHOWROOMDESC			BV28
#define TRIG_AUTORETURN				BV29

#define TELE_SHOWDESC				BV00
#define TELE_TRANSALL				BV01
#define TELE_TRANSALLPLUS			BV02


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE					BV00
#define ITEM_WEAR_FINGER			BV01
#define ITEM_WEAR_NECK				BV02
#define ITEM_WEAR_BODY				BV03
#define ITEM_WEAR_HEAD				BV04
#define ITEM_WEAR_LEGS				BV05
#define ITEM_WEAR_FEET				BV06
#define ITEM_WEAR_HANDS				BV07
#define ITEM_WEAR_ARMS				BV08
#define ITEM_WEAR_SHIELD			BV09
#define ITEM_WEAR_ABOUT				BV10
#define ITEM_WEAR_WAIST				BV11
#define ITEM_WEAR_WRIST				BV12
#define ITEM_WIELD					BV13
#define ITEM_HOLD					BV14
#define ITEM_DUAL_WIELD				BV15
#define ITEM_WEAR_EARS				BV16
#define ITEM_WEAR_CAPE				BV17
#define ITEM_UNUSED_WEAR_MISSILE_WIELD	BV18
#define ITEM_WEAR_SHOULDERS       	BV19
#define	ITEM_WEAR_QUIVER    		BV20
#define ITEM_WEAR_SCABBARD  		BV21




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
  APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
  APPLY_SEX, APPLY_CLASS, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
  APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
  APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
  APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
  APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
  APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
  APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP, APPLY_DODGE,
  APPLY_PEEK, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT, APPLY_DISARM,
  APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH, APPLY_CLIMB,
  APPLY_GRIP, APPLY_SCRIBE, APPLY_BREW, APPLY_WEARSPELL, APPLY_REMOVESPELL,
  APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE, APPLY_DIG,
  APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, APPLY_CONTAGION,
  MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   		1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		  		1
#define CONT_PICKPROOF		  		2
#define CONT_CLOSED		      		4
#define CONT_LOCKED		      		8

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      	2
#define ROOM_VNUM_POLY		      	3
#define ROOM_VNUM_DONATION_TRASH   	385
#define ROOM_VNUM_DONATION_FOOD    	383
#define ROOM_VNUM_DONATION_WEAPONS 	381
#define ROOM_VNUM_DONATION_ARMOR   	382
#define ROOM_VNUM_DONATION_MISC    	386
#define ROOM_VNUM_DONATION_MAGICAL	384

/* matthew */
#define ROOM_VNUM_MOUNT_STORAGE   	4

#define ROOM_VNUM_CHAT		   		1200
#define ROOM_VNUM_TEMPLE     		478
#define ROOM_VNUM_ALTAR		    	478
#define ROOM_VNUM_SCHOOL	   		4600
#define ROOM_AUTH_START		    	100

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

#define ROOM_DARK					BV00
#define ROOM_DEATH					BV01
#define ROOM_NO_MOB					BV02
#define ROOM_INDOORS				BV03
#define ROOM_LAWFUL					BV04
#define ROOM_NEUTRAL				BV05
#define ROOM_CHAOTIC				BV06
#define ROOM_NO_MAGIC				BV07
#define ROOM_TUNNEL					BV08
#define ROOM_PRIVATE				BV09
#define ROOM_SAFE					BV10
#define ROOM_SOLITARY				BV11
#define ROOM_PET_SHOP				BV12
#define ROOM_NO_RECALL				BV13
#define ROOM_DONATION				BV14
#define ROOM_NODROPALL				BV15
#define ROOM_SILENCE				BV16
#define ROOM_LOGSPEECH				BV17
#define ROOM_NODROP					BV18
#define ROOM_CLANSTOREROOM			BV19
#define ROOM_NO_SUMMON				BV20
#define ROOM_NO_ASTRAL				BV21
#define ROOM_TELEPORT				BV22
#define ROOM_TELESHOWDESC			BV23
#define ROOM_NOFLOOR				BV24
#define ROOM_STOREITEMS         	BV25
#define ROOM_NUKE               	BV26
#define ROOM_ONEPCONLY          	BV27
#define ROOM_AUCTIONROOM        	BV28 /* Warp */
#define ROOM_FORGE					BV29
#define ROOM_PROTOTYPE          	BV30
#define ROOM_ALWAYS_LIGHT               BV31 



/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
  DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
  DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define MAX_DIR						DIR_SOUTHWEST	/* max for normal walking */
#define DIR_PORTAL					DIR_SOMEWHERE	/* portal direction	  */


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		  			BV00
#define EX_CLOSED		  			BV01
#define EX_LOCKED		  			BV02
#define EX_SECRET		  			BV03
#define EX_SWIM			  			BV04
#define EX_PICKPROOF		  		BV05
#define EX_FLY			  			BV06
#define EX_CLIMB		  			BV07
#define EX_DIG			  			BV08
#define EX_NOMOBWANDER				BV09
#define EX_NOPASSDOOR		  		BV10	/* KSILYAN- I don't think so*/
#define EX_HIDDEN		  			BV11
#define EX_PASSAGE		  			BV12 	/* for mp open/close passage */
#define EX_PORTAL 		  			BV13
#define EX_BARRICADE	  			BV14 	/* can be completely barricaded by one person */
#define EX_RES3			  			BV15
#define EX_xCLIMB		  			BV16
#define EX_xENTER		  			BV17
#define EX_xLEAVE		  			BV18
#define EX_xAUTO		  			BV19
#define EX_RES4	  		  			BV20
#define EX_xSEARCHABLE				BV21
#define EX_BASHED         	        BV22
#define EX_BASHPROOF              	BV23
#define EX_NOMOB		  			BV24
#define EX_WINDOW		  			BV25
#define EX_xLOOK		  			BV26
#define MAX_EXFLAG		  			26

extern const char *  const   ex_flags [];

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
  SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
  SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
  SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_ROADS, SECT_MAX
} sector_types;

extern const int is_inside[SECT_MAX];

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
  WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
  WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
  WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
  WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EARS, WEAR_CAPE,
  WEAR_UNUSED_MISSILE_WIELD, WEAR_SHOULDERS, WEAR_QUIVER, WEAR_SCABBARD_1,
  WEAR_SCABBARD_2, WEAR_SHEATHED_1, WEAR_SHEATHED_2, MAX_WEAR
} wear_locations;

extern const char * const wear_locs [];

/* Board Types */
typedef enum { BOARD_NOTE, BOARD_MAIL } board_types;

/* Auth Flags */
#define FLAG_WRAUTH		      1
#define FLAG_AUTH		      2

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
  COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

/*
 * Positions.
 */
typedef enum
{
  POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_RESTING,
  POS_SITTING, POS_FIGHTING, POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG,
  POS_MEDITATING
} positions;

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      BV00	/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		      BV01
#define PLR_SHOVEDRAG		      BV02
#define PLR_AUTOEXIT		      BV03
#define PLR_AUTOLOOT		      BV04
#define PLR_AUTOSAC                   BV05
#define PLR_BLANK		      BV06
#define PLR_OUTCAST 		      BV07
#define PLR_BRIEF		      BV08
#define PLR_COMBINE		      BV09
#define PLR_PROMPT		      BV10
#define PLR_TELNET_GA		      BV11

#define PLR_HOLYLIGHT		   BV12
#define PLR_WIZINVIS		   BV13
#define PLR_ROOMVNUM		   BV14

#define	PLR_SILENCE		   BV15
#define PLR_NO_EMOTE		   BV16
/* #define PLR_PLAYBUILD			BV17*/	/* are we in playbuild mode? */
#define PLR_NO_TELL		   BV18
#define PLR_LOG			   BV19
#define PLR_DENY		   BV20
#define PLR_FREEZE		   BV21
#define PLR_TICKMSG        BV22
/* BV23 */
#define PLR_LITTERBUG	           BV24
#define PLR_ANSI	           BV25
#define PLR_RIP		           BV26
#define PLR_NICE	           BV27
#define PLR_FLEE	           BV28
#define PLR_AUTOGOLD               BV29
#define PLR_AUTOMAP                BV30
#define PLR_AFK                    BV31
/* #define PLR_INVISPROMPT         BV32  what the hell is this? */

/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
/* BIT 01 EMPTY */
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10

#define ACCOUNT_BANNED         BV00 /* no chars from this account can play */
#define ACCOUNT_CANMPLAY       BV01 /* allowed to play two chars at once */
#define ACCOUNT_IMMORTAL       BV02 /* allow past wizlock, et al */
#define ACCOUNT_LOGGED         BV03 /* log everything they do */
#define ACCOUNT_NOAUTOALLOW    BV04 /* don't autmatically approve */
#define ACCOUNT_WAITING        BV05 /* waiting to be approved */
#define ACCOUNT_NOTIFY         BV06 /* likes to stay informed */
#define ACCOUNT_CANWHO         BV07 /* wants proper who */
#define MAX_ACCOUNT_FLAGS      8

typedef enum
{
  TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
TIMER_APPLIED, TIMER_LOGON } timer_types;

struct timer_data
{
    TIMER  *	prev;
    TIMER  *	next;
    DO_FUN *	do_fun;
    int		value;
    sh_int	type;
    sh_int	count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_QUEST		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_ASK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_YELL		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
#define CHANNEL_HIGHGOD		   BV10
#define CHANNEL_CLAN		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_HIGH		   BV13
#define CHANNEL_AVTALK		   BV14
#define CHANNEL_PRAY		   BV15
#define CHANNEL_COUNCIL 	   BV16
#define CHANNEL_GUILD              BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_ORDER              BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_CODETALK       BV23
#define CHANNEL_OOC            BV24
#define CHANNEL_BELLOW         BV25
#define CHANNEL_ARENA          BV26
#define CHANNEL_JUSTICE        BV27
#define CHANNEL_EVTALK         BV28
#define CHANNEL_SGTALK         BV29
#define CHANNEL_OSAY			BV30

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
/*#define AFLAG_NOPKILL               BV00 */

//Ksilyan: area does not appear in command "areas"
#define AFLAG_NOTINLIST        BV01
#define AFLAG_NOPLAYERLOOT     BV02

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
class mob_index_data : public SkryptContainer
{
	public:
		mob_index_data();

		// old stuff

		MOB_INDEX_DATA *	next;
		MOB_INDEX_DATA *	next_sort;
		SPEC_FUN *		spec_fun;
		SHOP_DATA *		pShop;
		STABLE_DATA *   pStable; /* matthew */
		TRAIN_DATA *    train; /* matthew */
		REPAIR_DATA *	rShop;
		MPROG_DATA *	mudprogs;
		int64			progtypes;
		shared_str		playerName_;
		shared_str		shortDesc_;
		shared_str		longDesc_;
		shared_str		description_;
		int			vnum;
		sh_int		count;
		sh_int		killed;
		sh_int		sex;
		sh_int		level;
		int			act;
		int			affected_by;
		sh_int		alignment;
		sh_int		mobthac0;		/* Unused */
		sh_int		ac;
		sh_int		hitnodice;
		sh_int		hitsizedice;
		sh_int		hitplus;
		sh_int		damnodice;
		sh_int		damsizedice;
		sh_int		damplus;
		sh_int		numattacks;
		int			gold;
		int			exp;
		int			xflags;
		int			resistant;
		int			immune;
		int			susceptible;
		int			attacks;
		int			defenses;
		int			speaks;
		int 		speaking;
		sh_int		position;
		sh_int		defposition;
		sh_int		height;
		sh_int		weight;
		sh_int		race;
		sh_int		Class;
		sh_int		hitroll;
		sh_int		damroll;
		sh_int		perm_str;
		sh_int		perm_int;
		sh_int		perm_wis;
		sh_int		perm_dex;
		sh_int		perm_con;
		sh_int		perm_cha;
		sh_int		perm_lck;
		sh_int		saving_poison_death;
		sh_int		saving_wand;
		sh_int		saving_para_petri;
		sh_int		saving_breath;
		sh_int		saving_spell_staff;

		shared_str  exitDesc_;
		shared_str  enterDesc_;

		// Ksilyan:
		int HomeVnum;

		// Aiyan
		int ScentId;
};


struct HuntHateFear
{
	// Constructor
	HuntHateFear() : who(NULL) { /* empty */ }

	shared_str name_;
	Character* who;
};

struct fighting_data
{
    Character * who;
    int			xp;
    sh_int		align;
    sh_int		duration;
    sh_int		timeskilled;
};

struct	editor_data
{
    sh_int		numlines;
    sh_int		on_line;
    sh_int		size;
    char		line[MAX_BUFFER_LINES][MAX_BUFFER_COLS];
};

struct	extracted_char_data
{
    EXTRACT_CHAR_DATA *	next;
    CHAR_DATA *		ch;
    ROOM_INDEX_DATA *	room;
    ch_ret		retcode;
    bool		extract;
};


#define TEMPNUM_MOB 1
#define TEMPNUM_OBJ 2
#define TEMPNUM_CHAR 3

// character stuff.
#include "character.h"

struct killed_data
{
    int			vnum;
    char		count;
};

/* CITY THIEF STUFF
 added by Ksilyan
 */

#define CITY_WINDY 0
#define CITY_PHATEP 1
#define CITY_AINA 2
#define CITY_ERIA 3
#define CITY_KETTIN 4
#define CITY_VARIN 5
#define CITY_ELSIN 6
#define MAX_CITIES 7

extern const char * const city_names[];


// forward decl
struct SkillType;

/*
 * Data which only PC's have.
 */
class PlayerData
{
	public:
		// constructor
		PlayerData();

		ClanData *		clan;
		CouncilData * 	council;
		AREA_DATA *		area;
		DeityData *	deity;
		char *		homepage;
		char *      email;
		shared_str  clanName_;
		shared_str  councilName_;
		shared_str  deityName_;
		char *		pwd;
		shared_str  rank_;
		shared_str  title_;
		bool titleLocked; // is the title locked?      - Ksilyan
		shared_str  bestowments_;	/* Special bestowed commands	   */
		int                 flags;		/* Whether the player is deadly and whatever else we add.      */
		int			pkills;		/* Number of pkills on behalf of clan */
		int			pdeaths;	/* Number of times pkilled (legally)  */
		int			mkills;		/* Number of mobs killed		   */
		int			mdeaths;	/* Number of deaths due to mobs       */
		int			illegal_pk;	/* Number of illegal pk's committed   */
		time_t            secOutcastTime;	/* The time at which the char was outcast */
		time_t            secRestoreTime;	/* The last time the char did a restore all */
		int			r_range_lo;	/* room range */
		int			r_range_hi;
		int			m_range_lo;	/* mob range  */
		int			m_range_hi;
		int			o_range_lo;	/* obj range  */
		int			o_range_hi;
		sh_int		wizinvis;	/* wizinvis level */
		sh_int		min_snoop;	/* minimum snoop level */
		sh_int		condition	[MAX_CONDS];
		sh_int		learned		[MAX_SKILL];
		KILLED_DATA		killed		[MAX_KILLTRACK];
		sh_int		quest_number;	/* current *QUEST BEING DONE* DON'T REMOVE! */
		sh_int		quest_curr;	/* current number of quest points */
		sh_int      quest_deci;
		int			quest_accum;	/* quest points accumulated in players life */
		sh_int		favor;		/* deity favor */
		int			auth_state;
		time_t		secReleaseDate; /* Auto-helling.. Altrag */
		shared_str  helledBy_;
		char *		bio;		/* Personal Bio */
		shared_str  authedBy_;	/* what crazy imm authed this name ;) */
		SkillType *		special_skills[5]; /* personalized skills/spells */
		shared_str  prompt_;		/* User config prompts */
		shared_str	prompt2_;    // Secondary prompt   -Ksilyan
		char *		subprompt;	/* Substate prompt */
		sh_int		pagerlen;	/* For pager (NOT menus) */
		bool		openedtourney;
		sh_int              pc_colors[MAX_PCCOLORS];
		sh_int              last_color;
		/* new fields added by Testaur */
		int                 recall_room;    /* recall/death room vnum */
		int                 bank_gold;      /* how much gold locked away? */
		int                 rating;         /* arenarank rating */
		int                 ethos;          /* 0=default - which 51+ stream? */

		/* new fields added by Ksilyan */
		int         citythief[MAX_CITIES];
		int			CityWanted[MAX_CITIES];

		ObjectStorage * Vault; // the player's vault
		time_t secVaultLastAccessed; // the time at which the vault was last accessed

		shared_str  bamfIn_;
		shared_str  bamfOut_;
};
#define ETHOS_HERO 				1
#define ETHOS_VILLAIN 			2
#define ETHOS_SAGE 				3


/*
 * Liquids.
 */
#define LIQ_WATER        		0
#define LIQ_POISON   			18
#define LIQ_MAX					20

struct	liq_type
{
    const char *	liq_name;
    const char *	liq_color;
    sh_int	liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct	ExtraDescData
{
    ExtraDescData * next;	/* Next in list                     */
    ExtraDescData * prev;	/* Previous in list                 */
    shared_str keyword_;              /* Keyword in look/examine          */
    shared_str description_;          /* What to see                      */
};


/*
 * KSILYAN
 *	Information to more easily look at object values
 */


// Gem flags
#define GEM_PURIFIED  BV01
#define GEM_IMBUED    BV02

// Gem quality modifier (in %)
#define GEM_DEFORMED 40
#define GEM_FLAWED   70
#define GEM_FAIR     100
#define GEM_CLEAR    130
#define GEM_PERFECT  160

// Gem size modifier (in %)
#define GEM_TINY     20
#define GEM_SMALL    50
#define GEM_AVERAGE  100
#define GEM_LARGE    130
#define GEM_HUGE     160

#define OBJECT_GEM_VALUE 0 // base value of gem at average quality/size
#define OBJECT_GEM_QUALITY 1 // the quality modifier in % on value
#define OBJECT_GEM_SIZE 2 // the size modifier in % on value
#define OBJECT_GEM_FLAGS 3
#define OBJECT_GEM_SPELL 4
#define OBJECT_GEM_CHARGES 5

#define OBJECT_WAND_CHARGES 2
#define OBJECT_WAND_MAXCHARGES 1

#define OBJECT_CONTAINER_CAPACITY 0

#define OBJECT_QUIVER_CAPACITY 0
#define OBJECT_QUIVER_AMMOTYPE 1

#define OBJECT_SCABBARD_WEAPONTYPE 0

#define OBJECT_COMPONENT_QUANTITY 0

#define OBJECT_ARMOR_AC 0
#define OBJECT_ARMOR_CLASSIFICATION 5

#define OBJECT_WEAPON_DAMAGETYPE 0 /* for ranged weapons, damage type = ammo type */
#define OBJECT_WEAPON_AMMOTYPE OBJECT_WEAPON_DAMAGETYPE
#define OBJECT_WEAPON_MINDAMAGE 1	/* min/max damamge inactive for missile weapons. */
#define OBJECT_WEAPON_MAXDAMAGE 2
#define OBJECT_WEAPON_WEAPONTYPE 3
#define OBJECT_WEAPON_DAMAGEMESSAGE 4  /* for missile weapons is inactive */
#define OBJECT_WEAPON_POWER 5

/* A few weapon utility macros. */
#define IS_RANGED_WEAPON(type) ( ( (type >= WEAPON_LONGBOW) && (type <= WEAPON_SLING) ) ? TRUE : FALSE )
#define IS_RANGED_WEAPON_OBJ(obj) ( ( (obj->value[OBJECT_WEAPON_WEAPONTYPE] >= WEAPON_LONGBOW) && \
								(obj->value[OBJECT_WEAPON_WEAPONTYPE] <= WEAPON_SLING) ) ? TRUE : FALSE )

#define OBJECT_PROJECTILE_AMMOTYPE 0
#define OBJECT_PROJECTILE_MINDAMAGE 1
#define OBJECT_PROJECTILE_MAXDAMAGE 2
#define OBJECT_PROJECTILE_RANGE 3
#define OBJECT_PROJECTILE_DAMAGETYPE 4
#define OBJECT_PROJECTILE_DAMAGEMESSAGE 5

#define WEAPON_NONE				0
#define WEAPON_PUGILISM			1
#define WEAPON_LONGBLADE		2
#define WEAPON_SHORTBLADE		3
#define WEAPON_FLEXIBLE			4
#define WEAPON_BLUDGEON			5
#define WEAPON_POLEARM			6
#define WEAPON_LONGBOW			7
#define WEAPON_SHORTBOW			8
#define WEAPON_CROSSBOW			9
#define WEAPON_SLING			10
#define WEAPON_THROWINGSPEAR	11
#define WEAPON_THROWINGKNIFE	12

#define MAX_WEAPON_TYPE			12

//--------------------------------

#define MIN_DAMAGE_TYPE			13

#define DAMAGE_DEFAULT			13
#define DAMAGE_SLASH			14
#define DAMAGE_PIERCE			15
#define DAMAGE_BLUNT			16
#define DAMAGE_FIRE				17
#define DAMAGE_COLD				18
#define DAMAGE_ELECTRICITY		19
#define DAMAGE_ACID				20
#define DAMAGE_ENERGY			21

#define MAX_DAMAGE_TYPE			21

//--------------------------------

#define MIN_DAMAGE_MSG			13

#define DAMAGE_MSG_NONE			0

#define DAMAGE_MSG_DEFAULT		13
#define DAMAGE_MSG_SWING		14
#define DAMAGE_MSG_STAB			15
#define DAMAGE_MSG_WHIP			16
#define DAMAGE_MSG_POUND		17
#define DAMAGE_MSG_CRUSH		18
#define DAMAGE_MSG_CLAW			19
#define DAMAGE_MSG_BITE			20
#define DAMAGE_MSG_KNIFE		21 // throwing knife, NOT meelee knife
#define DAMAGE_MSG_SPEAR		22 // throwing spear, NOT meelee spear
#define DAMAGE_MSG_ARROW		23
#define DAMAGE_MSG_BOLT			24
#define DAMAGE_MSG_STONE		25
#define DAMAGE_MSG_FIRE			26
#define DAMAGE_MSG_COLD			27
#define DAMAGE_MSG_ELECTRICITY	28
#define DAMAGE_MSG_ACID			29
#define DAMAGE_MSG_ENERGY		30
#define DAMAGE_MSG_POLEARM		31
#define DAMAGE_MSG_KICK			32
#define DAMAGE_MSG_PUNCH		33
#define DAMAGE_MSG_SHIELDBASH	34

#define MAX_DAMAGE_MSG			34

//--------------------------------

#define AMMO_NONE				0
#define	AMMO_LONGARROW			1
#define AMMO_SHORTARROW			2
#define AMMO_BOLT				3
#define AMMO_STONE				4

#define MAX_AMMO_TYPE			4

//--------------------------------

/* Body parts- used to determine
 * where you got hit in combat.
*/

#define MIN_BODYPART			0

#define BODYPART_HEAD			0	/* 10% */
#define BODYPART_NECK			1	/* 4%  */
#define BODYPART_LEFTARM		2	/* 12% */
#define BODYPART_RIGHTARM		3	/* 12% */
#define BODYPART_CHEST			4	/* 25% */
#define BODYPART_WAIST			5	/* 5%  */
#define BODYPART_LEFTLEG		6	/* 10% */
#define BODYPART_RIGHTLEG		7	/* 10% */
#define BODYPART_LEFTFOOT		8	/* 2%  */
#define BODYPART_RIGHTFOOT		9	/* 2%  */
#define BODYPART_LEFTHAND		10	/* 4%  */
#define BODYPART_RIGHTHAND		11	/* 4%  */

#define MAX_BODYPART			11

/*
 * Prototype for an object.
 */
class obj_index_data : public SkryptContainer
{
public:
	obj_index_data();

    OBJ_INDEX_DATA *	next;
    OBJ_INDEX_DATA *	next_sort;
    ExtraDescData *	first_extradesc;
    ExtraDescData *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    MPROG_DATA *	mudprogs;               /* objprogs */
    int64			progtypes;              /* objprogs */
    shared_str  name_;
    shared_str  shortDesc_;
	shared_str  longDesc_; // changed from char* to hashed string for stability - Ksilyan sep-12-2004
    shared_str  actionDesc_;
    int			vnum;
    sh_int              level;
    sh_int		item_type;
    int			extra_flags;
	int			extra_flags_2; /* Needed more flags - Ksilyan */
    int			magic_flags; /*Need more bitvectors for spells - Scryn*/
    int			wear_flags;
    sh_int		count;
    sh_int		weight;
    int			cost;
    int			value	[6];
    int			serial;
    sh_int		layers;
    sh_int      rare;
    int         total_count;    /* number in game + in pfiles */
    int			rent;			/* Unused */
	int			max_condition;	/* maximum "strength" */
};

/*
 * Exit data.
 */
struct ExitData
{
	// Constructor
	ExitData();

    ExitData *		prev;		/* previous exit in linked list	*/
    ExitData *		next;		/* next exit in linked list	*/
    ExitData *		rexit;		/* Reverse exit pointer		*/
    ROOM_INDEX_DATA *	to_room;	/* Pointer to destination room	*/
    shared_str  keyword_;	/* Keywords for exit or door	*/
    shared_str  description_;	/* Description of exit		*/
    int			vnum;		/* Vnum of room exit leads to	*/
    int			rvnum;		/* Vnum of room in opposite dir	*/
    int			exit_info;	/* door states & other flags	*/
    int			key;		/* Key vnum			*/
    sh_int		vdir;		/* Physical "direction"		*/
    sh_int		distance;	/* how far to the next room	*/
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */



struct	reset_data
{
    RESET_DATA *	next;
    RESET_DATA *	prev;
    char		command;
    int			extra;
    int			arg1;
    int			arg2;
    int			arg3;
};

/* Constants for arg2 of 'B' resets. */
#define	BIT_RESET_DOOR			0
#define BIT_RESET_OBJECT		1
#define BIT_RESET_MOBILE		2
#define BIT_RESET_ROOM			3
#define BIT_RESET_TYPE_MASK		0xFF	/* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK		0xFF00	/* 256 should be enough */
#define BIT_RESET_SET			BV30
#define BIT_RESET_TOGGLE		BV31
#define BIT_RESET_FREEBITS	  0x3FFF0000	/* For reference */


#include "area.h"


/*
 * Load in the gods building data. -- Altrag
 */
struct	godlist_data
{
    GOD_DATA *		next;
    GOD_DATA *		prev;
    int			level;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    sh_int		low_m_vnum;
    sh_int		hi_m_vnum;
};


/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct	system_data
{
    int		maxplayers;		/* Maximum players this boot   */
    int		alltimemax;		/* Maximum players ever	  */
    char *	time_of_max;		/* Time of max ever */
    bool	NO_NAME_RESOLVING;	/* Hostnames are not resolved  */
    bool    	DENY_NEW_PLAYERS;	/* New players cannot connect  */
    bool	WAIT_FOR_AUTH;		/* New players must be auth'ed */
    sh_int	read_all_mail;		/* Read all player mail(was 54)*/
    sh_int	read_mail_free;		/* Read mail for free (was 51) */
    sh_int	write_mail_free;	/* Write mail for free(was 51) */
    sh_int	take_others_mail;	/* Take others mail (was 54)   */
    sh_int	muse_level;		/* Level of muse channel */
    sh_int	think_level;		/* Level of think channel LEVEL_HIGOD*/
    sh_int	build_level;		/* Level of build channel LEVEL_BUILD*/
    sh_int	log_level;		/* Level of log channel LEVEL LOG*/
    sh_int	level_modify_proto;	/* Level to modify prototype stuff LEVEL_LESSER */
    sh_int  level_modify_proto_flag; /* Level at which you can toggle the
                                        proto flag */
    sh_int	level_override_private;	/* override private flag */
    sh_int	level_mset_player;	/* Level to mset a player */
    sh_int	stun_plr_vs_plr;	/* Stun mod player vs. player */
    sh_int	stun_regular;		/* Stun difficult */
    sh_int	dam_plr_vs_plr;		/* Damage mod player vs. player */
    sh_int	dam_plr_vs_mob;		/* Damage mod player vs. mobile */
    sh_int	dam_mob_vs_plr;		/* Damage mod mobile vs. player */
    sh_int	dam_mob_vs_mob;		/* Damage mod mobile vs. mobile */
    sh_int	level_getobjnotake;     /* Get objects without take flag */
    sh_int      level_forcepc;          /* The level at which you can use force on players. */
    sh_int	max_sn;			/* Max skills */
    char       *guild_overseer;         /* Pointer to char containing the name of the */
    char       *guild_advisor;		/* guild overseer and advisor. */
    int		save_flags;		/* Toggles for saving conditions */
    sh_int	save_frequency;		/* How old to autosave someone */
    /* NEW */
    sh_int  level_interrupt_buffer;
    sh_int  level_monitor;
    sh_int  level_restore_all;
    sh_int  level_restore_on_player;
};

#include "room.h" // Get rid of this sometime!


/*
 * Delayed teleport type.
 */
struct	teleport_data
{
    TELEPORT_DATA *	next;
    TELEPORT_DATA *	prev;
    ROOM_INDEX_DATA *	room;
    sh_int		timer;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED				-1
#define TYPE_SKILL					0		/* allows for 1000 skills/spells */
#define TYPE_HIT					1000 	/* allows for 1000 attack types  */
#define TYPE_HERB					2000 	/* allows for 1000 herb types    */
#define TYPE_PERSONAL				3000

/* KSILYAN
	Small note here.
	TYPE_HIT is the type used for all weapon-based attacks.
*/

/*
 *  Target types.
 */
typedef enum
{
  TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
  TAR_OBJ_INV
} target_types;

typedef enum
{
  SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
  SKILL_HERB
} skill_types;



struct timerset
{
  int num_uses;
  struct timeval total_time;
  struct timeval min_time;
  struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct	SkillType
{
	// Cnstructor:
	SkillType();

	shared_str name_;			/* Name of skill		*/
	sh_int	skill_level[MAX_CLASS];	/* Level needed by class	*/
	sh_int	skill_adept[MAX_CLASS];	/* Max attainable % in this skill */
	SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
	DO_FUN *	skill_fun;		/* Skill pointer (for skills)	*/
	sh_int	target;			/* Legal targets		*/
	sh_int	minimum_position;	/* Position for caster / user	*/
	sh_int	slot;			/* Slot for #OBJECT loading	*/
	sh_int	min_mana;		/* Minimum mana used		*/
	sh_int	beats;			/* Rounds required to use skill	*/
	shared_str nounDamage_;		/* Damage message		*/
	shared_str msgOff_;		/* Wear off message		*/
	sh_int	guild;			/* Which guild the skill belongs to */
	sh_int	min_level;		/* Minimum level to be able to cast */
	sh_int	type;			/* Spell/Skill/Weapon/Tongue	*/
	int		flags;			/* extra stuff			*/
	char *	hit_char;		/* Success message to caster	*/
	char *	hit_vict;		/* Success message to victim	*/
	char *	hit_room;		/* Success message to room	*/
	char *	miss_char;		/* Failure message to caster	*/
	char *	miss_vict;		/* Failure message to victim	*/
	char *	miss_room;		/* Failure message to room	*/
	char *	die_char;		/* Victim death msg to caster	*/
	char *	die_vict;		/* Victim death msg to victim	*/
	char *	die_room;		/* Victim death msg to room	*/
	char *	imm_char;		/* Victim immune msg to caster	*/
	char *	imm_vict;		/* Victim immune msg to victim	*/
	char *	imm_room;		/* Victim immune msg to room	*/
	char *	dice;			/* Dice roll			*/
	int		value;			/* Misc value			*/
	char	saves;			/* What saving spell applies	*/
	char	difficulty;		/* Difficulty of casting/learning */
	SMAUG_AFF *	affects;		/* Spell affects, if any	*/
	char *	components;		/* Spell components, if any	*/
	char *	teachers;		/* Skill requires a special teacher */
	char	participants;		/* # of required participants	*/
	struct	timerset	userec;	/* Usage record			*/

	char * Prerequisites;   /* What skills you need to learn this one (in argument form) */
};


struct  auction_data
{
    Object  * item;   /* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    sh_int      going;  /* 1,2, sold */
    sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
    int 	starting;
};

/*
 * These are skill_lookup return values for common skills and spells.
 */
/* projectiles */
extern  sh_int  gsn_short_bows;
extern  sh_int  gsn_cross_bows;
extern  sh_int  gsn_long_bows;
extern  sh_int  gsn_slings;
extern  sh_int  gsn_throwing_daggers;
extern  sh_int  gsn_throwing_spears;

extern  sh_int  gsn_meditate;
extern	sh_int	gsn_detrap;
extern	sh_int	gsn_backstab;
extern  sh_int  gsn_circle;
extern	sh_int	gsn_dodge;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern  sh_int  gsn_scan;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;
extern	sh_int	gsn_gouge;
extern	sh_int	gsn_track;
extern	sh_int	gsn_search;
extern  sh_int  gsn_dig;
extern	sh_int	gsn_mount;
extern  sh_int  gsn_bashdoor;
extern	sh_int	gsn_berserk;
extern	sh_int	gsn_hitall;

extern	sh_int	gsn_disarm;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_parry;
extern	sh_int	gsn_rescue;
extern	sh_int	gsn_second_attack;
extern	sh_int	gsn_third_attack;
extern	sh_int	gsn_fourth_attack;
extern	sh_int	gsn_fifth_attack;
extern	sh_int	gsn_dual_wield;

extern	sh_int	gsn_feed;

extern	sh_int	gsn_aid;

/* used to do specific lookups */
extern	sh_int	gsn_first_spell;
extern	sh_int	gsn_first_skill;
extern	sh_int	gsn_first_weapon;
extern	sh_int	gsn_first_tongue;
extern	sh_int	gsn_top_sn;

/* spells */
extern	sh_int	gsn_blindness;
extern	sh_int	gsn_charm_person;
extern  sh_int  gsn_aqua_breath;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mass_invis;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_sleep;
extern  sh_int  gsn_possess;
extern	sh_int	gsn_fireball;		/* for fireshield  */
extern	sh_int	gsn_chill_touch;	/* for iceshield   */
extern	sh_int	gsn_lightning_bolt;	/* for shockshield */

/* newer attack skills */
extern	sh_int	gsn_punch;
extern	sh_int	gsn_bash;
extern	sh_int	gsn_stun;
extern	sh_int	gsn_bite;
extern	sh_int	gsn_claw;
extern	sh_int	gsn_sting;
extern	sh_int	gsn_tail;

extern  sh_int  gsn_poison_weapon;
extern	sh_int	gsn_climb;

extern	sh_int	gsn_pugilism;
extern	sh_int	gsn_long_blades;
extern	sh_int	gsn_short_blades;
extern	sh_int	gsn_flexible_arms;
extern	sh_int	gsn_polearms;
extern	sh_int	gsn_bludgeons;
extern	sh_int	gsn_shieldwork;

extern  sh_int  gsn_grip;
extern  sh_int  gsn_slice;

/* Language gsns. -- Altrag */
extern  sh_int  gsn_common;
extern  sh_int  gsn_elven;
extern  sh_int  gsn_dwarven;
extern  sh_int  gsn_pixie;
extern  sh_int  gsn_ogre;
extern  sh_int  gsn_orcish;
extern  sh_int  gsn_trollish;
extern  sh_int  gsn_goblin;
extern  sh_int  gsn_halfling;
extern  sh_int  gsn_katrin;


/* Crafting skills - Zoie */
extern  sh_int  gsn_scribe;
extern  sh_int  gsn_brew;
extern  sh_int  gsn_cook;
extern  sh_int  gsn_slice;
extern  sh_int  gsn_kindle_fire; /* Ksilyan's */
extern  sh_int  gsn_mine;
extern  sh_int  gsn_smelt;
extern  sh_int  gsn_forge;

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))
#define CH(d)			((d)->original ? (d)->original : (d)->character)

/*
 * Memory allocation macros.
 */

#define CREATE(result, type, number)				\
do								\
{								\
   if (!((result) = (type *) calloc ((number), sizeof(type))))	\
	{ perror("malloc failure"); abort(); }			\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
	{ perror("realloc failure"); abort(); }			\
} while(0)


#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer" ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)



#ifdef HASHSTR
#define STRALLOC(point)		str_alloc((point))
#define QUICKLINK(point)	quick_link((point))
#define QUICKMATCH(p1, p2)	(int) (p1) == (int) (p2)
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer" );	 			\
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else if (str_free((point))==-1) 				\
    fprintf( stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
} while(0)
#else
#define STRALLOC(point)		str_dup((point))
#define QUICKLINK(point)	str_dup((point))
#define QUICKMATCH(p1, p2)	strcmp((p1), (p2)) == 0
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer" );				\
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free((point));						\
} while(0)
#endif

/* double-linked list handling macros -Thoric */

#define LINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (last)->next		= (link);			\
    (link)->next		= NULL;				\
    (link)->prev		= (last);			\
    (last)			= (link);			\
} while(0)

#define INSERT(link, insert, first, next, prev)			\
do								\
{								\
    (link)->prev		= (insert)->prev;		\
    if ( !(insert)->prev )					\
      (first)			= (link);			\
    else							\
      (insert)->prev->next	= (link);			\
    (insert)->prev		= (link);			\
    (link)->next		= (insert);			\
} while(0)

#define UNLINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(link)->prev )					\
      (first)			= (link)->next;			\
    else							\
      (link)->prev->next	= (link)->next;			\
    if ( !(link)->next )					\
      (last)			= (link)->prev;			\
    else							\
      (link)->next->prev	= (link)->prev;			\
} while(0)


#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "CHECK_LINKS: last with NULL first!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "CHECK_LINKS: first with NULL last!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)


#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
	fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\n\r", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		(get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_AVATAR(ch) (ch->level >= LEVEL_HERO_MIN && ch->level <= LEVEL_HERO_MAX )
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_HERO_MIN && get_trust((ch)) <= LEVEL_HERO_MAX)
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || IS_SET((ch)->xflags, (part)))

#define CAN_CAST(ch)		((ch)->Class != 2 && (ch)->Class != 3)

#define IS_VAMPIRE(ch)		(!IS_NPC(ch)				    \
				&& ((ch)->race==RACE_VAMPIRE		    \
				||  (ch)->Class==CLASS_VAMPIRE))
#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_ORDERED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_ORDER)

#define IS_GUILDED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_GUILD)

#define IS_DEADLYCLAN(ch)	(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_NOKILL) \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER)  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_DEVOTED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->deity)

//#define WAIT_STATE(ch, npulse)	((ch)->AddWait((npulse)))


#define EXIT(ch, door)		( get_exit( (ch)->GetInRoom(), door ) )

#define CAN_GO(ch, door)	(EXIT((ch),(door))			 \
				&& (EXIT((ch),(door))->to_room != NULL)  \
                          	&& !IS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name_.length() > 0 )

#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name_.length() > 0 )

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )
#define SPELL_DAMAGE(skill)	( ((skill)->flags     ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->flags >> 3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->flags >> 6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->flags >> 9) & 3 )
#define SET_SDAM(skill, val)	( (skill)->flags =  ((skill)->flags & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->flags =  ((skill)->flags & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->flags =  ((skill)->flags & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->flags =  ((skill)->flags & SPOW_MASK) + (((val) & 3) << 9) )

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_GUEST))

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE )
#define IS_COLD(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_COLD )
#define IS_ACID(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ACID )
#define IS_ELECTRICITY(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ELECTRICITY )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )

#define IS_DRAIN(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_DRAIN )

#define IS_POISON(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_POISON )


#define NOT_AUTHED(ch)		(!IS_NPC(ch) && ch->pcdata && ch->pcdata->auth_state <= 3  \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->GetConnection()		     \
			      && ch->pcdata->auth_state == 1		     \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))



/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				(ch)->getShort().c_str() : "someone" )

#define NAME(ch)        ( (ch)->getShort().c_str() )

#define SEX_HE(ch) (ch->sex == SEX_NEUTRAL ? "it" : (ch->sex == SEX_MALE ? "he" : "she"))
#define SEX_HIS(ch) (ch->sex == SEX_NEUTRAL ? "its" : (ch->sex == SEX_MALE ? "his" : "her"))
#define SEX_HIM(ch) (ch->sex == SEX_NEUTRAL ? "it" : (ch->sex == SEX_MALE ? "him" : "her"))

#define GET_ADEPT(ch,sn)    (  skill_table[(sn)]->skill_adept[(ch)->Class])
#define LEARNED(ch,sn)      (IS_NPC(ch) ? 80 : URANGE(0, ch->pcdata->learned[sn], 101))

#define log_string( txt )	( log_string_plus( (txt), LOG_NORMAL, sysdata.log_level ) )


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    CMDTYPE *		next;
    char *		name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    struct		timerset	userec;
};



/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    SOCIALTYPE *	next;
    char *		name;
    char *		char_no_arg;
    char *		others_no_arg;
    char *		char_found;
    char *		others_found;
    char *		vict_found;
    char *		char_auto;
    char *		others_auto;
    int min_level;
    int flags;
    int resp_same_sex;
    int resp_vict_female;
    int resp_vict_male;
};



/*
 * Global constants.
 */
extern  bool fCopyOver;

extern  time_t secLastRestoreAllTime;
extern  time_t secBootTime;  /* this should be moved down */
extern  HOUR_MIN_SEC * set_boot_time;
extern  struct  tm *new_boot_time;
extern  time_t secNewBoot_time_t;

extern	const	struct	str_app_type	str_app		[26];
extern	const	struct	int_app_type	int_app		[26];
extern	const	struct	wis_app_type	wis_app		[26];
extern	const	struct	dex_app_type	dex_app		[26];
extern	const	struct	con_app_type	con_app		[26];
extern	const	struct	cha_app_type	cha_app		[26];
extern  const	struct	lck_app_type	lck_app		[26];

extern	const	struct	race_type	race_table	[MAX_RACE];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	const char *	const			 attack_table	[MAX_DAM_TYPES+1];

extern	const char *	const	skill_tname	[];
extern	sh_int	const	movement_loss	[SECT_MAX];
extern	const char *	const	dir_name	[];
extern	const char *	const	where_name	[];
extern	const	sh_int	rev_dir		[];
extern  const char *  const   account_flags[];
extern	const	int	trap_door	[];
extern	const char *	const	r_flags		[];
extern	const char *	const	w_flags		[];
extern	const char *	const	o_flags		[];
extern	const char *	const	o_flags_2	[];
extern	const char *	const	a_flags		[];
extern	const char *	const	o_types		[];
extern	const char *	const	a_types		[];
extern	const char *	const	act_flags	[];
extern	const char *	const	plr_flags	[];
extern	const char *	const	pc_flags	[];
extern	const char *	const	trap_flags	[];
extern	const char *	const	ris_flags	[];
extern	const char *	const	trig_flags	[];
extern	const char *	const	part_flags	[];
extern	const char *	const	npc_race	[];
extern	const char *	const	npc_class	[];
extern	const char *	const	defense_flags	[];
extern	const char *	const	attack_flags	[];
extern	const char *	const	area_flags	[];

extern	int	const	lang_array      [];
extern	const char *	const	lang_names      [];

extern  const char *  const   str_names       [];
extern  const char *  const   int_names       [];
extern  const char *  const   wis_names       [];
extern  const char *  const   dex_names       [];
extern  const char *  const   con_names       [];
extern  const char *  const   cha_names       [];
extern  const char *  const   lck_names       [];

/*
 * Global variables.
 */
extern	int	numobjsloaded;
extern	int	nummobsloaded;
extern	int	physicalobjects;
extern	int	num_descriptors;
extern	struct	system_data		sysdata;
extern	int	top_sn;
extern	int	top_vroom;
extern	int	top_herb;

extern		CMDTYPE		  *	command_hash	[126];

extern		ClassType *	class_table	[MAX_CLASS];
extern		char *			title_table	[MAX_CLASS]
							[MAX_LEVEL+1]
							[2];

extern		SkillType *	skill_table	[MAX_SKILL];
extern		SOCIALTYPE	  *	social_index	[27];
extern		CHAR_DATA	  *	cur_char;
extern		ROOM_INDEX_DATA	  *	cur_room;
extern		bool			cur_char_died;
extern		ch_ret			global_retcode;
extern		SkillType	  *	herb_table	[MAX_HERB];

extern		int			cur_obj;
extern		int			cur_obj_serial;
extern		bool			cur_obj_extracted;
extern		obj_ret			global_objcode;

extern		HELP_DATA	  *	first_help;
extern		HELP_DATA	  *	last_help;
extern		SHOP_DATA	  *	first_shop;
extern		SHOP_DATA	  *	last_shop;

/* matthew */
extern      STABLE_DATA   * first_stable;
extern      STABLE_DATA   * last_stable;

extern		REPAIR_DATA	  *	first_repair;
extern		REPAIR_DATA	  *	last_repair;

/* Matthew -- arena stuff */
extern          ARENA_DATA              current_arena;
/* Ksilyan -- automatic arenarank updating */
#define NUMBER_OF_ARENA_RANKINGS 40
extern          ARENA_RANK_DATA         ArenaRankings[NUMBER_OF_ARENA_RANKINGS];

/* Auction stuff - Warp */
extern       AUCTION_ROOM * first_auctionroom;

extern		BAN_DATA	  *	first_ban;
extern		BAN_DATA	  *	last_ban;

extern		CHAR_DATA	  *	first_char;
extern		CHAR_DATA	  *	last_char;
//extern		DESCRIPTOR_DATA   *	first_descriptor;
//extern		DESCRIPTOR_DATA   *	last_descriptor;
extern		BOARD_DATA	  *	first_board;
extern		BOARD_DATA	  *	last_board;
extern		Object	  *	first_object;
extern		Object	  *	last_object;
extern		ClanData *	first_clan;
extern		ClanData	  *	last_clan;
extern 		CouncilData *	first_council;
extern		CouncilData	  * 	last_council;
extern		DeityData *	first_deity;
extern		DeityData	  *	last_deity;
extern		AREA_DATA	  *	first_area;
extern		AREA_DATA	  *	last_area;
extern		AREA_DATA	  *	first_build;
extern		AREA_DATA	  *	last_build;
extern		AREA_DATA	  *	first_asort;
extern		AREA_DATA	  *	last_asort;
extern		AREA_DATA	  *	first_bsort;
extern		AREA_DATA	  *	last_bsort;

extern      int         top_mob_index;
extern      int         top_obj_index;

extern      TRAIN_DATA    * first_train;
extern      TRAIN_DATA    * last_train;

/*
extern		GOD_DATA	  *	first_imm;
extern		GOD_DATA	  *	last_imm;
*/
extern		TELEPORT_DATA	  *	first_teleport;
extern		TELEPORT_DATA	  *	last_teleport;
extern		Object	  *	extracted_obj_queue;
extern		EXTRACT_CHAR_DATA *	extracted_char_queue;
extern		Object	  *	save_equipment[MAX_WEAR][MAX_LAYERS];
extern		CHAR_DATA	  *	quitting_char;
extern		CHAR_DATA	  *	loading_char;
extern		CHAR_DATA	  *	saving_char;
extern		Object	  *	all_obj;
extern      time_t         secCurrentTime;
extern		char			bug_buf		[];
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		FILE *			fpLOG;
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;

extern          AUCTION_DATA      *     auction;
extern		struct act_prog_data *	mob_act_list;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		 ( const char *key, const char *salt ) ;
#endif

#if	defined(apollo)
int	atoi		 ( const char *string ) ;
void *	calloc		 ( unsigned nelem, size_t size ) ;
char *	crypt		 ( const char *key, const char *salt ) ;
#endif

#if	defined(hpux)
char *	crypt		 ( const char *key, const char *salt ) ;
#endif

#if	defined(interactive)
#endif

#if	defined(linux)
//char *	crypt		 ( const char *key, const char *salt ) ;
#endif

#if	defined(MIPS_OS)
char *	crypt		 ( const char *key, const char *salt ) ;
#endif

#if	defined(NeXT)
char *	crypt		 ( const char *key, const char *salt ) ;
#endif

#if	defined(sequent)
char *	crypt		 ( const char *key, const char *salt ) ;
int	fclose		 ( FILE *stream ) ;
int	fprintf		 ( FILE *stream, const char *format, ... ) ;
int	fread		 ( void *ptr, int size, int n, FILE *stream ) ;
int	fseek		 ( FILE *stream, long offset, int ptrname ) ;
void	perror		 ( const char *s ) ;
int	ungetc		 ( int c, FILE *stream ) ;
#endif

#if	defined(sun)
char *	crypt		 ( const char *key, const char *salt ) ;
int	fclose		 ( FILE *stream ) ;
int	fprintf		 ( FILE *stream, const char *format, ... ) ;
#if 	defined(SYSV)
size_t 	fread		( void *ptr, size_t size, size_t n,
				FILE *stream );
#else
int	fread		 ( void *ptr, int size, int n, FILE *stream ) ;
#endif
int	fseek		 ( FILE *stream, long offset, int ptrname ) ;
void	perror		 ( const char *s ) ;
int	ungetc		 ( int c, FILE *stream ) ;
#endif

#if	defined(ultrix)
char *	crypt		 ( const char *key, const char *salt ) ;
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 *
 * Added by Ksilyan: Windows doesn't have the crypt function, so
 * disable it even if they didn't specify NOCRYPT.
 */
#if	defined(NOCRYPT) || defined(WIN32) || defined(__CYGWIN__)
#define crypt(s1, s2)	(s1)
#endif

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	Object
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define BD	BOARD_DATA
#define CL	ClanData
#define EDD	ExtraDescData
#define RD	RESET_DATA
#define	ST	SOCIALTYPE
#define	CO	CouncilData
#define SK	SkillType

/* matthew */
/* stable.c */
void    stable_give      ( CHAR_DATA *ch, Object *obj, CHAR_DATA *keeper);

/* colors.c */
void reset_colors(CHAR_DATA* ch);

/* socials.c */
void color_social(char* txt, CHAR_DATA* ch, const void *arg1, const void* arg2, int type);

/* ARENA.C */
void  arena_clear_char(CHAR_DATA* ch);
Object * arena_strip_eq(CHAR_DATA * ch);
void arena_reequip_char(CHAR_DATA*ch, Object* pContainer);
bool  in_arena  ( CHAR_DATA *ch ) ;
int arena_rank_sort(const void * i, const void * j);
void update_arena_rank( CHAR_DATA * ch );

/* accounts.c */
void    save_account_banlist( void );

/* act_comm.c */

//void    write_main_menu (DESCRIPTOR_DATA*d);

bool	circle_follow	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
void	add_follower	 ( CHAR_DATA *ch, CHAR_DATA *master ) ;
void	stop_follower	 ( CHAR_DATA *ch ) ;
void	die_follower	 ( CHAR_DATA *ch ) ;
bool	is_same_group	 ( CHAR_DATA *ach, CHAR_DATA *bch ) ;
void	send_rip_screen  ( CHAR_DATA *ch ) ;
void	send_rip_title	 ( CHAR_DATA *ch ) ;
void	send_ansi_title  ( CHAR_DATA *ch ) ;
void	send_ascii_title  ( CHAR_DATA *ch ) ;
void	to_channel	( const char *argument, int channel,
				const char *verb, sh_int level );
void  	talk_auction     ( char *argument ) ;
bool    knows_language  ( CHAR_DATA *ch, int language,
				CHAR_DATA *cch );
bool    can_learn_lang   ( CHAR_DATA *ch, int language ) ;
int     countlangs       ( int languages ) ;
const char *  translate        ( CHAR_DATA *ch, CHAR_DATA *victim,
				const char *argument );
const char *	obj_short	 ( Object *obj ) ;
void	talk_channel	( CHAR_DATA *ch, const char *argument,
			    int channel, const char *verb );
std::string  scramble         ( const string & argument, int modifier ) ;
std::string  drunk_speech     ( const string & argument, CHAR_DATA *ch ) ;

#define MESSAGE_NO_COMM_DEVICE "You don't have a seerstone.\n\r"
bool CanGlobalCommunicate(CHAR_DATA * ch);

/* act_info.c */
const char *  player_title(const char *title_text);  /* vis part of locked title */
void	show_char_to_char	 ( CHAR_DATA * List, CHAR_DATA *ch ) ;
int	get_door	 ( const char *arg ) ;
char *	format_obj_to_char	( Object *obj, CHAR_DATA *ch,
				    bool fShort );
void	show_list_to_char	( Object * List, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing );
HELP_DATA *get_help(CHAR_DATA *ch, const char *argument);
int     can_who                  ( CHAR_DATA *ch, CHAR_DATA *vch) ;
bool IsOnNetwork(CHAR_DATA * ch);

/* act_move.c */
void look_from_dir(CHAR_DATA* ch, int dir);
void	clear_vrooms	 ( void ) ;
ExitData *	find_door	 ( CHAR_DATA *ch, const char *arg, bool quiet ) ;
ExitData *	get_exit	 ( ROOM_INDEX_DATA *room, sh_int dir ) ;
ExitData *	get_exit_to	 ( ROOM_INDEX_DATA *room, sh_int dir, int vnum ) ;
ExitData *	get_exit_num	 ( ROOM_INDEX_DATA *room, sh_int count ) ;
ch_ret	move_char	 ( CHAR_DATA *ch, ExitData *pexit, int fall ) ;
void	teleport	 ( CHAR_DATA *ch, int room, int flags ) ;
sh_int	encumbrance	 ( CHAR_DATA *ch, sh_int move ) ;
bool	will_fall	 ( CHAR_DATA *ch, int fall ) ;
bool has_key(CHAR_DATA*, int);
void remove_bexit_flag(ExitData*, int);
void mob_open(CHAR_DATA*, ExitData*);
void exit_act(sh_int, const char*, CHAR_DATA*, int, bool, int);
char *exit_act_string(const char*, CHAR_DATA*, CHAR_DATA*, int, bool);
const char *  rev_exit     ( sh_int vdir ) ;



/* act_obj.c */

obj_ret	damage_obj	 ( Object *obj ) ;
sh_int	get_obj_resistance  ( Object *obj ) ;
void    save_clan_storeroom  ( CHAR_DATA *ch, ClanData *clan ) ;
void    save_storeitems_room( CHAR_DATA *ch );
void    save_buried_items(CHAR_DATA *ch);
void    obj_fall  	 ( Object *obj, bool through ) ;
/* Ksilyan */
void	UpdateObjectHolders (Object * obj, CHAR_DATA * ch);
void	CheckObjectHolders (Object * obj, CHAR_DATA * ch);

/* act_wiz.c */
void    television_talk ( Object *obj, const char * text );
RID *	find_location	 ( CHAR_DATA *ch, char *arg ) ;
void	echo_to_all	( sh_int AT_COLOR, const char *argument,
				sh_int tar );
void    echo_to_room(sh_int AT_COLOR, ROOM_INDEX_DATA *room, const char* argument);
void   	get_reboot_string  ( void ) ;
struct tm *update_time   ( struct tm *old_time ) ;
void	free_social	 ( SOCIALTYPE *social ) ;
void	add_social	 ( SOCIALTYPE *social ) ;
void	free_command	 ( CMDTYPE *command ) ;
void	unlink_command	 ( CMDTYPE *command ) ;
void	add_command	 ( CMDTYPE *command ) ;

struct qvar
{
  int value;
  const char *desc;
};
#define QVAR_LOGWALKS 0
#define QVAR_NOPLAYERLOOT 1
#define QVAR_XX2 2

#define QVAR_MAX 3
extern struct qvar qvar[QVAR_MAX];

/* bank.c - Ksilyan */
int GetGemWorth(Object * gem);
void InitGems(int base);
void PopulateGemIndex();
void InitGemValues(int base);
Object * RandomGemDrop(CHAR_DATA * ch);

/* boards.c */
void	load_boards	 ( void ) ;
BD *	get_board	 ( Object *obj ) ;
void	free_note	 ( NoteData *pnote ) ;
void    write_boards_txt ( void );
void mail_count(CHAR_DATA *ch); // see if a player has any mail waiting

/* build.c */
ACCOUNT_DATA* get_account(Character *ch, const char* argument);
int get_accountflag(const char* flag);
int get_actflag(const char* flag);
int get_plrflag(const char* flag);
int get_pcflag(const char* flag);
int get_risflag(const char *flag);
const char *	flag_string	 ( int bitvector, const char * const flagarray[] ) ;
int	get_mpflag	 ( const char *flag ) ;
int	get_dir		 ( const char *txt  ) ;
const char *	strip_cr	 ( const char *str  ) ;

/* clans.c */
CL *	get_clan	( const char *name );
void	load_clans	( void );
void	save_clan	 ( ClanData *clan ) ;

CO *	get_council	( const char *name );
void	load_councils	 ( void ) ;
void 	save_council	 ( CouncilData *council ) ;

/* deity.c */
Object* check_for_deity_obj(Object* first_obj);
DeityData *	get_deity	 ( const char *name ) ;
void	load_deity	 ( void ) ;
void	save_deity	 ( DeityData *deity ) ;

/* comm.c */
int     dotted_to_vnum   ( int bvnum, const char* str) ;
char*   vnum_to_dotted   ( int vnum ) ;
const char*   random_password  ( ) ;
//void    init_descriptor  ( DESCRIPTOR_DATA *dnew, int desc) ;
//void	close_socket	 ( DESCRIPTOR_DATA *dclose, bool force ) ;
bool    write_to_descriptor  (int desc, char *txt, int length) ;
//void	write_to_buffer	( DESCRIPTOR_DATA *d, const char *txt,
//				int length );
//void	write_to_pager	( DESCRIPTOR_DATA *d, const char *txt,
//				int length );
int make_color_sequence(const char *col, char *buf, PlayerConnection *d);
int make_color_sequence(const char * col, std::ostringstream & os, PlayerConnection * d);
void	send_to_char_nocolor	( const char *txt, CHAR_DATA *ch );
void	send_to_char_color	( const char *txt, CHAR_DATA *ch );
void	send_to_pager_nocolor	( const char *txt, CHAR_DATA *ch );
void	send_to_pager_color	( const char *txt, CHAR_DATA *ch );
void	set_char_color  ( sh_int AType, CHAR_DATA *ch );
const char*	get_char_color  ( sh_int AType, CHAR_DATA *ch );
void	set_pager_color	( sh_int AType, CHAR_DATA *ch );
void	ch_printf	( CHAR_DATA *ch, const char *fmt, ... );
void    ch_printf_nocolor(CHAR_DATA *ch, const char *fmt, ...);
void	pager_printf	(CHAR_DATA *ch, const char *fmt, ...);
void	log_printf	(const char *fmt, ...);

extern int act_check_sneak; /* external parameter override */
void	act		 ( sh_int AType, const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type );
void	copyover_recover	 (void) ;
const char *  myobj        ( Object *obj ) ;

extern const  unsigned char    echo_off_str    [];

void trace(CHAR_DATA *ch, const char *cmdline);
void	display_prompt		 ( PlayerConnection *d ) ;

const char * ColorNumberToChars( short color ); // color.cpp

/* reset.c */
RD  *	make_reset	 ( char letter, int extra, int arg1, int arg2, int arg3 ) ;
RD  *	add_reset	 ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) ;
RD  *	place_reset	 ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) ;
void	reset_area	 ( AREA_DATA * pArea ) ;

/* db.c */
void    recount_greetings();
void    load_storeitems_room(const char* filename);
void    load_buried_items ();
void    load_storeitems_rooms ();
void	show_file	 ( CHAR_DATA *ch, const char *filename ) ;
char *	str_dup		 ( char const *str ) ;
void	boot_db		 ( ) ;
void	area_update	 ( void ) ;
void	add_char	 ( CHAR_DATA *ch ) ;
CD *	create_mobile	 ( MOB_INDEX_DATA *pMobIndex ) ;
OD *	create_object	 ( OBJ_INDEX_DATA *pObjIndex, int level ) ;
void	clear_char	 ( CHAR_DATA *ch ) ;
void	free_char	 ( const CHAR_DATA * ch ) ;
void	free_char	 ( CHAR_DATA *& ch ) ;
const char *	get_extra_descr	 ( const char *name, ExtraDescData *ed ) ;
MID *	get_mob_index	 ( int vnum ) ;
OID *	get_obj_index	 ( int vnum ) ;
RID *	get_room_index	 ( int vnum ) ;
char	fread_letter	 ( FILE *fp ) ;
int	fread_number	 ( FILE *fp ) ;
char *	fread_string	 ( FILE *fp ) ;
char *	fread_string_nohash  ( FILE *fp ) ;
std::string fread_string_noheap( FILE *fp );
void	fread_to_eol	 ( FILE *fp ) ;
char *	fread_word	 ( FILE *fp ) ;
char *	fread_line	 ( FILE *fp ) ;
int	number_fuzzy	 ( int number ) ;
int	number_range	 ( int from, int to ) ;
int	number_percent	 ( void ) ;
int	number_door	 ( void ) ;
int	number_bits	 ( int width ) ;
int	number_mm	 ( void ) ;
int	dice		 ( int number, int size ) ;
int	interpolate	 ( int level, int value_00, int value_32 ) ;
void	smash_tilde	 ( char *str ) ;
const char* smash_tilde_static	 ( const char *str ) ;
std::string SmashTilde (const char* str);
std::string SmashTilde (const std::string& str);
void	hide_tilde	 ( char *str ) ;
char *	show_tilde	 ( char *str ) ;
bool	str_cmp		 ( const char *astr, const char *bstr ) ;
bool	str_prefix	 ( const char *astr, const char *bstr ) ;
bool	str_infix	 ( const char *astr, const char *bstr ) ;
bool	str_suffix	 ( const char *astr, const char *bstr ) ;
const char *	capitalize	 ( const char *str ) ;
const char *	strlower	 ( const char *str ) ;
const char *	strupper	 ( const char *str ) ;
const char *  aoran		 ( const char *str ) ;
void	append_file	 ( CHAR_DATA *ch, const char *file, const char *str ) ;
void	append_to_file	 ( const char *file, const char *str ) ;
void	bug		 ( const char *str, ... ) ;
void    lbug     ( int, const char * str, ... ) ;
void	log_string_plus	 ( const char *str, sh_int log_type, sh_int level ) ;
void	log_multiplaying ( char * str );
RID *	make_room	 ( int vnum ) ;
OID *	make_object	 ( int vnum, int cvnum, const char *name ) ;
MID *	make_mobile	 ( int vnum, int cvnum, const char *name ) ;
ExitData *	make_exit	 ( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door ) ;
void	add_help	 ( HELP_DATA *pHelp ) ;
void	fix_area_exits	 ( AREA_DATA *tarea ) ;
void	load_area_file	 ( AREA_DATA *tarea, const char *filename ) ;
void	randomize_exits	 ( ROOM_INDEX_DATA *room, sh_int maxdir ) ;
void	make_wizlist	 ( void ) ;
void	tail_chain	 ( void ) ;
bool    delete_room      ( ROOM_INDEX_DATA *room ) ;
bool    delete_obj       ( OBJ_INDEX_DATA *obj ) ;
bool    delete_mob       ( MOB_INDEX_DATA *mob ) ;
/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	 ( MOB_INDEX_DATA *pMob ) ;
void	obj_sort	 ( OBJ_INDEX_DATA *pObj ) ;
void	room_sort	 ( ROOM_INDEX_DATA *pRoom ) ;*/
void	sort_area	 ( AREA_DATA *pArea, bool proto ) ;
/* Add a room into the auction room list. 08.08.2000 -- Warp */
void    add_auctionroom (ROOM_INDEX_DATA *room);

/* build.c */
void	start_editing	 ( CHAR_DATA *ch, const char *data ) ;
void	stop_editing	 ( CHAR_DATA *ch ) ;
void	edit_buffer	 ( CHAR_DATA *ch, const char *argument ) ;
char *	copy_buffer	 ( CHAR_DATA *ch ) ;
std::string copy_buffer_string ( Character * ch );
bool	can_rmodify	 ( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) ;
bool	can_omodify	 ( CHAR_DATA *ch, Object *obj  ) ;
bool	can_mmodify	 ( CHAR_DATA *ch, CHAR_DATA *mob ) ;
bool	can_medit	 ( CHAR_DATA *ch, MOB_INDEX_DATA *mob ) ;
void	free_reset	 ( AREA_DATA *are, RESET_DATA *res ) ;
void	free_area	 ( AREA_DATA *are ) ;
void	assign_area	 ( CHAR_DATA *ch ) ;
EDD *	SetRExtra	 ( ROOM_INDEX_DATA *room, const char *keywords ) ;
bool	DelRExtra	 ( ROOM_INDEX_DATA *room, const char *keywords ) ;
EDD *	SetOExtra	 ( Object *obj, const char *keywords ) ;
bool	DelOExtra	 ( Object *obj, const char *keywords ) ;
EDD *	SetOExtraProto	 ( OBJ_INDEX_DATA *obj, const char *keywords ) ;
bool	DelOExtraProto	 ( OBJ_INDEX_DATA *obj, const char *keywords ) ;
void	fold_area	 ( AREA_DATA *tarea, const char *filename, bool install ) ;
int	get_otype	 ( const char *type ) ;
int	get_atype	 ( const char *type ) ;
int	get_aflag	 ( const char *flag ) ;
int	get_oflag	 ( const char *flag ) ;
int	get_wflag	 ( const char *flag ) ;
int     playerbuilderallowed  ( CHAR_DATA *ch ) ; /* restrictions */
int     playerbuilderbadvnum  ( CHAR_DATA *ch, int vnum ) ;
int     playerbuildercannot   ( CHAR_DATA *ch ) ; /* refuse players */
int     is_playbuilding  ( CHAR_DATA *ch ) ;
void    do_playbuild  ( CHAR_DATA *ch, const char *arg ) ;

/* fight.c */
int     get_weapon_prof_gsn(int weapon_type);
ch_ret  projectile_hit  ( CHAR_DATA *ch, CHAR_DATA *victim, Object *wield,
                            Object *projectile, sh_int dist );
uint	max_fight	 ( CHAR_DATA *ch ) ;
void	violence_update	 ( void ) ;

bool    MeeleeHit(CHAR_DATA * ch, CHAR_DATA * victim, Object * Wielded, short HitBonus);

ch_ret	multi_hit	 ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) ;
sh_int	ris_damage	 ( CHAR_DATA *ch, sh_int dam, int ris ) ;
ch_ret	damage		( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, int damtype );
void	update_pos	 ( CHAR_DATA *victim ) ;
//void	set_fighting	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
//void	stop_fighting	 ( CHAR_DATA *ch, bool fBoth ) ;
//void	free_fight	 ( CHAR_DATA *ch ) ;
//CD *	who_fighting	 ( CHAR_DATA *ch ) ;
void	death_cry	 ( CHAR_DATA *ch ) ;
void	stop_hunting	 ( CHAR_DATA *ch ) ;
void	stop_hating	 ( CHAR_DATA *ch ) ;
void	stop_fearing	 ( CHAR_DATA *ch ) ;
void	start_hunting	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
void	start_hating	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
void	start_fearing	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool	is_hunting	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool	is_hating	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool	is_fearing	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool	is_safe		 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
sh_int	VAMP_AC		 ( const Character *ch ) ;
void    raw_kill         ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool	in_arena	 ( CHAR_DATA *ch ) ;

/* makeobjs.c */
void	make_corpse	 ( CHAR_DATA *ch, CHAR_DATA *killer ) ;
void	make_blood	 ( CHAR_DATA *ch ) ;
void	make_bloodstain  ( CHAR_DATA *ch ) ;
void	make_scraps	 ( Object *obj ) ;
void	make_fire	 ( ROOM_INDEX_DATA *in_room, sh_int timer) ;
OD *	make_trap	 ( int v0, int v1, int v2, int v3 ) ;
OD *	create_money	 ( int amount ) ;
//Ksilyan:
OD *    create_arrowstack( Object * arrowtype, int count);

/* misc.c */
void actiondesc  ( CHAR_DATA *ch, Object *obj, void *vo ) ;

/* deity.c */
void adjust_favor	 ( CHAR_DATA *ch, int field, int mod ) ;

/* mud_comm.c */
int get_color(const char* argument);
const char *	mprog_type_to_name	 ( int64 type ) ;
void auto_walk(CHAR_DATA*);

/* mud_prog.c */
extern Object * current_obj;
#ifdef DUNNO_STRSTR
char *  strstr                   (const char *s1, const char *s2 ) ;
#endif

void	rprog_command_check    ( char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, Object* object,
					void* vo, int64 type );
int     mprog_command_check    ( char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, Object* object,
					void* vo, bool bExact );
void	mprog_wordlist_check    ( const char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, Object* object,
					void* vo, int64 type );
void	mprog_percent_check     ( CHAR_DATA *mob, CHAR_DATA* actor,
					Object* object, void* vo,
					int64 type );
void	mprog_act_trigger       ( char* buf, CHAR_DATA* mob,
		                        CHAR_DATA* ch, Object* obj,
					void* vo );
void	mprog_bribe_trigger     ( CHAR_DATA* mob, CHAR_DATA* ch,
		                        int amount );
void	mprog_entry_trigger      ( CHAR_DATA* mob ) ;
void	mprog_give_trigger      ( CHAR_DATA* mob, CHAR_DATA* ch,
                		        Object* obj );
void	mprog_greet_trigger      ( CHAR_DATA* mob ) ;
void    mprog_greet_dir_trigger  ( CHAR_DATA* mob, int dir ) ;
void    rprog_greet_dir_trigger   ( CHAR_DATA* ch, int dir ) ;
void    oprog_greet_dir_trigger  ( CHAR_DATA * ch, int dir ) ;
void    mprog_fight_trigger      ( CHAR_DATA* mob, CHAR_DATA* ch ) ;
void    mprog_hitprcnt_trigger   ( CHAR_DATA* mob, CHAR_DATA* ch ) ;
void    mprog_death_trigger      ( CHAR_DATA *killer, CHAR_DATA* mob ) ;
void    mprog_random_trigger     ( CHAR_DATA* mob ) ;
void    mprog_speech_trigger     ( const char* txt, CHAR_DATA* mob ) ;
void    mprog_yell_trigger     ( const char* txt, CHAR_DATA* mob ) ;
void mpwalk_finished_trigger   ( CHAR_DATA *mob, bool bSuccess ) ;
void    mprog_script_trigger     ( CHAR_DATA *mob ) ;
void    mprog_hour_trigger       ( CHAR_DATA *mob ) ;
void    mprog_time_trigger       ( CHAR_DATA *mob ) ;
void    mprog_get_trigger      (CHAR_DATA*, Object*) ;

//Ksilyan:
void    mprog_steal_trigger      (CHAR_DATA* mob) ;
void    mprog_look_trigger		 (CHAR_DATA* mob, CHAR_DATA* victim) ;
void    mprog_otherdeath_trigger     (CHAR_DATA * actor) ;
bool	rprog_search_trigger	 (CHAR_DATA* ch) ;

void    progbug                  ( const char *str, CHAR_DATA *mob ) ;
void set_supermob( Object *obj);
void	rset_supermob		 ( ROOM_INDEX_DATA *room) ;
void	release_supermob	 ( ) ;

/* player.c */
void	set_title	 ( CHAR_DATA *ch, const std::string& title ) ;
const char*   positionstr     (CHAR_DATA *ch);
const char*   alignstr        (CHAR_DATA *ch);
const char*   armorstr        (CHAR_DATA *ch);
const char*   hitrollstr      (CHAR_DATA *ch);
const char*   damrollstr      (CHAR_DATA *ch);
const char * StatStr(Character * ch, const char * statName, int stat, const char * const* names);
void    print_states    (CHAR_DATA *ch);

/* skills.c */
bool	check_skill		 ( CHAR_DATA *ch, const char *command, const char *argument ) ;
void	learn_from_success	 ( CHAR_DATA *ch, int sn ) ;
void	learn_from_failure	 ( CHAR_DATA *ch, int sn ) ;
bool	check_parry		 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool	check_dodge		 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool 	check_grip		 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
void	disarm			 ( CHAR_DATA *ch, CHAR_DATA *victim) ;
void	trip			 ( CHAR_DATA *ch, CHAR_DATA *victim) ;
bool    mob_fire         ( CHAR_DATA *ch, char *name ) ;
CD *    scan_for_victim     ( CHAR_DATA *ch, ExitData *pexit,
                             char *name );

sh_int GetCountInQuiver(Object * quiver);
void ProjectileDecreaseCondition(Object * projectile);
int GetRangedProficiency(CHAR_DATA * ch, int gsn);

/* translate.c - Ksilyan */
const char * 	sector_number_to_name 		(int sectortype);
int 	sector_name_to_number 		(const char * sectorname);
const char * 	exit_number_to_name 		(int exitflags);
int 	exit_name_to_number 		(const char * exittext);
const char * 	ammotype_number_to_name		(int ammotype);
int 	ammotype_name_to_number		(const char * ammoname);
const char * 	weapontype_number_to_name	(int weaponnumber);
int 	weapontype_name_to_number	(const char * weaponname);
int weapontype_name_to_number_nospaces (const char * weaponname);
const char *	ris_number_to_name 			(int risflags);
int		ris_name_to_number 			(const char * ristext);
const char *	itemtype_number_to_name		(int itemtype);
int		itemtype_name_to_number		(const char * itemname);
const char *	damagemsg_number_to_name	(int damagemsg, int damage);
int		damagemsg_name_to_number	(const char * damagetext);
const char *	damagetype_number_to_name	(int damagetype);
int		damagetype_name_to_number	(const char * damagetext);


int		itemtype_old_to_new			(int oldtype);
int 	weapontype_to_ammotype		(int weapontype);
int		weapontype_to_damagetype	(int weapontype);
int		weapontype_to_damagemsg		(int damagetype);
int		ammotype_to_weapontype		(int ammotype);
int		ammotype_to_damagemessage	(int ammotype);
int		ammotype_to_damagetype		(int ammotype);

const char *	damage_number_to_name		(int damage);
const char *	power_number_to_name		(int power);

const char *  gem_size_number_to_name     (sh_int size);
const char *  gem_quality_number_to_name  (sh_int size);

sh_int		bodypart_to_wearloc			(sh_int bodypart);

/* handler.c */
CHAR_DATA* get_obj_carried_by(Object* obj);
int	get_exp		 ( CHAR_DATA *ch ) ;
int get_exp_worth_for_level( CHAR_DATA * ch, int level);
int	get_exp_worth	 ( CHAR_DATA *ch ) ;
int	exp_level	 ( CHAR_DATA *ch, sh_int level ) ;
sh_int	get_trust	 ( CHAR_DATA *ch ) ;
sh_int	get_age		 ( CHAR_DATA *ch ) ;
bool	can_take_proto	 ( CHAR_DATA *ch ) ;
int	can_carry_n	 ( CHAR_DATA *ch ) ;
int	can_carry_w	 ( CHAR_DATA *ch ) ;
char*   remove_name  ( const char *str, char *namelist ) ;
bool	is_name		 ( const char *str, const char *namelist ) ;
bool	is_name_prefix	 ( const char *str, const char *namelist ) ;
bool	nifty_is_name	 ( const char *str, const char *namelist ) ;
bool	nifty_is_name_prefix  ( const char *str, const char *namelist ) ;
const char    * strip_the  ( const char *name) ;
const char    * strip_a  ( const char *name) ;
void	affect_modify	 ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd, CHAR_DATA * caster = NULL ) ;
void	affect_to_char	 ( CHAR_DATA *ch, AFFECT_DATA *paf, CHAR_DATA * caster = NULL ) ;
void	affect_remove	 ( CHAR_DATA *ch, AFFECT_DATA *paf ) ;
void	affect_strip	 ( CHAR_DATA *ch, int sn ) ;
void	affect_strip_one  ( CHAR_DATA *ch, int sn ) ;
bool	is_affected	 ( CHAR_DATA *ch, int sn ) ;
void	affect_join	 ( CHAR_DATA *ch, AFFECT_DATA *paf ) ;
void	char_from_room	 ( CHAR_DATA *ch ) ;
void	char_to_room	 ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) ;
OD *	obj_to_char	 ( Object *obj, CHAR_DATA *ch ) ;
void	obj_from_char	 ( Object *obj ) ;
int	apply_ac	 ( Object *obj, int iWear ) ;
OD *	get_eq_char	 ( CHAR_DATA *ch, int iWear ) ;
void	equip_char	 ( CHAR_DATA *ch, Object *obj, int iWear ) ;
void	unequip_char	 ( CHAR_DATA *ch, Object *obj ) ;
int	count_obj_list	 ( OBJ_INDEX_DATA *obj, Object *List ) ;
void	obj_from_room	 ( Object *obj ) ;
OD *	obj_to_room	 ( Object *obj, ROOM_INDEX_DATA *pRoomIndex ) ;
OD *	obj_to_obj	 ( Object *obj, Object *obj_to ) ;
void	obj_from_obj	 ( Object *obj ) ;

void	extract_obj	 ( Object *obj, bool bDecrement ) ;

void	extract_exit	 ( ROOM_INDEX_DATA *room, ExitData *pexit ) ;
void	extract_room	 ( ROOM_INDEX_DATA *room ) ;
void	clean_room	 ( ROOM_INDEX_DATA *room ) ;
void	clean_obj	 ( OBJ_INDEX_DATA *obj ) ;
void	clean_mob	 ( MOB_INDEX_DATA *mob ) ;
void	clean_resets	 ( AREA_DATA *tarea ) ;
void	extract_char	 ( CHAR_DATA *ch, bool fPull ) ;
CD *	get_char_room	 ( CHAR_DATA *ch, const char *argument ) ;
CD *	get_char_world	 ( CHAR_DATA *ch, const char *argument ) ;
OD *	get_obj_type	 ( OBJ_INDEX_DATA *pObjIndexData ) ;
OD *	get_obj_list	 ( CHAR_DATA *ch, const char *argument,
			    Object *List );
OD *	get_obj_list_rev  ( CHAR_DATA *ch, const char *argument,
			    Object *List );
OD *	get_obj_carry	 ( CHAR_DATA *ch, const char *argument ) ;
OD *	get_obj_wear	 ( CHAR_DATA *ch, const char *argument ) ;
OD *	get_obj_here	 ( CHAR_DATA *ch, const char *argument ) ;
OD *	get_obj_world	 ( CHAR_DATA *ch, const char *argument ) ;
int	get_obj_number	 ( Object *obj ) ;
int	get_obj_weight	 ( Object *obj ) ;
bool	room_is_dark	 ( ROOM_INDEX_DATA *pRoomIndex ) ;
bool	room_is_private	 ( ROOM_INDEX_DATA *pRoomIndex ) ;
bool	can_see		 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
bool	can_see_obj	 ( CHAR_DATA *ch, Object *obj ) ;
bool	can_drop_obj	 ( CHAR_DATA *ch, Object *obj ) ;
const char *	item_type_name	 ( Object *obj ) ;
const char *	affect_loc_name	 ( int location ) ;
const char *	affect_bit_name	 ( int vector ) ;
const char *	extra_bit_name	 ( int extra_flags ) ;
const char *	magic_bit_name	 ( int magic_flags ) ;
ch_ret	check_for_trap	 ( CHAR_DATA *ch, Object *obj, int flag ) ;
ch_ret	check_room_for_traps  ( CHAR_DATA *ch, int flag ) ;
bool	is_trapped	 ( Object *obj ) ;
OD *	get_trap	 ( Object *obj ) ;
ch_ret	spring_trap      ( CHAR_DATA *ch, Object *obj ) ;
void	name_stamp_stats  ( CHAR_DATA *ch ) ;
void	fix_char	 ( CHAR_DATA *ch ) ;
void	showaffect	 ( CHAR_DATA *ch, AFFECT_DATA *paf ) ;
void	set_cur_obj	 ( Object *obj ) ;
bool	obj_extracted	 ( Object *obj ) ;
void	queue_extracted_obj	 ( Object *obj ) ;
void	clean_obj_queue	 ( void ) ;
void	set_cur_char	 ( CHAR_DATA *ch ) ;
bool	char_died	 ( CHAR_DATA *ch ) ;
void	queue_extracted_char	 ( CHAR_DATA *ch, bool extract ) ;
void	clean_char_queue	 ( void ) ;
void	add_timer	 ( CHAR_DATA *ch, sh_int type, sh_int count, DO_FUN *fun, int value ) ;
TIMER * get_timerptr	 ( CHAR_DATA *ch, sh_int type ) ;
sh_int	get_timer	 ( CHAR_DATA *ch, sh_int type ) ;
void	extract_timer	 ( CHAR_DATA *ch, TIMER *timer ) ;
void	remove_timer	 ( CHAR_DATA *ch, sh_int type ) ;
bool	in_soft_range	 ( CHAR_DATA *ch, AREA_DATA *tarea ) ;
bool	in_hard_range	 ( CHAR_DATA *ch, AREA_DATA *tarea ) ;
OD *	clone_object	 ( Object *obj ) ;
void	split_obj	 ( Object *obj, int num ) ;
void	separate_obj	 ( Object *obj ) ;
bool	empty_obj	( Object *obj, Object *destobj,
				ROOM_INDEX_DATA *destroom );
OD *	find_obj	( CHAR_DATA *ch, const char *argument,
				bool carryonly );
bool	ms_find_obj	 ( CHAR_DATA *ch ) ;
void	worsen_mental_state  ( CHAR_DATA *ch, int mod ) ;
void	better_mental_state  ( CHAR_DATA *ch, int mod ) ;
void	boost_economy	 ( AREA_DATA *tarea, int gold ) ;
void	lower_economy	 ( AREA_DATA *tarea, int gold ) ;
void	economize_mobgold  ( CHAR_DATA *mob ) ;
bool	economy_has	 ( AREA_DATA *tarea, int gold ) ;
void	add_kill	 ( CHAR_DATA *ch, CHAR_DATA *mob ) ;
int	times_killed	 ( CHAR_DATA *ch, CHAR_DATA *mob ) ;
/* Testaur's additions  */
bool    is_contagious(CHAR_DATA *ch);
void    fix_affected_by(CHAR_DATA *ch);

/* interp.c */
bool	check_pos	 ( CHAR_DATA *ch, sh_int position ) ;
void	interpret	 ( Character * ch, const char *argument ) ;
bool	is_number	 ( const char *arg ) ;
int	number_argument	 ( const char *argument, char *arg ) ;
char *	one_argument	( const char *argument, char *arg_first );
char *	one_argument2	( const char *argument, char *arg_first );
ST *	find_social	 ( const char *command ) ;
CMDTYPE *find_command	 ( const char *command ) ;
void	hash_commands	 ( ) ;
void	start_timer	 ( struct timeval *stime ) ;
time_t	end_timer	 ( struct timeval *stime ) ;
void	send_timer	 ( struct timerset *vtime, CHAR_DATA *ch ) ;
void	update_userec	 ( struct timeval *time_used,
				struct timerset *userec );

/* magic.c */
bool process_spell_components  	( CHAR_DATA *ch, int sn ) ;
int	ch_slookup	 				( CHAR_DATA *ch, const char *name ) ;
int	find_spell	 				( CHAR_DATA *ch, const char *name, bool know ) ;
int	find_skill	 				( CHAR_DATA *ch, const char *name, bool know ) ;
int	find_weapon	 				( CHAR_DATA *ch, const char *name, bool know ) ;
int	find_tongue	 				( CHAR_DATA *ch, const char *name, bool know ) ;
int	skill_lookup	 			( const char *name ) ;
int	herb_lookup	 				( const char *name ) ;
int	personal_lookup	 			( CHAR_DATA *ch, const char *name ) ;
int	slot_lookup	 				( int slot ) ;
int	bsearch_skill	 			( const char *name, int first, int top ) ;
int	bsearch_skill_exact  		( const char *name, int first, int top ) ;
bool saves_poison_death	 		( int level, CHAR_DATA *victim ) ;
bool saves_wand		 			( int level, CHAR_DATA *victim ) ;
bool saves_para_petri	 		( int level, CHAR_DATA *victim ) ;
bool saves_breath		 		( int level, CHAR_DATA *victim ) ;
bool saves_spell_staff	 		( int level, CHAR_DATA *victim ) ;
ch_ret obj_cast_spell	 		( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, Object *obj ) ;
int	dice_parse	 				(CHAR_DATA *ch, int level, char *exp) ;
SK * get_skilltype	 			( int sn ) ;
void spread_contagion			(CHAR_DATA *ch, CHAR_DATA *victim );
OBJ_DATA * find_vnum_component	( CHAR_DATA * ch, int vnum );

extern const char * target_name; // Kludgy fix on a cludgier problem.

/* request.c */
void	init_request_pipe	 ( void ) ;
void	check_requests		 ( void ) ;

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
#define OS_BURIED       2
#define OS_CARRY_NOINC  3
void	save_char_obj	 ( CHAR_DATA *ch ) ;
bool	load_char_obj	 ( PlayerConnection *d, const char *name, bool preload ) ;
void    free_account_data (ACCOUNT_DATA *act);
ACCOUNT_DATA *load_account_data (const char* address);
ACCOUNT_DATA *find_account_data (const char* address);
void write_account_data (ACCOUNT_DATA* act);
void	set_alarm	 ( long seconds ) ;
void	requip_char	 ( CHAR_DATA *ch ) ;
void    fread_char(CHAR_DATA *ch, FILE*fp, bool preload);
void    fwrite_obj      ( CHAR_DATA *ch,  Object  *obj, FILE *fp,
				int iNest, sh_int os_type );
void	fread_obj	 ( CHAR_DATA *ch,  FILE *fp, sh_int os_type ) ;
void	de_equip_char	 ( CHAR_DATA *ch ) ;
void	re_equip_char	 ( CHAR_DATA *ch ) ;

/* comments.c */
void fread_comment( ACCOUNT_DATA *pact, FILE *fp );
void fwrite_comments( ACCOUNT_DATA *pact, FILE *fp );

/* shops.c */

/* special.c */
SF *	spec_lookup	 ( const char *name ) ;
const char *	lookup_spec	 ( SPEC_FUN *special ) ;

/* tables.c */
const char * spell_name		( SPELL_FUN *spell );
const char * skill_name		( DO_FUN *skill );
int		get_skill	 		( const char *skilltype );
void	load_skill_table  	( void );
void	save_skill_table  	( void );
void	sort_skill_table  	( void );
void	load_socials	 	( void );
void	save_socials	 	( void );
void	load_commands	 	( void );
void	save_commands	 	( void );
SPELL_FUN *spell_function  	( const char *name );
DO_FUN *skill_function   	( const char *name );
void	write_class_file  	( int cl );
void	save_classes	 	( void );
void	load_classes	 	( void );
void	load_herb_table	 	( void );
void	save_herb_table	 	( void );
void 	read_last_file		( CHAR_DATA *ch, int count, char *name, int file );
void 	write_last_file		( char *entry, int file );

/* track.c */
void	found_prey	 ( CHAR_DATA *ch, CHAR_DATA *victim ) ;
void	hunt_victim	 ( CHAR_DATA *ch) ;
int find_first_step(ROOM_INDEX_DATA*, ROOM_INDEX_DATA*, int);

/* update.c */
void    deadvance_level  ( CHAR_DATA *ch ) ;
void	advance_level	 ( CHAR_DATA *ch ) ;
void    gain_forage (CHAR_DATA *ch);
void	gain_exp	 ( CHAR_DATA *ch, int gain, bool bAccumulate ) ;
void	gain_condition	 ( CHAR_DATA *ch, int iCond, int value ) ;
void	update_handler	 ( void ) ;
void	reboot_check	 ( time_t secReset ) ;
#if 0
void    reboot_check     ( char *arg ) ;
#endif
void    auction_update   ( void ) ;
void	remove_portal	 ( Object *portal ) ;
int     char_in_auctionroom (CHAR_DATA* ch);
void    donate_auctioned_item (void);
int     sunrise_hour     ( void ) ;
int     sunset_hour      ( void ) ;
int     sun_broken       ( void ) ;
#define SUN_BROKEN_VAMPEFFECT 3 /* out of 4 chance that blood goes bad */

/* hashstr.c */
char *	str_alloc	 ( const char *str ) ;
char *	quick_link	 ( char *str ) ;
int	str_free	 ( char *str ) ;
void	show_hash	 ( int count ) ;
char *	hash_stats	 ( void ) ;
const char *	check_hash	 ( const char *str ) ;
void	hash_dump	 ( int hash ) ;
void	show_high_hash	 ( int top ) ;

/* newscore.c */
const char *  get_class 	 (CHAR_DATA *ch) ;
const char *  get_race 	 (CHAR_DATA *ch) ;

/* new_dump.c */
void do_new_dump( CHAR_DATA *ch, const char *argument );


void  start_auth( PlayerConnection *d );
void stop_idling( CHAR_DATA *ch );

#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 *
 *  New Build Interface Stuff Follows
 *
 * Commented by Ksilyan
 */


/*
 *  Data for a menu page
 */

/*
struct	menu_data
{
    char		*sectionNum;
    char		*charChoice;
    int			x;
    int			y;
    char		*outFormat;
    void		*data;
    int			ptrType;
    int			cmdArgs;
    char		*cmdString;
};

DECLARE_DO_FUN( do_redraw_page  );
DECLARE_DO_FUN( do_refresh_page );
DECLARE_DO_FUN( do_pagelen	);
DECLARE_DO_FUN( do_omenu  	);
DECLARE_DO_FUN( do_rmenu  	);
DECLARE_DO_FUN( do_mmenu  	);
DECLARE_DO_FUN( do_clear  	);

extern		MENU_DATA		room_page_a_data[];
extern		MENU_DATA		room_page_b_data[];
extern		MENU_DATA		room_page_c_data[];
extern		MENU_DATA		room_help_page_data[];

extern		MENU_DATA		mob_page_a_data[];
extern		MENU_DATA		mob_page_b_data[];
extern		MENU_DATA		mob_page_c_data[];
extern		MENU_DATA		mob_page_d_data[];
extern		MENU_DATA		mob_page_e_data[];
extern		MENU_DATA		mob_page_f_data[];
extern		MENU_DATA		mob_help_page_data[];

extern		MENU_DATA		obj_page_a_data[];
extern		MENU_DATA		obj_page_b_data[];
extern		MENU_DATA		obj_page_c_data[];
extern		MENU_DATA		obj_page_d_data[];
extern		MENU_DATA		obj_page_e_data[];
extern		MENU_DATA		obj_help_page_data[];

extern		MENU_DATA		control_page_a_data[];
extern		MENU_DATA		control_help_page_data[];

extern	const   char    room_page_a[];
extern	const   char    room_page_b[];
extern	const   char    room_page_c[];
extern	const   char    room_help_page[];

extern	const   char    obj_page_a[];
extern	const   char    obj_page_b[];
extern	const   char    obj_page_c[];
extern	const   char    obj_page_d[];
extern	const   char    obj_page_e[];
extern	const   char    obj_help_page[];

extern	const   char    mob_page_a[];
extern	const   char    mob_page_b[];
extern	const   char    mob_page_c[];
extern	const   char    mob_page_d[];
extern	const   char    mob_page_e[];
extern	const   char    mob_page_f[];
extern	const   char    mob_help_page[];
extern	const   char *  npc_sex[3];
extern	const   char *  ris_strings[];

extern	const   char    control_page_a[];
extern	const   char    control_help_page[];

#define SH_INT 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5


#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19

#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4

#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST
*/
/*
 * defines for use with this get_affect function
 */
/*
#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30
*/


/*
 *   Map Structures
 */

DECLARE_DO_FUN( do_mapout 	);
DECLARE_DO_FUN( do_lookmap	);

struct  map_data	/* contains per-room data */
{
  int vnum;		/* which map this room belongs to */
  int x;		/* horizontal coordinate */
  int y;		/* vertical coordinate */
  char entry;		/* code that shows up on map */
};


struct  map_index_data
{
  MAP_INDEX_DATA  *next;
  int 		  vnum;  		  /* vnum of the map */
  int             map_of_vnums[49][81];   /* room vnums aranged as a map */
};


MAP_INDEX_DATA *get_map_index(int vnum);
void            init_maps();


/*
 * mudprograms stuff
 */
extern	CHAR_DATA *supermob;

void oprog_speech_trigger( const char *txt, CHAR_DATA *ch );
void oprog_random_trigger( Object *obj );
void oprog_wear_trigger( CHAR_DATA *ch, Object *obj );
bool oprog_use_trigger( CHAR_DATA *ch, Object *obj,
                        CHAR_DATA *vict, Object *targ, void *vo );
void oprog_remove_trigger( CHAR_DATA *ch, Object *obj );
void oprog_sac_trigger( CHAR_DATA *ch, Object *obj );
void oprog_damage_trigger( CHAR_DATA *ch, Object *obj );
void oprog_repair_trigger( CHAR_DATA *ch, Object *obj );
void oprog_drop_trigger( CHAR_DATA *ch, Object *obj );
void oprog_zap_trigger( CHAR_DATA *ch, Object *obj );
char *oprog_type_to_name( int type );

/*
 * MUD_PROGS START HERE
 * (object stuff)
 */
void oprog_greet_trigger( CHAR_DATA *ch );
void oprog_get_trigger( CHAR_DATA *ch, Object *obj );
void oprog_examine_trigger( CHAR_DATA *ch, Object *obj );
void oprog_pull_trigger( CHAR_DATA *ch, Object *obj );
void oprog_push_trigger( CHAR_DATA *ch, Object *obj );


/* mud prog defines */

#define ERROR_PROG        ((int64) -1)
#define IN_FILE_PROG       ((int64) 0)
#define ACT_PROG           ((int64) BV00)
#define SPEECH_PROG        ((int64) BV01)
#define RAND_PROG          ((int64) BV02)
#define FIGHT_PROG         ((int64) BV03)
#define RFIGHT_PROG        ((int64) BV03)
#define DEATH_PROG         ((int64) BV04)
#define RDEATH_PROG        ((int64) BV04)
#define HITPRCNT_PROG      ((int64) BV05)
#define ENTRY_PROG         ((int64) BV06)
#define ENTER_PROG         ((int64) BV06)
#define GREET_PROG         ((int64) BV07)
#define RGREET_PROG	   ((int64) BV07)
#define OGREET_PROG        ((int64) BV07)
#define ALL_GREET_PROG	   ((int64) BV08)
#define GIVE_PROG	   ((int64) BV09)
#define BRIBE_PROG	   ((int64) BV10)
#define HOUR_PROG	   ((int64) BV11)
#define TIME_PROG	   ((int64) BV12)
#define WEAR_PROG          ((int64) BV13)
#define REMOVE_PROG        ((int64) BV14)
#define SAC_PROG           ((int64) BV15)
#define LOOK_PROG          ((int64) BV16)
#define EXA_PROG           ((int64) BV17)
#define ZAP_PROG           ((int64) BV18)
#define GET_PROG 	   ((int64) BV19)
#define DROP_PROG	   ((int64) BV20)
#define DAMAGE_PROG	   ((int64) BV21)
#define REPAIR_PROG	   ((int64) BV22)
#define RANDIW_PROG	   ((int64) BV23)
#define SPEECHIW_PROG	   ((int64) BV24)
#define PULL_PROG	   ((int64) BV25)
#define PUSH_PROG	   ((int64) BV26)
#define SLEEP_PROG         ((int64) BV27)
#define REST_PROG          ((int64) BV28)
#define LEAVE_PROG         ((int64) BV29)
#define SCRIPT_PROG	   ((int64) BV30)
#define USE_PROG           (((int64)1)<<31)
#define COMMAND_PROG   ((int64) BV32)
#define GREET_DIR_PROG ((int64) BV33)
#define YELL_PROG      ((int64) BV34)
#define MPWALK_FINISHED_PROG ((int64) BV35)
#define STEAL_PROG      ((int64) BV36)
#define OTHERDEATH_PROG ((int64) BV37)
#define SEARCH_PROG		((int64) BV38)

void rprog_leave_trigger( CHAR_DATA *ch );
void rprog_enter_trigger( CHAR_DATA *ch );
void rprog_sleep_trigger( CHAR_DATA *ch );
void rprog_rest_trigger( CHAR_DATA *ch );
void rprog_rfight_trigger( CHAR_DATA *ch );
void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch );
void rprog_speech_trigger( const char *txt, CHAR_DATA *ch );
void rprog_random_trigger( CHAR_DATA *ch );
void rprog_time_trigger( CHAR_DATA *ch );
void rprog_hour_trigger( CHAR_DATA *ch );
char *rprog_type_to_name( int type );
signed int prog_command_trigger( CHAR_DATA* ch, char *command, bool bExact );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( char *buf, Object *mobj, CHAR_DATA *ch,
			Object *obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
			Object *obj, void *vo );
#endif


#define GET_ADEPT(ch,sn)    (  skill_table[(sn)]->skill_adept[(ch)->Class])
#define send_to_char send_to_char_color
#define send_to_pager send_to_pager_color

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


/* AUTO AUCTION INCREMENT */
#define AUCTION_MINIMUM		10

/* hp regen race modifier */
#define HP_REGEN_HUMAN		0
#define HP_REGEN_ELF		-1
#define HP_REGEN_DWARF		2
#define HP_REGEN_HALFLING	0
#define HP_REGEN_HALF_OGRE	2
#define HP_REGEN_HALF_ORC	2
#define HP_REGEN_HALF_TROLL	1
#define HP_REGEN_HALF_ELF	1
#define HP_REGEN_GITH		0
#define HP_REGEN_GLANDAR	-2
#define HP_REGEN_GNOME		1
#define HP_REGEN_KATRIN		1
#define HP_REGEN_ARAYAN		1
#define HP_REGEN_HARAN		2
#define HP_REGEN_EYAN		-1
#define HP_REGEN_MINOTAUR	3
#define HP_REGEN_ELDARI		-1
#define HP_REGEN_PIXIE		-1
#define HP_REGEN_ZERO		0

/* mana regen race modifier */
#define MN_REGEN_HUMAN		0
#define MN_REGEN_ELF		2
#define MN_REGEN_DWARF		0
#define MN_REGEN_HALFLING	0
#define MN_REGEN_HALF_OGRE	-3
#define MN_REGEN_HALF_ORC	-3
#define MN_REGEN_HALF_TROLL	-2
#define MN_REGEN_HALF_ELF	1
#define MN_REGEN_GITH		3
#define MN_REGEN_GLANDAR	6
#define MN_REGEN_GNOME		0
#define MN_REGEN_KATRIN		0
#define MN_REGEN_ARAYAN		0
#define MN_REGEN_HARAN		-3
#define MN_REGEN_EYAN		1
#define MN_REGEN_MINOTAUR	-5
#define MN_REGEN_ELDARI		1
#define MN_REGEN_PIXIE		1
#define MN_REGEN_ZERO		0

// movement regen race modifier
#define MV_REGEN_HALFLING	1
#define MV_REGEN_HALF_OGRE	1
#define MV_REGEN_HALF_ORC	3
#define MV_REGEN_HALF_TROLL	1
#define MV_REGEN_HALF_ELF	1
#define MV_REGEN_GITH		1
#define MV_REGEN_GLANDAR	-4
#define MV_REGEN_GNOME		0
#define MV_REGEN_KATRIN		4
#define MV_REGEN_ARAYAN		0
#define MV_REGEN_HARAN		3
#define MV_REGEN_EYAN		1
#define MV_REGEN_MINOTAUR	3
#define MV_REGEN_ELDARI		1
#define MV_REGEN_PIXIE		5
#define MV_REGEN_ZERO		0



/* regen base rates */
#define HP_REGEN_BASE		5
#define MN_REGEN_BASE		5
#define MV_REGEN_BASE		15

#if 0
#endif

#include "object.h" // get rid of this sometime!!!!

#endif // include guard

