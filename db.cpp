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
 * 			Database management module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "mud.h"
#include "connection_manager.h"
#include "object.h"

#include "World.h"

#include "paths.const.h"

#include "ScentController.h"

// STL includes
#include <map>

extern	int	_filbuf		 (FILE *) ;

#if defined(KEY)
#undef KEY
#endif

void init_supermob();

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


ExitData::ExitData()
{
	prev = next = rexit = NULL;
	to_room = NULL;

	vnum = rvnum = exit_info = key = 0;

	vdir = distance = 0;
}


/*
 * Globals.
 */

WIZENT *	first_wiz;
WIZENT *	last_wiz;

time_t                  secLastRestoreAllTime = 0;

HELP_DATA *		first_help;
HELP_DATA *		last_help;

/* Multiple greeting screen support -- Warp */
HELP_DATA *             curr_greeting;

SHOP_DATA *		first_shop;
SHOP_DATA *		last_shop;

/* matthew -- stable shops */
STABLE_DATA *   first_stable;
STABLE_DATA *   last_stable;

REPAIR_DATA *		first_repair;
REPAIR_DATA *		last_repair;

TELEPORT_DATA *		first_teleport;
TELEPORT_DATA *		last_teleport;

TRAIN_DATA * first_train;
TRAIN_DATA * last_train;

OBJ_DATA *		extracted_obj_queue;
EXTRACT_CHAR_DATA *	extracted_char_queue;

char			bug_buf		[2*MAX_INPUT_LENGTH];
CHAR_DATA *		first_char;
CHAR_DATA *		last_char;
char *			help_greeting;
char			log_buf		[2*MAX_INPUT_LENGTH];

OBJ_DATA *		first_object;
OBJ_DATA *		last_object;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

int			cur_qobjs;
int			cur_qchars;
int			nummobsloaded;
int			numobjsloaded;
int			physicalobjects;

MAP_INDEX_DATA  *       first_map;	/* maps */

AUCTION_DATA    * 	auction;	/* auctions */

FILE		*	fpLOG;

/* Auction stuff -- Warp */
AUCTION_ROOM *          first_auctionroom = NULL;
AUCTION_ROOM *          last_auctionroom = NULL;

/* weaponry */
sh_int			gsn_pugilism;
sh_int			gsn_long_blades;
sh_int			gsn_short_blades;
sh_int			gsn_flexible_arms;
sh_int			gsn_polearms;
sh_int			gsn_bludgeons;
sh_int			gsn_shieldwork;

/* projectiles */
sh_int          gsn_short_bows;
sh_int          gsn_cross_bows;
sh_int          gsn_long_bows;
sh_int          gsn_slings;
sh_int          gsn_throwing_daggers;
sh_int          gsn_throwing_spears;

/* thief */
sh_int			gsn_detrap;
sh_int			gsn_backstab;
sh_int			gsn_circle;
sh_int			gsn_dodge;
sh_int			gsn_hide;
sh_int			gsn_peek;
sh_int			gsn_pick_lock;
sh_int			gsn_sneak;
sh_int			gsn_steal;
sh_int			gsn_gouge;
sh_int			gsn_poison_weapon;

/* thief & warrior */
sh_int			gsn_disarm;
sh_int			gsn_enhanced_damage;
sh_int			gsn_kick;
sh_int			gsn_parry;
sh_int			gsn_rescue;
sh_int			gsn_second_attack;
sh_int			gsn_third_attack;
sh_int          gsn_fourth_attack;
sh_int          gsn_fifth_attack;
sh_int			gsn_dual_wield;
sh_int			gsn_punch;
sh_int			gsn_bash;
sh_int			gsn_stun;
sh_int          gsn_bashdoor;
sh_int			gsn_grip;
sh_int			gsn_berserk;
sh_int			gsn_hitall;

/* vampire */
sh_int          gsn_feed;

/* other   */

sh_int			gsn_aid;
sh_int			gsn_track;
sh_int			gsn_search;
sh_int			gsn_dig;
sh_int			gsn_mount;
sh_int			gsn_bite;
sh_int			gsn_claw;
sh_int			gsn_sting;
sh_int			gsn_tail;
sh_int			gsn_climb;
sh_int          gsn_scan;
sh_int          gsn_meditate;

/* spells */
sh_int			gsn_aqua_breath;
sh_int          gsn_blindness;
sh_int			gsn_charm_person;
sh_int			gsn_curse;
sh_int			gsn_invis;
sh_int			gsn_mass_invis;
sh_int			gsn_poison;
sh_int			gsn_sleep;
sh_int			gsn_possess;
sh_int			gsn_fireball;
sh_int			gsn_chill_touch;
sh_int			gsn_lightning_bolt;

/* languages */
sh_int			gsn_common;
sh_int			gsn_elven;
sh_int			gsn_dwarven;
sh_int			gsn_pixie;
sh_int			gsn_ogre;
sh_int			gsn_orcish;
sh_int			gsn_trollish;
sh_int			gsn_goblin;
sh_int			gsn_halfling;
sh_int			gsn_katrin;

/* Crafting skills */
sh_int			gsn_scribe;
sh_int			gsn_brew;
sh_int          gsn_cook;
sh_int          gsn_slice;
sh_int          gsn_kindle_fire; /* Ksilyan's */
sh_int			gsn_mine;
sh_int			gsn_smelt;
sh_int			gsn_forge;

/* for searching */
sh_int			gsn_first_spell;
sh_int			gsn_first_skill;
sh_int			gsn_first_weapon;
sh_int			gsn_first_tongue;
sh_int			gsn_top_sn;


/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

AREA_DATA *		first_area;
AREA_DATA *		last_area;
AREA_DATA *		first_build;
AREA_DATA *		last_build;
AREA_DATA *		first_asort;
AREA_DATA *		last_asort;
AREA_DATA *		first_bsort;
AREA_DATA *		last_bsort;

SYSTEM_DATA		sysdata;

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int         top_stable; /* matthew */
int			top_repair;
int			top_vroom;
int         top_train;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];



/*
 * Local booting procedures.
 */
void	init_mm		 ( void ) ;

void	boot_log	 ( const char *str, ... ) ;
void	load_area	 ( FILE *fp ) ;
void    load_author      ( AREA_DATA *tarea, FILE *fp ) ;
void    load_economy     ( AREA_DATA *tarea, FILE *fp ) ;
void    load_installdate  ( AREA_DATA *tarea, FILE *fp ) ;
void	load_resetmsg	 ( AREA_DATA *tarea, FILE *fp ) ; /* Rennard */
void    load_flags       ( AREA_DATA *tarea, FILE *fp ) ;
void	load_helps	 ( AREA_DATA *tarea, FILE *fp ) ;
void	load_mobiles	 ( AREA_DATA *tarea, FILE *fp ) ;
void	load_objects	 ( AREA_DATA *tarea, FILE *fp ) ;
void	load_resets	 ( AREA_DATA *tarea, FILE *fp ) ;
void	load_rooms	 ( AREA_DATA *tarea, FILE *fp ) ;
void	load_shops	 ( AREA_DATA *tarea, FILE *fp ) ;

/* Ksilyan */
void    load_randomdescs  ( AREA_DATA *tarea, FILE *fp ) ;
void	init_arenarank ();
void    LoadMovementMessages( const string & filename ); // act_move.cpp

/* Sap */
void    validate_resets ();

/* matthew */
void    load_stables  ( AREA_DATA *tarea, FILE *fp ) ;
void    load_trainers    ( AREA_DATA *tarea, FILE *fp ) ;

void 	load_repairs	 ( AREA_DATA *tarea, FILE *fp ) ;
void	load_specials	 ( AREA_DATA *tarea, FILE *fp ) ;
void    load_ranges	 ( AREA_DATA *tarea, FILE *fp ) ;
void	load_buildlist	 ( void ) ;
bool	load_systemdata	 ( SYSTEM_DATA *sys ) ;
void    load_banlist     ( void ) ;
void	initialize_economy  ( void ) ;

void	fix_exits	 ( void ) ;
void count_pfile_objs();

/*
 * External booting function
 */
void	load_corpses	 ( void ) ;
void	renumber_put_resets	 ( AREA_DATA *pArea ) ;

extern void load_account_banlist  (void) ;

/*
 * MUDprogram locals
 */

long long 		mprog_name_to_type	 ( char* name ) ;
MPROG_DATA *	mprog_file_read 	( char* f, MPROG_DATA* mprg,
						MOB_INDEX_DATA *pMobIndex );
/* int 		oprog_name_to_type	 ( char* name ) ; */
MPROG_DATA *	oprog_file_read 	 ( char* f, MPROG_DATA* mprg,
						OBJ_INDEX_DATA *pObjIndex );
/* int 		rprog_name_to_type	 ( char* name ) ; */
MPROG_DATA *	rprog_file_read 	 ( char* f, MPROG_DATA* mprg,
						ROOM_INDEX_DATA *pRoomIndex );
void		load_mudprogs            ( AREA_DATA *tarea, FILE* fp ) ;
void		load_objprogs            ( AREA_DATA *tarea, FILE* fp ) ;
void		load_roomprogs           ( AREA_DATA *tarea, FILE* fp ) ;
void   		mprog_read_programs      ( FILE* fp,
						MOB_INDEX_DATA *pMobIndex);
void   		oprog_read_programs      ( FILE* fp,
						OBJ_INDEX_DATA *pObjIndex);
void   		rprog_read_programs      ( FILE* fp,
						ROOM_INDEX_DATA *pRoomIndex);
void            add_auctionroom         (ROOM_INDEX_DATA* room);


void shutdown_mud( const char *reason )
{
    FILE *fp;

    if ( (fp = fopen( SHUTDOWN_FILE, "a" )) != NULL )
    {
	fprintf( fp, "%s\n", reason );
	fclose( fp );
    }
}


/*
 * Big mama top level function.
 */
void boot_db( )
{
    sh_int wear, x;

    show_hash( 32 );
    unlink( BOOTLOG_FILE );
    boot_log( "---------------------[ Boot Log ]--------------------" );

    log_string( "Loading commands" );
    load_commands();

    log_string( "Loading sysdata configuration..." );

    /* default values */
    sysdata.read_all_mail		= LEVEL_IMMORTAL;
    sysdata.read_mail_free 		= LEVEL_IMMORTAL;
    sysdata.write_mail_free 		= LEVEL_IMMORTAL;
    sysdata.take_others_mail		= LEVEL_IMMORTAL;
    sysdata.muse_level			= LEVEL_IMMORTAL;
    sysdata.think_level			= LEVEL_IMMORTAL;
    sysdata.build_level			= LEVEL_IMMORTAL;
    sysdata.log_level			= LEVEL_IMMORTAL;
    sysdata.level_modify_proto		= LEVEL_IMMORTAL;
    sysdata.level_override_private	= LEVEL_IMMORTAL;
    sysdata.level_mset_player		= LEVEL_IMMORTAL;
    sysdata.stun_plr_vs_plr		= 65;
    sysdata.stun_regular		= 15;
    sysdata.dam_plr_vs_plr		= 100;
    sysdata.dam_plr_vs_mob		= 100;
    sysdata.dam_mob_vs_plr		= 100;
    sysdata.dam_mob_vs_mob		= 100;
    sysdata.level_getobjnotake 		= LEVEL_IMMORTAL;
    sysdata.save_frequency		= 20;	/* minutes */
    sysdata.save_flags			= SV_DEATH | SV_PASSCHG | SV_AUTO
    					| SV_PUT | SV_DROP | SV_GIVE
    					| SV_AUCTION | SV_ZAPDROP | SV_IDLE;
    if ( !load_systemdata(&sysdata) )
    {
	log_string( "Not found.  Creating new configuration." );
	sysdata.alltimemax = 0;
    }

    // AIYAN - 26/03/2005 - SCENT SYSTEM
    log_string("Loading scents");
    ScentController::instance()->loadScents();

    log_string("Loading socials");
    load_socials();

    log_string("Loading skill table");
    load_skill_table();
    sort_skill_table();

    gsn_first_spell  = 0;
    gsn_first_skill  = 0;
    gsn_first_weapon = 0;
    gsn_first_tongue = 0;
    gsn_top_sn	     = top_sn;

    for ( x = 0; x < top_sn; x++ )
	if ( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
	    gsn_first_spell = x;
	else
	if ( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
	    gsn_first_skill = x;
	else
	if ( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
	    gsn_first_weapon = x;
	else
	if ( !gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE )
	    gsn_first_tongue = x;

    first_train = last_train = NULL;

    log_string("Loading classes");
    load_classes();

    log_string("Loading herb table");
    load_herb_table();

    log_string("Making wizlist");
    make_wizlist();

	/* Ksilyan */
	log_string("Initializing arena rank");
	init_arenarank();

	log_string("Loading movement types");
	LoadMovementMessages( MOVEMENT_MESSAGE_FILE );

    log_string("Initializing request pipe");
    init_request_pipe();

    fBootDb		= TRUE;

    nummobsloaded	= 0;
    numobjsloaded	= 0;
    physicalobjects	= 0;
    sysdata.maxplayers	= 0;
    first_object	= NULL;
    last_object		= NULL;
    /* Matthew */
    first_account   = NULL;
    last_account    = NULL;

    first_char		= NULL;
    last_char		= NULL;
    first_area		= NULL;
    last_area		= NULL;
    first_build		= NULL;
    last_area		= NULL;
    first_shop		= NULL;
    last_shop		= NULL;

	/* matthew -- stable shops */
	first_stable    = NULL;
	last_stable     = NULL;

    first_repair	= NULL;
    last_repair		= NULL;
    first_teleport	= NULL;
    last_teleport	= NULL;
    first_asort		= NULL;
    last_asort		= NULL;
    extracted_obj_queue	= NULL;
    extracted_char_queue= NULL;
    cur_qobjs		= 0;
    cur_qchars		= 0;
    cur_char		= NULL;
    cur_obj		= 0;
    cur_obj_serial	= 0;
    cur_char_died	= FALSE;
    cur_obj_extracted	= FALSE;
    cur_room		= NULL;
    quitting_char	= NULL;
    loading_char	= NULL;
    saving_char		= NULL;
    CREATE( auction, AUCTION_DATA, 1);
    auction->item 	= NULL;
    for ( wear = 0; wear < MAX_WEAR; wear++ )
	for ( x = 0; x < MAX_LAYERS; x++ )
	    save_equipment[wear][x] = NULL;

    /*
     * Init random number generator.
     */
    log_string("Initializing random number generator");
    init_mm( );

    /*
     * Set time and weather.
     */
    {
	long lhour, lday, lmonth;

	log_string("Setting time and weather");

	lhour		= (secCurrentTime - 650336715)
			/ (PULSE_TICK / PULSE_PER_SECOND);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	     if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
	else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
	else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
	else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
	else                            weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    weather_info.mmhg += number_range( 1, 50 );
	else
	    weather_info.mmhg += number_range( 1, 80 );

	     if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
	else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
	else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
	else                                  weather_info.sky = SKY_CLOUDLESS;

    }


    /*
     * Assign gsn's for skills which need them.
     */
    {
	log_string("Assigning gsn's");

    /* projectiles */
    ASSIGN_GSN( gsn_short_bows,		  "short bows"      );
    ASSIGN_GSN( gsn_cross_bows,       "cross bows"      );
    ASSIGN_GSN( gsn_long_bows,        "long bows"       );
    ASSIGN_GSN( gsn_slings,           "slings"          );
    ASSIGN_GSN( gsn_throwing_daggers, "throwing daggers");
    ASSIGN_GSN( gsn_throwing_spears,  "throwing spears" );

    ASSIGN_GSN( gsn_meditate, 		"meditate");
	ASSIGN_GSN( gsn_pugilism,		"pugilism" );
	ASSIGN_GSN( gsn_long_blades,	"long blades" );
	ASSIGN_GSN( gsn_short_blades,	"short blades" );
	ASSIGN_GSN( gsn_flexible_arms,	"flexible arms" );
	ASSIGN_GSN( gsn_polearms,		"polearms" );
	ASSIGN_GSN( gsn_bludgeons,		"bludgeons" );
	ASSIGN_GSN( gsn_shieldwork,		"shieldwork" );
	ASSIGN_GSN( gsn_detrap,			"detrap" );
	ASSIGN_GSN( gsn_backstab,		"backstab" );
	ASSIGN_GSN( gsn_circle,			"circle" );
	ASSIGN_GSN( gsn_dodge,			"dodge" );
	ASSIGN_GSN( gsn_hide,			"hide" );
	ASSIGN_GSN( gsn_peek,			"peek" );
	ASSIGN_GSN( gsn_pick_lock,		"pick lock" );
	ASSIGN_GSN( gsn_sneak,			"sneak" );
	ASSIGN_GSN( gsn_steal,			"steal" );
	ASSIGN_GSN( gsn_gouge,			"gouge" );
	ASSIGN_GSN( gsn_poison_weapon, 	"poison weapon" );
	ASSIGN_GSN( gsn_disarm,			"disarm" );
	ASSIGN_GSN( gsn_enhanced_damage,"enhanced damage" );
	ASSIGN_GSN( gsn_kick,			"kick" );
	ASSIGN_GSN( gsn_parry,			"parry" );
	ASSIGN_GSN( gsn_rescue,			"rescue" );
	ASSIGN_GSN( gsn_second_attack, 	"second attack" );
	ASSIGN_GSN( gsn_third_attack, 	"third attack" );
	ASSIGN_GSN( gsn_fourth_attack, 	"fourth attack" );
	ASSIGN_GSN( gsn_fifth_attack, 	"fifth attack" );
	ASSIGN_GSN( gsn_dual_wield,		"dual wield" );
	ASSIGN_GSN( gsn_punch,			"punch" );
	ASSIGN_GSN( gsn_bash,			"bash" );
	ASSIGN_GSN( gsn_stun,			"stun" );
	ASSIGN_GSN( gsn_bashdoor,		"doorbash" );
	ASSIGN_GSN( gsn_grip,			"grip" );
	ASSIGN_GSN( gsn_berserk,		"berserk" );
	ASSIGN_GSN( gsn_hitall,			"hitall" );
	ASSIGN_GSN( gsn_feed,			"feed" );
	ASSIGN_GSN( gsn_aid,			"aid" );
	ASSIGN_GSN( gsn_track,			"track" );
	ASSIGN_GSN( gsn_search,			"search" );
	ASSIGN_GSN( gsn_dig,			"dig" );
	ASSIGN_GSN( gsn_mount,			"mount" );
	ASSIGN_GSN( gsn_bite,			"bite" );
	ASSIGN_GSN( gsn_claw,			"claw" );
	ASSIGN_GSN( gsn_sting,			"sting" );
	ASSIGN_GSN( gsn_tail,			"tail" );
	ASSIGN_GSN( gsn_climb,			"climb" );
	ASSIGN_GSN( gsn_scan,			"scan" );
	ASSIGN_GSN( gsn_fireball,		"fireball" );
	ASSIGN_GSN( gsn_chill_touch,	"chill touch" );
	ASSIGN_GSN( gsn_lightning_bolt,	"lightning bolt" );
	ASSIGN_GSN( gsn_aqua_breath,	"aqua breath" );
	ASSIGN_GSN( gsn_blindness,		"blindness" );
	ASSIGN_GSN( gsn_charm_person, 	"charm person" );
	ASSIGN_GSN( gsn_curse,			"curse" );
	ASSIGN_GSN( gsn_invis,			"invis" );
	ASSIGN_GSN( gsn_mass_invis,		"mass invis" );
	ASSIGN_GSN( gsn_poison,			"poison" );
	ASSIGN_GSN( gsn_sleep,			"sleep" );
	ASSIGN_GSN( gsn_possess,		"possess" );
	ASSIGN_GSN( gsn_common,			"common" );
	ASSIGN_GSN( gsn_elven,			"elven" );
	ASSIGN_GSN( gsn_dwarven,		"dwarven" );
	ASSIGN_GSN( gsn_pixie,			"pixie" );
	ASSIGN_GSN( gsn_ogre,			"ogre" );
	ASSIGN_GSN( gsn_orcish,			"orcish" );
	ASSIGN_GSN( gsn_trollish,		"trollese" );
	ASSIGN_GSN( gsn_goblin,			"goblin" );
	ASSIGN_GSN( gsn_halfling,		"halfling" );
	ASSIGN_GSN( gsn_katrin,			"katrin" );
	ASSIGN_GSN( gsn_scribe,			"scribe" );
	ASSIGN_GSN( gsn_brew,			"brew" );
    ASSIGN_GSN( gsn_cook, 			"cook");
    ASSIGN_GSN( gsn_slice, 			"slice");
    ASSIGN_GSN( gsn_kindle_fire, 	"kindle fire" );
    ASSIGN_GSN( gsn_mine,			"mine" );
    ASSIGN_GSN( gsn_smelt,			"smelt" );
    ASSIGN_GSN( gsn_forge,			"forge" );
    }

    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	log_string("Reading in area files...");
	if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
	{
	    perror( AREA_LIST );
	    shutdown_mud( "Unable to open area list" );
	    exit( 1 );
	}

	for ( ; ; ) {
	   strcpy( strArea, fread_word( fpList ) );

	   if ( strArea[0] == '$' )
	     break;

	   load_area_file( last_area, strArea );
	}

	fclose( fpList );
    }

   /*
    *   initialize supermob.
    *    must be done before reset_area!
    *
    */
    init_supermob();



    /*
     * Validate resets.
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    {
	log_string( "Validating resets" );
	validate_resets( );
    	log_string( "Fixing exits" );
    	fix_exits( );
    	fBootDb	= FALSE;
    	log_string( "Loading buildlist" );
    	load_buildlist( );
    	log_string( "Loading boards" );
    	load_boards( );
    	log_string( "Loading clans" );
    	load_clans( );
    	log_string( "Loading councils" );
    	load_deity( );
    	log_string( "Loading deities" );
    	load_councils( );
        log_string( "Loading bans" );
        load_banlist( );
        load_account_banlist( );
        /* scan pfiles, must be done before reset_area! */
        log_string("Scanning the pfiles");
        count_pfile_objs();


/*        if ( !fCopyOver ) {*/
            log_string( "Initializing economy" );
    	    initialize_economy( );

            log_string( "Resetting areas" );
    	    area_update( );

            log_string( "Loading storeitem rooms" );
            load_storeitems_rooms();

            log_string( "Loading buried items");
            load_buried_items();

            log_string( "Loading corpses" );
            load_corpses( );
/*        }*/

        MOBtrigger = TRUE;

        if (fCopyOver)
        {
            log_string("Running copyover_recover.");
            copyover_recover();
        }


    }

	// Ksilyan:
	log_string("Initializing gem list");
	InitGems(10);
	// Replace 10 with whatever the base gem value should be.

    /* init_maps ( ); */

    return;
}



/*
 * Load an 'area' header line.
 */
 void load_area( FILE *fp )
 {
	 AREA_DATA *pArea;
	 unsigned int i, j;

	 pArea = new Area();

	 pArea->first_reset  = NULL;
	 pArea->last_reset	 = NULL;
	 pArea->name		 = fread_string_nohash( fp );
	 pArea->author		   = STRALLOC( "unknown" );
	 pArea->filename	 = str_dup( strArea );
	 pArea->age 	 = 15;
	 pArea->nplayer  = 0;
	 pArea->low_r_vnum	 = 0;
	 pArea->low_o_vnum	 = 0;
	 pArea->low_m_vnum	 = 0;
	 pArea->hi_r_vnum	 = 0;
	 pArea->hi_o_vnum	 = 0;
	 pArea->hi_m_vnum	 = 0;
	 pArea->low_soft_range = 0;
	 pArea->hi_soft_range  = MAX_LEVEL;
	 pArea->low_hard_range = 0;
	 pArea->hi_hard_range  = MAX_LEVEL;
	 pArea->version = 1;

	 for (i = 0; i < sizeof(pArea->random_description_counts)/sizeof(int); i++)
	 {
		 pArea->random_description_counts[i] = 0;
	 }
	 for (i= 0; i <= MAX_RANDOM_DESCRIPTION_TYPE; i++)
	 {
		 for (j = 0; j < MAX_RANDOM_DESCRIPTIONS; j++)
		 {
			 pArea->random_descriptions[i][j] = NULL;
			 pArea->random_night_descriptions[i][j] = NULL;
		 }
	 }

	 LINK( pArea, first_area, last_area, next, prev );
	 top_area++;
	 return;
 }

void load_trainers( AREA_DATA * pArea, FILE *fp )
{
   for ( ; ; )
     {
	TRAIN_DATA *train = NULL;
	MOB_INDEX_DATA *mob = NULL;
	char letter;

	switch ( letter = fread_letter(fp) )
	  {
	   default:
	     bug("Load_trainers: letter '%c' not M or S.\r\n", letter);
	     exit(1);
	     break;

	   case 'M':
	       {
		  int vnum = 0;
		  int _class = 0;
		  int alignment = 0;
		  int min_level = 0;
		  int max_level = 0;
		  int base_cost = 0;
          int gain_cost = 0;

		  vnum = fread_number(fp);
		  _class = fread_number(fp);
		  alignment = fread_number(fp);
		  min_level = fread_number(fp);
		  max_level = fread_number(fp);
		  base_cost = fread_number(fp);

          if ( pArea->version >= 3 ) {
              gain_cost = fread_number(fp);
          }

		  if ( (mob = get_mob_index(vnum)) == NULL )
		    {
		       bug("Load_trainers: 'M': vnum %s.", vnum_to_dotted(vnum));
		       bug("Mob doesn't exist.\r\n", 0);
		       exit(1);
		    }

		  if ( mob->train )
		    {
		       bug("Load_trainers: 'M': vnum %s.", vnum_to_dotted(vnum));
		       bug("Mob train data already exists.\r\n", 0);
		       exit(1);
		    }

		  CREATE(train, TRAIN_DATA, 1);
		  train->vnum = vnum;
		  train->max_level = max_level;
		  train->min_level = min_level;
		  train->_class = _class;
		  train->alignment = alignment;
		  train->base_cost = base_cost;
          train->gain_cost = gain_cost;
		  mob->train = train;
		  LINK(train, first_train, last_train, next, prev);
		  break;
	       }
	   case 'S':
	     return;
	  }
	fread_to_eol(fp);
     }
   return;
}

void load_trainerskills( AREA_DATA * pArea, FILE *fp )
{
   for ( ; ; )
     {
	TRAIN_LIST_DATA *tlist = NULL;
	MOB_INDEX_DATA *mob = NULL;
	char letter;

	switch ( letter = fread_letter(fp) )
	  {
	   default:
	     bug("Load_trainerskills: letter '%c' not M or S.\r\n", letter);
	     exit(1);
	     break;

	   case 'M':
	       {
		  int vnum = 0;
		  int sn = 0;
		  int max = 0;
		  int cost = 0;
		  char *spell;

		  vnum = fread_number(fp);
		  max = fread_number(fp);
		  cost = fread_number(fp);
		  spell = fread_word(fp);

		  sn = skill_lookup(spell);

		  if ( (mob = get_mob_index(vnum)) == NULL )
		    {
		       bug("Load_trainerskills: 'M': vnum %s.", vnum_to_dotted(vnum));
		       bug("Mob doesn't exist.\r\n", 0);
		       exit(1);
		    }

		  if ( !mob->train )
		    {
		       bug("Load_trainerskills: 'M': vnum %s.", vnum_to_dotted(vnum));
		       bug("Mob is not a trainer.\r\n", 0);
		       exit(1);
		    }

		  CREATE(tlist, TRAIN_LIST_DATA, 1);
		  tlist->sn = sn;
		  tlist->max = max;
		  tlist->cost = cost;
		  LINK(tlist, mob->train->first_in_list, mob->train->last_in_list, next, prev);
		  break;
	       }
	   case 'S':
	     return;
	  }
	fread_to_eol(fp);
     }
   return;
}




void load_version( AREA_DATA *tarea, FILE* fp)
{

      if ( !tarea )
     {
	        bug ( "Load version: no #AREA seenyet." );

	        if ( fBootDb )
	  {
	                  shutdown_mud( " No #AREA");
	                  exit(1);
	  }
	        else
	            return;
     }

      tarea->version = fread_number(fp);


      return;
}

/*
 * Load an author section. Scryn 2/1/96
 */
void load_author( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_author: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->author )
      STRFREE( tarea->author );
    tarea->author   = fread_string( fp );
    return;
}

/*
 * Load an economy section. Thoric
 */
void load_economy( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_economy: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    tarea->high_economy	= fread_number( fp );
    tarea->low_economy	= fread_number( fp );
    return;
}

/*
 * Load the isntall date, Ksilyan
 */
void load_installdate( AREA_DATA * tarea, FILE * fp)
{
	if ( !tarea)
	{
		bug( "Load_installdate: no #AREA seen yet." );
		if ( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit(1);
		}
		else
			return;
	}

	tarea->secInstallDate = fread_number(fp);
	return;
}

/* Reset Message Load, Rennard */
void load_resetmsg( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_resetmsg: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->resetmsg )
	DISPOSE( tarea->resetmsg );
    tarea->resetmsg = fread_string_nohash( fp );
    return;
}

/*
 * Load area flags. Narn, Mar/96
 */
void load_flags( AREA_DATA *tarea, FILE *fp )
{
    char *ln;
    int x1, x2;

    if ( !tarea )
    {
	bug( "Load_flags: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }
    ln = fread_line( fp );
    x1=x2=0;
    sscanf( ln, "%d %d",
	&x1, &x2 );
    tarea->flags = x1;
    tarea->reset_frequency = x2;
    if ( x2 )
	tarea->age = x2;
    return;
}

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help( HELP_DATA *pHelp )
{
    HELP_DATA *tHelp;
    int match;

    for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
	if ( pHelp->level == tHelp->level
	&&   strcmp(pHelp->keyword, tHelp->keyword) == 0 )
	{
	    bug( "add_help: duplicate: %s.  Deleting.", pHelp->keyword );
	    STRFREE( pHelp->text );
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    return;
	}
	else
	if ( (match=strcmp(pHelp->keyword[0]=='\'' ? pHelp->keyword+1 : pHelp->keyword,
			   tHelp->keyword[0]=='\'' ? tHelp->keyword+1 : tHelp->keyword)) < 0
	||   (match == 0 && pHelp->level > tHelp->level) )
	{
	    if ( !tHelp->prev )
		first_help	  = pHelp;
	    else
		tHelp->prev->next = pHelp;
	    pHelp->prev		  = tHelp->prev;
	    pHelp->next		  = tHelp;
	    tHelp->prev		  = pHelp;
	    break;
	}

    if ( !tHelp )
	LINK( pHelp, first_help, last_help, next, prev );

    top_help++;
}

/*
 * Load a help section.
 */
void load_helps( AREA_DATA *tarea, FILE *fp )
{
   HELP_DATA *pHelp, *pFirstGreet = NULL, *ptempGreet = NULL;

   for (;;) {
	   CREATE( pHelp, HELP_DATA, 1 );
	   pHelp->level	= fread_number( fp );
	   pHelp->keyword	= fread_string( fp );

	   if ( pHelp->keyword[0] == '$' )
	   {
		   STRFREE( pHelp->keyword );
		   DISPOSE( pHelp );
		   break;
	   }

	   pHelp->text	= fread_string( fp );

	   if ( pHelp->keyword[0] == '\0') {
		   STRFREE( pHelp->text );
		   STRFREE( pHelp->keyword );
		   DISPOSE( pHelp );
		   continue;
	   }

	   if (!strncmp(pHelp->keyword, "GREETING", 8)) {
		   CREATE(ptempGreet, HELP_DATA, 1);
		   ptempGreet->text = STRALLOC(pHelp->text);

		   if(pFirstGreet == NULL) {
			   pFirstGreet = ptempGreet;
		   }
		   else {
			   curr_greeting->next = ptempGreet;
		   }

		   curr_greeting = ptempGreet;
	   }

	   add_help( pHelp );
   }

   /* Create a loop of the greets -- Warp */
   if(curr_greeting) {
      curr_greeting->next = pFirstGreet;
   }

   return;
}

void recount_greetings() {
    HELP_DATA* pTemp;
    HELP_DATA* pFirst;
    HELP_DATA* pCurr;

    /* free the current list */
    pFirst = curr_greeting;
    curr_greeting = curr_greeting->next;

    while ( curr_greeting != pFirst ) {
        pTemp = curr_greeting->next;
        STRFREE(curr_greeting->text);
        DISPOSE(curr_greeting);
        curr_greeting = pTemp;
    }

    STRFREE(curr_greeting->text);
    DISPOSE(curr_greeting);

    /* create a new one */

    pFirst = NULL;

    for ( pTemp = first_help; pTemp; pTemp = pTemp->next ) {
        if ( !str_prefix("greeting", pTemp->keyword) ) {
            CREATE(pCurr, HELP_DATA, 1);
            pCurr->text = STRALLOC(pTemp->text);

            if ( !pFirst ) {
                pFirst = curr_greeting = pCurr;
            } else {
                curr_greeting->next = pCurr;
                curr_greeting = pCurr;
            }
        }
    }

    curr_greeting->next = pFirst;
}

/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA *ch )
{
    LINK( ch, first_char, last_char, next, prev );
}


/*
 * Load a mob section.
 */
void load_mobiles( AREA_DATA *tarea, FILE *fp )
{
	MOB_INDEX_DATA *pMobIndex;
	char *ln;
	int x1, x2, x3, x4, x5, x6, x7, x8;

	if ( !tarea )
	{
		bug( "Load_mobiles: no #AREA seen yet." );
		if ( fBootDb )
		{
			shutdown_mud( "No #AREA" );
			exit( 1 );
		}
		else
			return;
	}

	for ( ; ; )
	{
		char buf[MAX_STRING_LENGTH];
		int vnum;
		char letter;
		int iHash;
		bool oldmob;
		bool tmpBootDb;

		letter				= fread_letter( fp );
		if ( letter != '#' )
		{
			bug( "Load_mobiles: # not found." );
			if ( fBootDb )
			{
				shutdown_mud( "# not found" );
				exit( 1 );
			}
			else
				return;
		}

		vnum				= fread_number( fp );
		if ( vnum == 0 )
			break;

		tmpBootDb = fBootDb;
		fBootDb = FALSE;
		if ( get_mob_index( vnum ) )
		{
			if ( tmpBootDb )
			{
				bug( "Load_mobiles: vnum %s duplicated.", vnum_to_dotted(vnum) );
				shutdown_mud( "duplicate vnum" );
				exit( 1 );
			}
			else
			{
				pMobIndex = get_mob_index( vnum );
				sprintf( buf, "Cleaning mobile: %s", vnum_to_dotted(vnum) );
				log_string_plus( buf, LOG_BUILD, sysdata.log_level );
				clean_mob( pMobIndex );
				oldmob = TRUE;
			}
		}
		else
		{
			oldmob = FALSE;
			pMobIndex = new mob_index_data();
		}
		fBootDb = tmpBootDb;

		pMobIndex->vnum			= vnum;
		if ( fBootDb )
		{
			if ( !tarea->low_m_vnum )
				tarea->low_m_vnum	= vnum;
			if ( vnum > tarea->hi_m_vnum )
				tarea->hi_m_vnum	= vnum;
		}
		pMobIndex->playerName_  = fread_string_noheap( fp );
		pMobIndex->shortDesc_   = fread_string_noheap( fp );
		pMobIndex->longDesc_    = fread_string_noheap( fp );
		pMobIndex->description_ = fread_string_noheap( fp );

		if ( tarea->version >= 2 )
		{
			pMobIndex->exitDesc_   = fread_string_noheap( fp );
			pMobIndex->enterDesc_  = fread_string_noheap( fp );
		}

		pMobIndex->longDesc_      = capitalize_first( pMobIndex->longDesc_ );
		pMobIndex->description_   = capitalize_first( pMobIndex->description_ );

		pMobIndex->act			= fread_number( fp ) | ACT_IS_NPC;
		pMobIndex->affected_by		= fread_number( fp );
		pMobIndex->pShop		= NULL;
		pMobIndex->rShop		= NULL;
		pMobIndex->alignment		= fread_number( fp );
		letter				= fread_letter( fp );
		pMobIndex->level		= fread_number( fp );

		pMobIndex->mobthac0		= fread_number( fp );
		pMobIndex->ac			= fread_number( fp );
		pMobIndex->hitnodice		= fread_number( fp );
		/* 'd'		*/		  fread_letter( fp );
		pMobIndex->hitsizedice		= fread_number( fp );
		/* '+'		*/		  fread_letter( fp );
		pMobIndex->hitplus		= fread_number( fp );
		pMobIndex->damnodice		= fread_number( fp );
		/* 'd'		*/		  fread_letter( fp );
		pMobIndex->damsizedice		= fread_number( fp );
		/* '+'		*/		  fread_letter( fp );
		pMobIndex->damplus		= fread_number( fp );
		pMobIndex->gold			= fread_number( fp );
		pMobIndex->exp			= fread_number( fp );
		pMobIndex->position		= fread_number( fp );
		pMobIndex->defposition		= fread_number( fp );

		/*
		 * Back to meaningful values.
		 */
		pMobIndex->sex			= fread_number( fp );

		if ( letter != 'S' && letter != 'C' )
		{
			bug( "Load_mobiles: vnum %s: letter '%c' not S or C.", vnum_to_dotted(vnum),
					letter );
			shutdown_mud( "bad mob data" );
			exit( 1 );
		}
		if ( letter == 'C' ) /* Realms complex mob 	-Thoric */
		{
			pMobIndex->perm_str			= fread_number( fp );
			pMobIndex->perm_int			= fread_number( fp );
			pMobIndex->perm_wis			= fread_number( fp );
			pMobIndex->perm_dex			= fread_number( fp );
			pMobIndex->perm_con			= fread_number( fp );
			pMobIndex->perm_cha			= fread_number( fp );
			pMobIndex->perm_lck			= fread_number( fp );
			pMobIndex->saving_poison_death	= fread_number( fp );
			pMobIndex->saving_wand		= fread_number( fp );
			pMobIndex->saving_para_petri	= fread_number( fp );
			pMobIndex->saving_breath		= fread_number( fp );
			pMobIndex->saving_spell_staff	= fread_number( fp );
			ln = fread_line( fp );
			x1=x2=x3=x4=x5=x6=x7=0;
			sscanf( ln, "%d %d %d %d %d %d %d",
					&x1, &x2, &x3, &x4, &x5, &x6, &x7 );
			pMobIndex->race		= x1;
			pMobIndex->Class		= x2;
			pMobIndex->height		= x3;
			pMobIndex->weight		= x4;
			pMobIndex->speaks		= x5;
			pMobIndex->speaking		= x6;
			pMobIndex->numattacks	= x7;
			if ( !pMobIndex->speaks )
				pMobIndex->speaks = race_table[pMobIndex->race].language | LANG_COMMON;
			if ( !pMobIndex->speaking )
				pMobIndex->speaking = race_table[pMobIndex->race].language;

			ln = fread_line( fp );
			x1=x2=x3=x4=x5=x6=x7=x8=0;
			sscanf( ln, "%d %d %d %d %d %d %d %d",
					&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
			pMobIndex->hitroll		= x1;
			pMobIndex->damroll		= x2;
			pMobIndex->xflags		= x3;
			pMobIndex->resistant	= x4;
			pMobIndex->immune		= x5;
			pMobIndex->susceptible	= x6;
			pMobIndex->attacks		= x7;
			pMobIndex->defenses		= x8;

			if ( tarea->version >= 9 )
				pMobIndex->HomeVnum = fread_number( fp );
		}
		else
		{
			pMobIndex->perm_str		= 13;
			pMobIndex->perm_dex		= 13;
			pMobIndex->perm_int		= 13;
			pMobIndex->perm_wis		= 13;
			pMobIndex->perm_cha		= 13;
			pMobIndex->perm_con		= 13;
			pMobIndex->perm_lck		= 13;
			pMobIndex->race		= 0;
			pMobIndex->Class		= 3;
			pMobIndex->xflags		= 0;
			pMobIndex->resistant	= 0;
			pMobIndex->immune		= 0;
			pMobIndex->susceptible	= 0;
			pMobIndex->numattacks	= 0;
			pMobIndex->attacks		= 0;
			pMobIndex->defenses		= 0;
		}

		letter = fread_letter( fp );
		if ( letter == '>' )
		{
			ungetc( letter, fp );
			mprog_read_programs( fp, pMobIndex );
		}
		else ungetc( letter,fp );

		// Skrypt - this is (c) Ksilyan and belongs to him
		letter = fread_letter( fp );
		if ( letter == '}' )
		{
			pMobIndex->skryptRead(fp);
		}
		else ungetc( letter,fp );

		if ( !oldmob )
		{
			iHash			= vnum % MAX_KEY_HASH;
			pMobIndex->next		= mob_index_hash[iHash];
			mob_index_hash[iHash]	= pMobIndex;
			top_mob_index++;
		}
	}

	return;
}



/*
 * Load an obj section.
 */
void load_objects( AREA_DATA *tarea, FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    char letter;
    char *ln;
    int x1, x2, x3, x4, x5, x6;
	int oldtype;

    if ( !tarea )
    {
	bug( "Load_objects: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	int vnum;
	int iHash;
	bool tmpBootDb;
	bool oldobj;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_obj_index( vnum ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_objects: vnum %s duplicated.", vnum_to_dotted(vnum) );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pObjIndex = get_obj_index( vnum );
		sprintf( buf, "Cleaning object: %s", vnum_to_dotted(vnum) );
		log_string_plus( buf, LOG_BUILD, sysdata.log_level );
		clean_obj( pObjIndex );
		oldobj = TRUE;
	    }
	}
	else
	{
	  oldobj = FALSE;
	  pObjIndex = new obj_index_data();
	}
	pObjIndex->max_condition = 0;
	fBootDb = tmpBootDb;

	pObjIndex->vnum			= vnum;
	if ( fBootDb )
	{
	  if ( !tarea->low_o_vnum )
	    tarea->low_o_vnum		= vnum;
	  if ( vnum > tarea->hi_o_vnum )
	    tarea->hi_o_vnum		= vnum;
	}
	pObjIndex->name_         = fread_string_noheap( fp );
	pObjIndex->shortDesc_    = fread_string_noheap( fp );
	pObjIndex->longDesc_     = fread_string_noheap( fp );
	pObjIndex->actionDesc_   = fread_string_noheap( fp );

        /* Commented out by Narn, Apr/96 to allow item short descs like
           Bonecrusher and Oblivion */
	/*pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);*/

	// this is a cheap hack
	pObjIndex->longDesc_ = capitalize_first( pObjIndex->longDesc_ );

	ln = fread_line( fp );
	if (tarea->version <= 4)
	{
		x1=x2=x3=x4=0;
		sscanf( ln, "%d %d %d %d",
			&x1, &x2, &x3, &x4 );
		pObjIndex->item_type		= x1;
		pObjIndex->extra_flags		= x2;
		pObjIndex->wear_flags		= x3;
		pObjIndex->layers		= x4;
	}
	else if (tarea->version == 5)
	{
		x1=x2=x3=x4=x5=0;
		sscanf( ln, "%d %d %d %d %d",
			&x1, &x2, &x3, &x4, &x5 );
		pObjIndex->item_type		= x1;
		pObjIndex->extra_flags		= x2;
		pObjIndex->wear_flags		= x3;
		pObjIndex->layers		= x4;
		pObjIndex->max_condition = x5;
	}
	else if (tarea->version >= 6)
	{
		x1=x2=x3=x4=x5=x6=0;
		sscanf( ln, "%d %d %d %d %d %d",
			&x1, &x2, &x3, &x4, &x5, &x6 );
		pObjIndex->item_type		= x1;
		pObjIndex->extra_flags		= x2;
		pObjIndex->wear_flags		= x3;
		pObjIndex->layers		= x4;
		pObjIndex->max_condition = x5;
		pObjIndex->extra_flags_2	= x6;
	}

	oldtype = 0;

	if (tarea->version <= 5)
	{
		oldtype = pObjIndex->item_type;
		pObjIndex->item_type = itemtype_old_to_new(pObjIndex->item_type);
	}


	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6 );
	pObjIndex->value[0]		= x1;
	pObjIndex->value[1]		= x2;
	pObjIndex->value[2]		= x3;
	pObjIndex->value[3]		= x4;

	pObjIndex->value[4]		= x5;
	pObjIndex->value[5]		= x6;

	if (pObjIndex->item_type == ITEM_ARMOR)
	{
		if (tarea->version <= 4)
			pObjIndex->max_condition = x2;
	}
	if (pObjIndex->item_type == ITEM_CONTAINER)
	{
		if (tarea->version <= 4)
			pObjIndex->max_condition = x4;
	}

	if (pObjIndex->max_condition <= 0)
		pObjIndex->max_condition = 8;

	if (pObjIndex->item_type == ITEM_WEAPON)
	{
		if (tarea->version <= 4)
		{
			switch(x4)
			{
				default:
					sprintf(buf, "Unknown weapon type!! (vnum %d)\r\n", pObjIndex->vnum);
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_NONE;
					break;
				case DAM_HIT: case DAM_SUCTION: case DAM_BITE: case DAM_BLAST:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_PUGILISM;
					break;
				case DAM_SLASH: case DAM_SLICE:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_LONGBLADE;
					break;
				case DAM_PIERCE: case DAM_STAB:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_SHORTBLADE;
					break;
				case DAM_WHIP:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_FLEXIBLE;
					break;
				case DAM_CLAW:
					sprintf(buf, "Weapon %d has type talonous. Making polearm...\r\n", pObjIndex->vnum);
					bug(buf);
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_POLEARM;
					break;
				case DAM_POUND: case DAM_CRUSH:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_BLUDGEON;
					break;
				case DAM_LARROW:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_LONGBOW;
					pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = AMMO_LONGARROW;
					break;
				case DAM_SARROW:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_SHORTBOW;
					pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = AMMO_SHORTARROW;
					break;
				case DAM_BOLT:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_CROSSBOW;
					pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = AMMO_BOLT;
					break;
				case DAM_DART:
					sprintf(buf, "Weapon %d has type dart gun. Making short bow...\r\n", pObjIndex->vnum);
					bug(buf);
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_SHORTBOW;
					pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = AMMO_SHORTARROW;
					break;
				case DAM_STONE:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_SLING;
					pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = AMMO_STONE;
					break;
				case DAM_TDAGGER:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_THROWINGKNIFE;
					break;
				case DAM_TSPEAR:
					pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = WEAPON_THROWINGSPEAR;
					break;
			}
		}
		else if (tarea->version >= 5)
		{
			pObjIndex->value[3]		= x4;
		}

		if (pObjIndex->value[OBJECT_WEAPON_MAXDAMAGE] <= pObjIndex->value[OBJECT_WEAPON_MINDAMAGE])
		{
			/* Ksilyan
			 * this fix is to help get rid of the old system of XdY for damage, instead
			 * of minimum to maximum, which is the new system.
			 */
			pObjIndex->value[OBJECT_WEAPON_MAXDAMAGE] *= pObjIndex->value[OBJECT_WEAPON_MINDAMAGE];
		}

		if (pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] < MIN_DAMAGE_TYPE)
		{
			/* small fix to get the damage types set up properly - Ksilyan */
			if (!IS_RANGED_WEAPON(pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE]))
			{
				pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] =
						weapontype_to_damagetype(pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE]);
			}
		}
		if (IS_RANGED_WEAPON(pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE])
				&& pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] == AMMO_NONE)
		{
			pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = weapontype_to_ammotype(pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE]);
		}
		if (pObjIndex->value[OBJECT_WEAPON_DAMAGEMESSAGE] == DAMAGE_MSG_NONE)
		{
			/* If there is no damage message, then we need to default to one. */
			pObjIndex->value[OBJECT_WEAPON_DAMAGEMESSAGE] = weapontype_to_damagemsg(pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE]);
		}
	}
	if (oldtype == OLD_ITEM_MISSILE_WEAPON)
	{
		int newammo;
		switch(pObjIndex->value[3])
		{
			default:
				sprintf(buf, "Unknown ammo type for weapon %d : %d!", pObjIndex->vnum, pObjIndex->value[3]);
				bug(buf);
				break;
			case 13:
				newammo = AMMO_LONGARROW;
				break;
			case 14:
				newammo = AMMO_SHORTARROW;
				break;
			case 15:
				newammo = AMMO_BOLT;
				break;
			case 16:
				newammo = AMMO_SHORTARROW;
				break;
			case 17:
				newammo = AMMO_STONE;
				break;
		}
		pObjIndex->item_type = ITEM_WEAPON;
		pObjIndex->value[OBJECT_WEAPON_WEAPONTYPE] = ammotype_to_weapontype(newammo);
		pObjIndex->value[OBJECT_WEAPON_DAMAGETYPE] = newammo;
		sprintf(buf, "Updating weapon %d to new format...", pObjIndex->vnum);
		log_string(buf);
	}

	pObjIndex->weight		= fread_number( fp );
	pObjIndex->weight = UMAX( 1, pObjIndex->weight );
	pObjIndex->cost			= fread_number( fp );
	pObjIndex->rent		  	= fread_number( fp ); /* unused */

    if ( tarea->version >= 4 ) {
        pObjIndex->rare     = fread_number( fp );
    } else {
        pObjIndex->rare     = 0;
    }

	for ( ; ; )
	{
		letter = fread_letter( fp );

		if ( letter == 'A' )
		{
			AFFECT_DATA *paf;

			CREATE( paf, AFFECT_DATA, 1 );
			paf->type		= -1;
			paf->duration		= -1;
			paf->location		= fread_number( fp );
			if ( paf->location == APPLY_WEAPONSPELL
					||   paf->location == APPLY_WEARSPELL
					||   paf->location == APPLY_REMOVESPELL
					||   paf->location == APPLY_STRIPSN )
				paf->modifier		= slot_lookup( fread_number(fp) );
			else
				paf->modifier		= fread_number( fp );
			paf->bitvector		= 0;
			LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
					next, prev );
			top_affect++;
		}

		else if ( letter == 'E' )
		{
			ExtraDescData *ed;

			ed = new ExtraDescData;
			ed->keyword_      = fread_string_noheap( fp );
			ed->description_  = fread_string_noheap( fp );
			LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
					next, prev );
			top_ed++;
		}

		else if ( letter == '>' )
		{
			ungetc( letter, fp );
			oprog_read_programs( fp, pObjIndex );
		}
		// Skrypt - this is (c) Ksilyan and belongs to him
		else if ( letter == '}' )
		{
			pObjIndex->skryptRead(fp);
		}
		else
		{
			ungetc( letter, fp );
			break;
		}
	}

	/*
	 * Translate spell "slot numbers" to internal "skill numbers."
	 */
	switch ( pObjIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	    pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
	    pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;

	case ITEM_STAFF:
	case ITEM_WAND:
	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	    break;
	case ITEM_SALVE:
	    pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
	    pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
	    break;
	}

	if ( !oldobj )
	{
	  iHash			= vnum % MAX_KEY_HASH;
	  pObjIndex->next	= obj_index_hash[iHash];
	  obj_index_hash[iHash]	= pObjIndex;
	  top_obj_index++;
	}
    }

    return;
}



/*
 * Load a reset section.
 */
void load_resets( AREA_DATA *tarea, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];

    if ( !tarea )
    {
	bug( "Load_resets: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->first_reset )
    {
	if ( fBootDb )
	{
	  bug( "load_resets: WARNING: resets already exist for this area." );
	}
	else
	{
	 /*
	  * Clean out the old resets
	  */
	  sprintf( buf, "Cleaning resets: %s", tarea->name );
	  log_string_plus( buf, LOG_BUILD, sysdata.log_level );
	  clean_resets( tarea );
	}
    }

    for ( ; ; )
    {
	char letter;
	int extra, arg1, arg2, arg3;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	extra	= fread_number( fp );
	arg1	= fread_number( fp );
	arg2	= fread_number( fp );
	arg3	= (letter == 'G' || letter == 'R')
		  ? 0 : fread_number( fp );
		  fread_to_eol( fp );

	/* finally, add the reset */
	add_reset( tarea, letter, extra, arg1, arg2, arg3 );
    }

    return;
}



/* Validates every loaded reset.
 * Used to be in load_resets, but moved out to avoid fake bug
 * logs caused by unloaded objects from other areas.
 */
void validate_resets()
{
    AREA_DATA *tarea;
    RESET_DATA *reset;
    ROOM_INDEX_DATA *pRoomIndex;
    ExitData *pexit;
    bool not01 = FALSE;
    char letter;
    int extra, arg1, arg2, arg3;
    int count;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
	count = 0;
        for ( reset = tarea->first_reset; reset; reset = reset->next )
        {
            letter = reset->command;
            extra = reset->extra;
            arg1 = reset->arg1;
            arg2 = reset->arg2;
            arg3 = reset->arg3;

            switch ( letter )
            {
            default:
                bug( "Load_resets: bad command '%c'.", letter );
                if ( fBootDb )
		    boot_log( "Load_resets: %s (%d) bad command '%c'.", tarea->filename, count, letter );
                return;

            case 'M':
                if ( get_mob_index( arg1 ) == NULL && fBootDb )
                    boot_log( "Load_resets: %s (%d) 'M': mobile %d doesn't exist.",
                              tarea->filename, count, arg1 );
                if ( get_room_index( arg3 ) == NULL && fBootDb )
                    boot_log( "Load_resets: %s (%d) 'M': room %d doesn't exist.",
                              tarea->filename, count, arg3 );
                break;

            case 'O':
                if ( get_obj_index(arg1) == NULL && fBootDb )
                    boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
                              tarea->filename, count, letter, arg1 );
                if ( get_room_index(arg3) == NULL && fBootDb )
                    boot_log( "Load_resets: %s (%d) '%c': room %d doesn't exist.",
                              tarea->filename, count, letter, arg3 );
                break;

            case 'P':
                if ( get_obj_index(arg1) == NULL && fBootDb )
                    boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
                              tarea->filename, count, letter, arg1 );
                if ( arg3 > 0 )
                {
                    if ( get_obj_index(arg3) == NULL && fBootDb )
                        boot_log( "Load_resets: %s (%d) 'P': destination object %d doesn't exist.",
				  tarea->filename, count, arg3 );
                    else if ( extra > 1 )
                        not01 = TRUE;
		}
                break;

	    case 'G':
	    case 'E':
	        if ( get_obj_index(arg1) == NULL && fBootDb )
		    boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
			      tarea->filename, count, letter, arg1 );
                break;

            case 'T':
                break;

            case 'H':
                if ( arg1 > 0 )
                    if ( get_obj_index(arg1) == NULL && fBootDb )
                        boot_log( "Load_resets: %s (%d) 'H': object %d doesn't exist.",
				  tarea->filename, count, arg1 );
                break;

            case 'D':
                pRoomIndex = get_room_index( arg1 );
                if ( !pRoomIndex )
                {
                    bug( "Load_resets: 'D': room %d doesn't exist.", arg1 );
                    bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                         arg3 );
                    if ( fBootDb )
                        boot_log( "Load_resets: %s (%d) 'D': room %d doesn't exist.",
                                  tarea->filename, count, arg1 );
                    break;
                }

                if ( arg2 < 0
                     ||   arg2 > MAX_DIR+1
                     || ( pexit = get_exit(pRoomIndex, arg2)) == NULL
                     || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
                {
                    bug( "Load_resets: 'D': exit %d not door.", arg2 );
                    bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                         arg3 );
                    if ( fBootDb )
                        boot_log( "Load_resets: %s (%d) 'D': exit %d not door.",
                                  tarea->filename, count, arg2 );
                }

                if ( arg3 < 0 || arg3 > 2 )
                {
                    bug( "Load_resets: 'D': bad 'locks': %d.", arg3 );
                    if ( fBootDb )
                        boot_log( "Load_resets: %s (%d) 'D': bad 'locks': %d.",
                                  tarea->filename, count, arg3 );
                }
                break;

            case 'R':
                pRoomIndex = get_room_index( arg1 );
                if ( !pRoomIndex && fBootDb )
                    boot_log( "Load_resets: %s (%d) 'R': room %d doesn't exist.",
                              tarea->filename, count, arg1 );

                if ( arg2 < 0 || arg2 > 6 )
                {
                    bug( "Load_resets: 'R': bad exit %d.", arg2 );
                    if ( fBootDb )
                        boot_log( "Load_resets: %s (%d) 'R': bad exit %d.",
                                  tarea->filename, count, arg2 );
                    break;
		}

                break;
            }

	    ++count;
        }

	if ( !not01 )
	    renumber_put_resets(tarea);
    }

}



/*
 * Load a room section.
 */
void load_rooms(AREA_DATA *tarea, FILE *fp)
{
   ROOM_INDEX_DATA *pRoomIndex;
   char buf[MAX_STRING_LENGTH];
   char *ln;

   if (!tarea) {
      bug("Load_rooms: no #AREA seen yet.");
      shutdown_mud("No #AREA");
      exit(1);
   }

   for (; ;) {
	   int vnum;
	   char letter;
	   int door;
	   int iHash;
	   bool tmpBootDb;
	   bool oldroom;
	   int x1, x2, x3, x4, x5, x6;

	   letter				= fread_letter(fp);
	   if (letter != '#') {
		   bug("Load_rooms: # not found.");
		   if (fBootDb) {
			   shutdown_mud("# not found");
			   exit(1);
		   }
		   else
			   return;
	   }

	   vnum				= fread_number(fp);
	   if (vnum == 0)
		   break;

	   tmpBootDb = fBootDb;
	   fBootDb = FALSE;
	   if (get_room_index(vnum) != NULL) {
		   if (tmpBootDb) {
			   bug("Load_rooms: vnum %d duplicated.", vnum);
			   shutdown_mud("duplicate vnum");
			   exit(1);
		   }
		   else {
			   pRoomIndex = get_room_index(vnum);
			   sprintf(buf, "Cleaning room: %d", vnum);
			   log_string_plus(buf, LOG_BUILD, sysdata.log_level);
			   clean_room(pRoomIndex);
			   oldroom = TRUE;
		   }
	   }
	   else {
		   oldroom = FALSE;
		   pRoomIndex = new Room();
	   }

	   fBootDb = tmpBootDb;
	   pRoomIndex->area		= tarea;
	   pRoomIndex->vnum		= vnum;
	   pRoomIndex->first_extradesc	= NULL;
	   pRoomIndex->last_extradesc	= NULL;

	   if (fBootDb) {
		   if (!tarea->low_r_vnum)
			   tarea->low_r_vnum		= vnum;
		   if (vnum > tarea->hi_r_vnum)
			   tarea->hi_r_vnum		= vnum;
	   }
	   pRoomIndex->name_        = fread_string_noheap(fp);
	   pRoomIndex->description_ = fread_string_noheap(fp);

	   /* Random room stuff */
	   if (tarea->version >= 8)
	   {
		   ln = fread_line(fp);
		   x1 = x2 = 0;
		   sscanf(ln, "%d %d", &x1, &x2);
		   pRoomIndex->random_room_type = x1;
		   pRoomIndex->random_description = x2;
	   }

	   /* Area number			  fread_number(fp); */
	   ln = fread_line(fp);
	   x1=x2=x3=x4=x5=x6=0;
	   sscanf(ln, "%d %d %d %d %d %d",
			   &x1, &x2, &x3, &x4, &x5, &x6);

	   pRoomIndex->room_flags		= x2;

	   if(IS_SET(pRoomIndex->room_flags, ROOM_AUCTIONROOM))
		   add_auctionroom(pRoomIndex);

	   pRoomIndex->sector_type		= x3;
	   pRoomIndex->tele_delay		= x4;
	   pRoomIndex->tele_vnum		= x5;
	   pRoomIndex->tunnel		= x6;

	   if (pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX) {
		   bug("Fread_rooms: vnum %d has bad sector_type %d.", vnum ,
				   pRoomIndex->sector_type);
		   pRoomIndex->sector_type = 1;
	   }
	   pRoomIndex->light		= 0;
	   pRoomIndex->first_exit		= NULL;
	   pRoomIndex->last_exit		= NULL;

	   for (; ;) {
		   letter = fread_letter(fp);

		   if (letter == 'S')
			   break;

		   if (letter == 'D') {
			   ExitData *pexit;
			   int locks;

			   door = fread_number(fp);
			   if (door < 0 || door > 10) {
				   bug("Fread_rooms: vnum %d has bad door number %d.", vnum, door);
				   if (fBootDb)
					   exit(1);
			   }
			   else {
				   pexit = make_exit(pRoomIndex, NULL, door);
				   pexit->description_ = fread_string_noheap(fp);
				   pexit->keyword_     = fread_string_noheap(fp);
				   pexit->exit_info	= 0;
				   ln = fread_line(fp);
				   x1=x2=x3=x4=0;
				   sscanf(ln, "%d %d %d %d",
						   &x1, &x2, &x3, &x4);

				   locks			= x1;
				   pexit->key		= x2;
				   pexit->vnum		= x3;
				   pexit->vdir		= door;
				   pexit->distance	= x4;

				   switch (locks) {
					   case 1:  pexit->exit_info = EX_ISDOOR;                break;
					   case 2:  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
					   default: pexit->exit_info = locks;
				   }
			   }
		   }
		   else if (letter == 'E') {
			   ExtraDescData *ed;
			   ed = new ExtraDescData;
			   ed->keyword_     = fread_string_noheap(fp);
			   ed->description_ = fread_string_noheap(fp);
			   LINK(ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
					   next, prev);
			   top_ed++;
		   }

		   else if ( letter == 'A')     /* Scents - why A? Because it wasn't being used */
		   {
			   pRoomIndex->scentID = fread_number(fp);
		   }

		   else if ( letter == 'I')     /* Minable materials */
		   {
			   int vnum = 0;
			   int rarity = 0;

			   vnum = fread_number(fp);
			   rarity = fread_number(fp);

			   pRoomIndex->mineMap[vnum] = rarity;
		   }

		   else if (letter == 'M') {    /* maps */
			   MAP_DATA *map;
			   MAP_INDEX_DATA *map_index;
			   int i, j;

			   CREATE(map, MAP_DATA, 1);
			   map->vnum                     = fread_number(fp);
			   map->x                        = fread_number(fp);
			   map->y                        = fread_number(fp);
			   map->entry		      = fread_letter(fp);

			   pRoomIndex->map               = map;
			   if( (map_index = get_map_index(map->vnum)) == NULL ) {
				   CREATE(map_index, MAP_INDEX_DATA, 1);
				   map_index->vnum = map->vnum;
				   map_index->next = first_map;
				   first_map       = map_index;
				   for (i = 0; i <  49; i++) {
					   for (j = 0; j <  79; j++) {
						   map_index->map_of_vnums[i][j] = -1;
						   /* map_index->map_of_ptrs[i][j] = NULL; */
					   }
				   }
			   }
			   if((map->y <0) || (map->y >48))
				   bug("Map y coord out of range.  Room %d\r\n", map->y);
			   if((map->x <0) || (map->x >78))
				   bug("Map x coord out of range.  Room %d\r\n", map->x);
			   if( (map->x >0)
					   &&(map->x <80)
					   &&(map->y >0)
					   &&(map->y <48))
				   map_index->map_of_vnums[map->y][map->x]=pRoomIndex->vnum;
		   }
		   else if (letter == '>') {
			   ungetc(letter, fp);
			   rprog_read_programs(fp, pRoomIndex);
		   }
		   // Skrypt - this is (c) Ksilyan and belongs to him
		   else if ( letter == '}' )
		   {
			   pRoomIndex->RoomSkryptContainer->skryptRead(fp);

			   // Initialize Skrypt.
			   list<Argument*> arguments; // empty
			   pRoomIndex->skryptSendEvent("initRoom", arguments);
		   }
		   else {
			   bug("Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum, letter);
			   shutdown_mud("Room flag not DES");
			   exit(1);
		   }
	   }

	   if (!oldroom) {
		   iHash			 = vnum % MAX_KEY_HASH;
		   pRoomIndex->next	 = room_index_hash[iHash];
		   room_index_hash[iHash] = pRoomIndex;
		   top_room++;
	   }
   }
   return;
}

/*
 * Load a stable section.
 */
void load_stables( AREA_DATA *tarea, FILE *fp )
{
    STABLE_DATA *pStable;

    for ( ; ; )
    {
		MOB_INDEX_DATA *pMobIndex;

		CREATE( pStable, STABLE_DATA, 1 );
		pStable->keeper		= fread_number( fp );

		if ( pStable->keeper == 0 )
		{
			DISPOSE( pStable );
		    break;
		}

		pStable->stable_cost		= fread_number( fp );
		pStable->unstable_cost	= fread_number( fp );
		pStable->open_hour		= fread_number( fp );
		pStable->close_hour		= fread_number( fp );
		fread_to_eol( fp );
		pMobIndex				= get_mob_index( pStable->keeper );
		pMobIndex->pStable		= pStable;

		if ( !first_stable )
		    first_stable		= pStable;
		else
		    last_stable->next	= pStable;

		pStable->next			= NULL;
		pStable->prev			= last_stable;
		last_stable				= pStable;
		top_stable++;
	}
	return;
}


/*
 * Load a shop section.
 */
void load_shops( AREA_DATA *tarea, FILE *fp )
{
	SHOP_DATA *pShop;

	for ( ; ; )
	{
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		CREATE( pShop, SHOP_DATA, 1 );
		pShop->keeper		= fread_number( fp );
		if ( pShop->keeper == 0 )
		{
			DISPOSE(pShop);
			break;
		}
		for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
			pShop->buy_type[iTrade]	= fread_number( fp );
		pShop->profit_buy	= fread_number( fp );
		pShop->profit_sell	= fread_number( fp );
		pShop->profit_buy	= URANGE( pShop->profit_sell+5, pShop->profit_buy, 1000 );
		pShop->profit_sell	= URANGE( 0, pShop->profit_sell, pShop->profit_buy-5 );
		pShop->open_hour	= fread_number( fp );
		pShop->close_hour	= fread_number( fp );
		fread_to_eol( fp );
		pMobIndex		= get_mob_index( pShop->keeper );
		pMobIndex->pShop	= pShop;

		if ( !first_shop )
			first_shop		= pShop;
		else
			last_shop->next	= pShop;
		pShop->next		= NULL;
		pShop->prev		= last_shop;
		last_shop		= pShop;
		top_shop++;
	}
	return;
}


/*
 * Load a repair shop section.					-Thoric
 */
void load_repairs( AREA_DATA *tarea, FILE *fp )
{
	REPAIR_DATA *rShop;

	for ( ; ; )
	{
		MOB_INDEX_DATA *pMobIndex;
		int iFix;

		CREATE( rShop, REPAIR_DATA, 1 );
		rShop->keeper		= fread_number( fp );
		if ( rShop->keeper == 0 )
		{
			DISPOSE(rShop);
			break;
		}
		for ( iFix = 0; iFix < MAX_FIX; iFix++ )
		{
			rShop->fix_type[iFix] = fread_number( fp );
			if (tarea->version <= 5)
				rShop->fix_type[iFix] = itemtype_old_to_new(rShop->fix_type[iFix]);
		}
		rShop->profit_fix	= fread_number( fp );
		rShop->shop_type	= fread_number( fp );
		rShop->open_hour	= fread_number( fp );
		rShop->close_hour	= fread_number( fp );
		fread_to_eol( fp );
		pMobIndex		= get_mob_index( rShop->keeper );
		pMobIndex->rShop	= rShop;

		if ( !first_repair )
			first_repair		= rShop;
		else
			last_repair->next	= rShop;
		rShop->next		= NULL;
		rShop->prev		= last_repair;
		last_repair		= rShop;
		top_repair++;
	}
	return;
}


/*
 * Load spec proc declarations.
 */
void load_specials( AREA_DATA *tarea, FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ) );
	    pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
	    if ( pMobIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'M': vnum %s.", vnum_to_dotted(pMobIndex->vnum) );
		exit( 1 );
	    }
	    break;
	}

	fread_to_eol( fp );
    }
}

/*
 * Ksilyan
 * Start up the arena rank list.
 */

void init_arenarank()
{
	int x;
	FILE * fp;

	for (x = 0; x < NUMBER_OF_ARENA_RANKINGS; x++)
	{
		ArenaRankings[x].Rank = 0;
		strcpy(ArenaRankings[x].Name, "-");
	}

	if ( ( fp = fopen( ARENA_RANK_FILE, "r" ) ) != NULL )
	{
		char * line;
		x = 0;
		for (; ;)
		{
			int rank;
			char name[MAX_INPUT_LENGTH];

			if ( feof(fp) )
				break;
			line = fread_line(fp);
			if (strlen(line) > 0)
			{
				sscanf( line, "%d %s", &rank, name);
				ArenaRankings[x].Rank = rank;
				strcpy(ArenaRankings[x].Name, name);
				x++;
			}
		}
	}

	qsort(ArenaRankings, NUMBER_OF_ARENA_RANKINGS, sizeof(ARENA_RANK_DATA), arena_rank_sort);
	fclose(fp);

	return;
}

/*
    Ksilyan-
    used for reading in the random room descriptions.
*/

void load_randomdescs( AREA_DATA *tarea, FILE *fp )
{
    int forestdescs, desertdescs, plainsdescs, mountaindescs, hilldescs, swampdescs;
    AREA_DATA * area;
    int i, j;
    char *word;
	char buf[MAX_INPUT_LENGTH];

    forestdescs = desertdescs = plainsdescs = mountaindescs = hilldescs = swampdescs = 0;

    if (!tarea)
    {
        bug( "Load_randomdescs: no #AREA seen yet." );
        shutdown_mud( "No #AREA" );
        exit( 1 );
    }

    for ( ; ; )
    {
        word = fread_word( fp );

        if (word[0] == '$')
            break;

        if (!strcmp(word, "ForestDesc") )
        {
            if (forestdescs >= MAX_RANDOM_DESCRIPTIONS)
            {
                bug( "Load_randomdescs: too many forest descriptions." );
                continue;
            }

            tarea->random_descriptions[RANDOM_FOREST][forestdescs] = fread_string(fp);
			word = fread_word( fp );
			if (word[0] == '$')
				tarea->random_night_descriptions[RANDOM_FOREST][forestdescs] = fread_string(fp);
            tarea->random_description_counts[RANDOM_FOREST]++;
            forestdescs++;
        }
        if (!strcmp(word, "DesertDesc") )
        {
            if (desertdescs >= MAX_RANDOM_DESCRIPTIONS)
            {
                bug( "Load_randomdescs: too many desert descriptions." );
                continue;
            }

            tarea->random_descriptions[RANDOM_DESERT][desertdescs] = fread_string(fp);
			word = fread_word( fp );
			if (word[0] == '$')
				tarea->random_night_descriptions[RANDOM_DESERT][desertdescs] = fread_string(fp);
            tarea->random_description_counts[RANDOM_DESERT]++;
            desertdescs++;
        }
        if (!strcmp(word, "PlainsDesc") )
        {
            if (plainsdescs >= MAX_RANDOM_DESCRIPTIONS)
            {
                bug( "Load_randomdescs: too many plains descriptions." );
                continue;
            }

            tarea->random_descriptions[RANDOM_PLAINS][plainsdescs] = fread_string(fp);
			word = fread_word( fp );
			if (word[0] == '$')
				tarea->random_night_descriptions[RANDOM_PLAINS][plainsdescs] = fread_string(fp);
            tarea->random_description_counts[RANDOM_PLAINS]++;
            plainsdescs++;
			sprintf(buf, "Loaded one plains desc. Now at: %d descriptions.", tarea->random_description_counts[RANDOM_PLAINS]);
			log_string(buf);
        }
        if (!strcmp(word, "HillDesc") )
        {
            if (hilldescs >= MAX_RANDOM_DESCRIPTIONS)
            {
                bug( "Load_randomdescs: too many hill descriptions." );
                continue;
            }

            tarea->random_descriptions[RANDOM_HILL][hilldescs] = fread_string(fp);
			word = fread_word( fp );
			if (word[0] == '$')
				tarea->random_night_descriptions[RANDOM_HILL][hilldescs] = fread_string(fp);
            tarea->random_description_counts[RANDOM_HILL]++;
            hilldescs++;
        }
        if (!strcmp(word, "MountainDesc") )
        {
            if (mountaindescs >= MAX_RANDOM_DESCRIPTIONS)
            {
                bug( "Load_randomdescs: too many mountain descriptions." );
                continue;
            }

            tarea->random_descriptions[RANDOM_MOUNTAIN][mountaindescs] = fread_string(fp);
			word = fread_word( fp );
			if (word[0] == '$')
				tarea->random_night_descriptions[RANDOM_MOUNTAIN][mountaindescs] = fread_string(fp);
            tarea->random_description_counts[RANDOM_MOUNTAIN]++;
            mountaindescs++;
        }
		if (!strcmp(word, "SwampDesc") )
		{
			if (swampdescs >= MAX_RANDOM_DESCRIPTIONS)
			{
				bug( "Load_randomdescs: too many swamp descriptions." );
				continue;
			}

			tarea->random_descriptions[RANDOM_SWAMP][swampdescs] = fread_string(fp);
			word = fread_word( fp );
			if (word[0] == '$')
				tarea->random_night_descriptions[RANDOM_SWAMP][swampdescs] = fread_string(fp);
			tarea->random_description_counts[RANDOM_MOUNTAIN]++;
			swampdescs++;
		}
    }

    area = first_area;
    while ((area) && (strcmp(area->filename, "limbo.are") != 0))
    {
        area = area->next;
    }

	if (!area)
		return;

	for (i = 0; i <= MAX_RANDOM_DESCRIPTION_TYPE; i++)
	{
		if ( tarea->random_description_counts[i] == 0 )
		{
			for (j = 0; j < MAX_RANDOM_DESCRIPTIONS; j++)
			{
				if (area->random_descriptions[i][j])
					tarea->random_descriptions[i][j] = STRALLOC(area->random_descriptions[i][j]);
				if (area->random_night_descriptions[i][j])
					tarea->random_night_descriptions[i][j] = STRALLOC(area->random_night_descriptions[i][j]);
			}
			tarea->random_description_counts[i] = area->random_description_counts[i];
		}
	}
}

/*
 * Load soft / hard area ranges.
 */
void load_ranges( AREA_DATA *tarea, FILE *fp )
{
    int x1, x2, x3, x4;
    char *ln;

    if ( !tarea )
    {
	bug( "Load_ranges: no #AREA seen yet." );
	shutdown_mud( "No #AREA" );
	exit( 1 );
    }

    for ( ; ; )
    {
	ln = fread_line( fp );

	if (ln[0] == '$')
	  break;

	x1=x2=x3=x4=0;
	sscanf( ln, "%d %d %d %d",
	      &x1, &x2, &x3, &x4 );

	tarea->low_soft_range = x1;
	tarea->hi_soft_range = x2;
	tarea->low_hard_range = x3;
	tarea->hi_hard_range = x4;
    }
    return;

}

/*
 * Go through all areas, and set up initial economy based on mob
 * levels and gold
 */
void initialize_economy( void )
{
    AREA_DATA *tarea;
    MOB_INDEX_DATA *mob;
    int idx, gold, rng;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
	/* skip area if they already got some gold */
	if ( tarea->high_economy > 0 || tarea->low_economy > 10000 )
	  continue;
	rng = tarea->hi_soft_range - tarea->low_soft_range;
	if ( rng )
	  rng /= 2;
	else
	  rng = 25;
	gold = rng * rng * 50000;
	boost_economy( tarea, gold );
	for ( idx = tarea->low_m_vnum; idx < tarea->hi_m_vnum; idx++ )
	    if ( (mob=get_mob_index(idx)) != NULL )
		boost_economy( tarea, mob->gold * 10 );
    }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
	ROOM_INDEX_DATA *pRoomIndex;
	ExitData *pexit, *pexit_next, *rev_exit;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for ( pRoomIndex  = room_index_hash[iHash];
				pRoomIndex;
				pRoomIndex  = pRoomIndex->next )
		{
			bool fexit;

			fexit = FALSE;
			for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
			{
				pexit_next = pexit->next;
				pexit->rvnum = pRoomIndex->vnum;
				if ( pexit->vnum <= 0
						||  (pexit->to_room=get_room_index(pexit->vnum)) == NULL )
				{
					if ( fBootDb )
					{
						string vnum1 = vnum_to_dotted(pRoomIndex->vnum);
						string vnum2 = vnum_to_dotted(pexit->vnum);
						boot_log( "Fix_exits: room %s, exit %s leads to bad vnum (%s)",
								vnum1.c_str(), dir_name[pexit->vdir], vnum2.c_str() );
					}

					bug( "Deleting %s exit in room %s", dir_name[pexit->vdir],
							vnum_to_dotted(pRoomIndex->vnum) );
					extract_exit( pRoomIndex, pexit );
				}
				else
					fexit = TRUE;
			}
			if ( !fexit )
				SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
		}
	}
	boot_log(" Fix_exits: setting rexit pointers...");
	/* Set all the rexit pointers 	-Thoric */
	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for ( pRoomIndex  = room_index_hash[iHash];
				pRoomIndex;
				pRoomIndex  = pRoomIndex->next )
		{
			for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
			{
				if ( pexit->to_room && !pexit->rexit )
				{
					rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
					if ( rev_exit )
					{
						pexit->rexit	= rev_exit;
						rev_exit->rexit	= pexit;
					}
				}
			}
		}
	}
	boot_log("Fix_exits: done");
	return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
ExitData *get_exit_number( ROOM_INDEX_DATA *room, int xit )
{
    ExitData *pexit;
    int count;

    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
	if ( ++count == xit )
	  return pexit;
    return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( ExitData **xit1, ExitData **xit2 )
{
    int d1, d2;

    d1 = (*xit1)->vdir;
    d2 = (*xit2)->vdir;

    if ( d1 < d2 )
      return -1;
    if ( d1 > d2 )
      return 1;
    return 0;
}

void sort_exits( ROOM_INDEX_DATA *room )
{
	ExitData *pexit; /* *texit */ /* Unused */
	ExitData *exits[MAX_REXITS];
	int x, nexits;

	nexits = 0;
	for ( pexit = room->first_exit; pexit; pexit = pexit->next )
	{
		exits[nexits++] = pexit;
		if ( nexits > MAX_REXITS )
		{
			bug( "sort_exits: more than %d exits in room... fatal", nexits );
			return;
		}
	}
	qsort( &exits[0], nexits, sizeof( ExitData * ),
			(int(*)(const void *, const void *)) exit_comp );
	for ( x = 0; x < nexits; x++ )
	{
		if ( x > 0 )
			exits[x]->prev	= exits[x-1];
		else
		{
			exits[x]->prev	= NULL;
			room->first_exit	= exits[x];
		}
		if ( x >= (nexits - 1) )
		{
			exits[x]->next	= NULL;
			room->last_exit	= exits[x];
		}
		else
			exits[x]->next	= exits[x+1];
	}
}

void randomize_exits( ROOM_INDEX_DATA *room, sh_int maxdir )
{
    ExitData *pexit;
    int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
    int vdirs[MAX_REXITS];

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       vdirs[nexits++] = pexit->vdir;

    for ( d0 = 0; d0 < nexits; d0++ )
    {
	if ( vdirs[d0] > maxdir )
	  continue;
	count = 0;
	while ( vdirs[(d1 = number_range( d0, nexits - 1 ))] > maxdir
	||      ++count > 5 );
	if ( vdirs[d1] > maxdir )
	  continue;
	door		= vdirs[d0];
	vdirs[d0]	= vdirs[d1];
	vdirs[d1]	= door;
    }
    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       pexit->vdir = vdirs[count++];

    sort_exits( room );
}


/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
	CHAR_DATA *pch;
	int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;




	if ( (reset_age == -1 && pArea->age == -1)
	||    ++pArea->age < (reset_age-1) )
	    continue;

	/*
	 * Check for PC's.
	 */
	if ( pArea->nplayer > 0 && pArea->age == (reset_age-1) && pArea->resetmsg && pArea->resetmsg[0] != '\0')
	{
	    char buf[MAX_STRING_LENGTH];

	    /* Rennard */
	    if ( pArea->resetmsg )
		sprintf( buf, "%s\r\n", pArea->resetmsg );
	    else
		strcpy( buf, "You hear some squeaking sounds...\r\n" );
	    for ( pch = first_char; pch; pch = pch->next )
	    {


		if ( !IS_NPC(pch)
		&&   IS_AWAKE(pch)
		&&   pch->GetInRoom()
		&&   pch->GetInRoom()->area == pArea
        )
		{
            if ( IS_SET(pch->act, PLR_TICKMSG) ) {
            set_char_color( AT_RESET, pch );
		    send_to_char( buf, pch );
            }
		}
	    }
	}

	/*
	 * Check age and reset.
	 * Note: Mud Academy resets every 3 minutes (not 15).
	 */
	if ( pArea->nplayer == 0 || pArea->age >= reset_age )
	{
	    ROOM_INDEX_DATA *pRoomIndex;

/*	    fprintf( stderr, "Resetting: %s\n", pArea->filename ); spammy */
	    reset_area( pArea );
	    if ( reset_age == -1 )
		pArea->age = -1;
	    else
		pArea->age = number_range( 0, reset_age / 5 );
	    pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
	    if ( pRoomIndex != NULL && pArea == pRoomIndex->area
	    &&   pArea->reset_frequency == 0 )
		pArea->age = 15 - 3;
	}
    }
    return;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
	CHAR_DATA *mob;

	if ( !pMobIndex )
	{
		bug( "Create_mobile: NULL pMobIndex." );
		exit( 1 );
	}

	mob = new Character();
	mob->pIndexData		= pMobIndex;

	mob->editor         = NULL;
	mob->setName         ( pMobIndex->playerName_ );
	mob->setShort        ( pMobIndex->shortDesc_  );
	mob->longDesc_      = pMobIndex->longDesc_;
	mob->exitDesc_      = pMobIndex->exitDesc_;
	mob->enterDesc_     = pMobIndex->enterDesc_;
	mob->description_   = pMobIndex->description_;
	mob->spec_fun       = pMobIndex->spec_fun;
	mob->mpscriptpos    = 0;
	mob->level			= number_fuzzy( pMobIndex->level );
	mob->act			= pMobIndex->act;
	mob->affected_by		= pMobIndex->affected_by;
	mob->alignment		= pMobIndex->alignment;
	mob->sex			= pMobIndex->sex;

	if ( !pMobIndex->ac )
		mob->armor		= pMobIndex->ac;
	else
		mob->armor		= interpolate( mob->level, 100, -100 );

	if ( !pMobIndex->hitnodice )
		mob->max_hit		= mob->level * 8 + number_range(
				mob->level * mob->level / 4,
				mob->level * mob->level );
	else
	{
		sh_int i = 0;
		mob->max_hit = 0;

		for (i = 1; i <= pMobIndex->hitnodice; i++)
			mob->max_hit += number_range(1, pMobIndex->hitsizedice );
	}
	mob->max_hit += pMobIndex->hitplus;
	mob->hit			= mob->max_hit;

	// Give the mob MP proportional to its level
	mob->max_move = 85 + mob->level * 15;
	mob->move = mob->max_move;


	/* lets put things back the way they used to be! -Thoric */
	mob->gold			= pMobIndex->gold;
	mob->exp			= pMobIndex->exp;
	mob->position		= pMobIndex->position;
	mob->defposition		= pMobIndex->defposition;
	mob->barenumdie		= pMobIndex->damnodice;
	mob->baresizedie		= pMobIndex->damsizedice;
	mob->mobthac0		= pMobIndex->mobthac0;
	mob->hitplus		= pMobIndex->hitplus;
	mob->damplus		= pMobIndex->damplus;

	mob->perm_str		= pMobIndex->perm_str;
	mob->perm_dex		= pMobIndex->perm_dex;
	mob->perm_wis		= pMobIndex->perm_wis;
	mob->perm_int		= pMobIndex->perm_int;
	mob->perm_con		= pMobIndex->perm_con;
	mob->perm_cha		= pMobIndex->perm_cha;
	mob->perm_lck 		= pMobIndex->perm_lck;
	mob->hitroll		= pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damroll;
	mob->race			= pMobIndex->race;
	mob->Class			= pMobIndex->Class;
	mob->xflags			= pMobIndex->xflags;
	mob->saving_poison_death	= pMobIndex->saving_poison_death;
	mob->saving_wand		= pMobIndex->saving_wand;
	mob->saving_para_petri	= pMobIndex->saving_para_petri;
	mob->saving_breath		= pMobIndex->saving_breath;
	mob->saving_spell_staff	= pMobIndex->saving_spell_staff;
	mob->height			= pMobIndex->height;
	mob->weight			= pMobIndex->weight;
	mob->resistant		= pMobIndex->resistant;
	mob->immune			= pMobIndex->immune;
	mob->susceptible		= pMobIndex->susceptible;
	mob->attacks		= pMobIndex->attacks;
	mob->defenses		= pMobIndex->defenses;
	mob->numattacks		= pMobIndex->numattacks;
	mob->speaks			= pMobIndex->speaks;
	mob->speaking		= pMobIndex->speaking;
	mob->HomeVnum		= pMobIndex->HomeVnum;

	/*
	 * Insert in list.
	 */
	add_char( mob );
	pMobIndex->count++;
	nummobsloaded++;

	mob->skryptSetContainer( pMobIndex );

	// Initialize Skrypt.
	list<Argument*> arguments; // empty
	mob->skryptSendEvent("initMob", arguments);
	return mob;
}



/*
 * Create an instance of an object.
 */

/* USE_OBJECT_LEVELS */
/* Delete this if perm. removing levels on objs */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
	OBJ_DATA *obj;

	if ( !pObjIndex )
	{
		bug( "Create_object: NULL pObjIndex." );
		exit( 1 );
	}

	obj = new Object();

	obj->pIndexData = pObjIndex;

#ifdef USE_OBJECT_LEVELS
	obj->level		= level;
#endif
	obj->wear_loc	= -1;
	obj->count		= 1;
	cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
	obj->serial = obj->pIndexData->serial = cur_obj_serial;

	obj->name_        = pObjIndex->name_;
	obj->shortDesc_   = pObjIndex->shortDesc_;
	obj->longDesc_    = pObjIndex->longDesc_;
	obj->actionDesc_  = pObjIndex->actionDesc_;
	obj->item_type	= pObjIndex->item_type;
	obj->extra_flags	= pObjIndex->extra_flags;
	obj->extra_flags_2	= pObjIndex->extra_flags_2;
	obj->wear_flags = pObjIndex->wear_flags;
	obj->value[0]	= pObjIndex->value[0];
	obj->value[1]	= pObjIndex->value[1];
	obj->value[2]	= pObjIndex->value[2];
	obj->value[3]	= pObjIndex->value[3];
	obj->value[4]	= pObjIndex->value[4];
	obj->value[5]	= pObjIndex->value[5];
	obj->weight 	= pObjIndex->weight;
	obj->cost		= pObjIndex->cost;
	obj->max_condition = pObjIndex->max_condition;
	obj->condition = pObjIndex->max_condition;

	if ( obj->max_condition == 0 )
		obj->max_condition = obj->condition = 12;

	/* matthew */
	obj->pObjNPC	= NULL;

	/*
	obj->cost		= number_fuzzy( 10 )
	* number_fuzzy( level ) * number_fuzzy( level );
	*/

	/*
	* Mess with object properties.
	*/
	switch ( obj->item_type )
	{
		default:
			bug( "Read_object: vnum %s bad type.", vnum_to_dotted(pObjIndex->vnum) );
			bug( "------------------------>   %d", obj->item_type );
			break;

		case ITEM_GEM:
			if (obj->value[OBJECT_GEM_SIZE] == 0)
				obj->value[OBJECT_GEM_SIZE] = 100;
			if (obj->value[OBJECT_GEM_QUALITY] == 0)
				obj->value[OBJECT_GEM_QUALITY] = 100;
			break;

		case ITEM_ORE:
		case ITEM_LIGHT:
		case ITEM_TREASURE:
		case ITEM_FURNITURE:
		case ITEM_TRASH:
		case ITEM_CONTAINER:
		case ITEM_DRINK_CON:
		case ITEM_KEY:
		case ITEM_MEAT:
			break;
		case ITEM_FOOD:
			/*
			 * optional food condition (rotting food)		-Thoric
			 * value1 is the max condition of the food
			 * value4 is the optional initial condition
			 */
			if ( obj->value[4] )
				obj->timer = obj->value[4];
			else
				obj->timer = obj->value[1];
			break;
		case ITEM_BOAT:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
		case ITEM_FOUNTAIN:
		case ITEM_BLOOD:
		case ITEM_BLOODSTAIN:
		case ITEM_SCRAPS:
		case ITEM_PIPE:
		case ITEM_HERB_CON:
		case ITEM_HERB:
		case ITEM_INCENSE:
		case ITEM_FIRE:
		case ITEM_BOOK:
		case ITEM_SWITCH:
		case ITEM_LEVER:
		case ITEM_PULLCHAIN:
		case ITEM_BUTTON:
		case ITEM_DIAL:
		case ITEM_MATCH:
		case ITEM_TRAP:
		case ITEM_MAP:
		case ITEM_PORTAL:
		case ITEM_PAPER:
		case ITEM_PEN:
		case ITEM_TINDER:
		case ITEM_LOCKPICK:
		case ITEM_SPIKE:
		case ITEM_DISEASE:
		case ITEM_OIL:
		case ITEM_FUEL:
		case ITEM_QUIVER:
		case ITEM_SCABBARD:
		case ITEM_SHOVEL:
			break;

		case ITEM_SALVE:
			obj->value[3]	= number_fuzzy( obj->value[3] );
			break;

		case ITEM_SCROLL:
			obj->value[0]	= number_fuzzy( obj->value[0] );
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
			obj->value[0]	= number_fuzzy( obj->value[0] );
			obj->value[1]	= number_fuzzy( obj->value[1] );
			obj->value[2]	= obj->value[1];
			break;

		case ITEM_WEAPON:
		case ITEM_PROJECTILE:
			if ( obj->value[1] && obj->value[2] )
				obj->value[2] += 0;
			else
			{
				obj->value[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
				obj->value[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
			}
			if (obj->value[0] == 0)
				obj->value[0] = INIT_WEAPON_CONDITION;
			break;

		case ITEM_ARMOR:
			if ( obj->value[0] == 0 )
				obj->value[0]	= number_fuzzy( level / 4 + 2 );
			if (obj->value[1] == 0)
				obj->value[1] = obj->value[0];
			break;

		case ITEM_POTION:
		case ITEM_PILL:
			obj->value[0]	= number_fuzzy( number_fuzzy( obj->value[0] ) );
			break;

		case ITEM_MONEY:
			obj->value[0]	= obj->cost;
			break;
	}

	LINK( obj, first_object, last_object, next, prev );
	++pObjIndex->count;
	++pObjIndex->total_count;
	++numobjsloaded;
	++physicalobjects;

	obj->skryptSetContainer( pObjIndex );

	// Initialize Skrypt.
	list<Argument*> arguments; // empty
	obj->skryptSendEvent("initObj", arguments);

	return obj;
}


/*
 * Free a character.
 */
void free_char( const Character * ch )
{
	delete ch; // automatically removes from map
}
void free_char( Character *& ch )
{
	delete ch; // automatically removes from map
	ch = NULL;
}



/*
 * Get an extra description from a list.
 */
const char *get_extra_descr( const char *name, ExtraDescData *ed )
{
    char time_name[MAX_STRING_LENGTH];
    ExtraDescData *ped;

    if ( weather_info.sunlight!=SUN_DARK ) {
        sprintf(time_name, "%s_day",   name);
    } else {
        sprintf(time_name, "%s_night", name);
    }

    for ( ped = ed; ped; ped = ped->next )
    {
	    if ( is_name(time_name, ped->keyword_.c_str()) )
        {
	        return ped->description_.c_str();
        }
    }
    for ( ped = ed; ped; ped = ped->next )
    {
	    if ( is_name(     name, ped->keyword_.c_str()) )
        {
	        return ped->description_.c_str();
        }
    }

    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
	MOB_INDEX_DATA *pMobIndex;

	if ( vnum < 0 )
		vnum = 0;

	for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	      pMobIndex;
	      pMobIndex  = pMobIndex->next )
	{
		if ( pMobIndex->vnum == vnum )
			return pMobIndex;
	}

	if ( fBootDb )
		bug( "Get_mob_index: bad vnum %s.", vnum_to_dotted(vnum) );

	return NULL;
}



/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ( vnum < 0 )
      vnum = 0;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex;
	  pObjIndex  = pObjIndex->next )
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;

    if ( fBootDb )
	bug( "Get_obj_index: bad vnum %s.", vnum_to_dotted(vnum) );

    return NULL;
}



/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
	ROOM_INDEX_DATA *pRoomIndex;

	if ( vnum < 0 )
		vnum = 0;

	for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
			pRoomIndex;
			pRoomIndex  = pRoomIndex->next )
		if ( pRoomIndex->vnum == vnum )
			return pRoomIndex;

	if ( fBootDb )
		bug( "Get_room_index: bad vnum %s.", vnum_to_dotted(vnum) );

	return NULL;
}



/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */


/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_letter: EOF encountered on read.\r\n");
          if ( fBootDb )
            exit(1);
          return '\0';
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\r\n");
          if ( fBootDb )
            exit(1);
          return 0;
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format. (%c)", c );
	if ( fBootDb )
	  exit( 1 );
	return 0;
    }

    while ( isdigit(c) )
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\r\n");
          if ( fBootDb )
            exit(1);
          return number;
        }
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}


/*
 * custom str_dup using create					-Thoric
 */
char *str_dup( char const *str )
{
    static char *ret;
    int len;

    if ( !str )
	return NULL;

    len = strlen(str)+1;

    CREATE( ret, char, len );
    strcpy( ret, str );
    return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE *fp )
{
    char buf[MAX_BUFFER_SIZE];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    return STRALLOC("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return STRALLOC( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	     bug( "fread_string: string too long" );

         bug( "Stripping chars");
         while(*plast++ != '~');
	     *plast = '\0';
	     return STRALLOC( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return STRALLOC(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return STRALLOC( buf );
	}
    }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string_no_hash: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    return str_dup("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return str_dup( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	   bug( "fread_string_no_hash: string too long" );
	   *plast = '\0';
	   return str_dup( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string_no_hash: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return str_dup(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return str_dup( buf );
	}
    }
}

/*
 * Read a string from file fp using STL strings (i.e. no heap allocation)
 *    - Ksilyan
 */
string fread_string_noheap( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string_no_hash: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    return string("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return string( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	   bug( "fread_string_no_hash: string too long" );
	   *plast = '\0';
	   return string( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string_no_hash: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return string(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return string( buf );
	}
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_to_eol: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    return;
	}
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE *fp )
{
    static char line[MAX_STRING_LENGTH];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    strcpy(line, "");
	    return line;
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    *pline = '\0';
	    return line;
	}
	c = getc( fp );
	*pline++ = c; ln++;
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	    bug( "fread_line: line too long" );
	    break;
	}
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    word[0] = '\0';
	    return word;
	}
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\r\n");
	    if ( fBootDb )
		exit(1);
	    *pword = '\0';
	    return word;
	}
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long" );
    exit( 1 );
    return NULL;
}


void do_memory(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	int hash;

	argument = one_argument( argument, arg );
	ch_printf( ch, "Affects %5d    Areas   %5d\r\n",  top_affect, top_area	 );
	ch_printf( ch, "ExtDes  %5d    Exits   %5d\r\n", top_ed,	 top_exit	);
	ch_printf( ch, "Helps   %5d    Resets  %5d\r\n", top_help,	 top_reset	);
	ch_printf( ch, "IdxMobs %5d    Mobs    %5d\r\n", top_mob_index, nummobsloaded );
	ch_printf( ch, "IdxObjs %5d    Objs    %5d (%d)\r\n", top_obj_index, numobjsloaded, physicalobjects );
	ch_printf( ch, "Rooms   %5d    VRooms  %5d\r\n", top_room,	 top_vroom	 );
	ch_printf( ch, "Shops   %5d    RepShps %5d\r\n", top_shop,	 top_repair );
	ch_printf( ch, "CurOq's %5d    CurCq's %5d\r\n", cur_qobjs,  cur_qchars );
	ch_printf( ch, "Players %5d    Maxplrs %5d\r\n", gTheWorld->ConnectionCount(), sysdata.maxplayers );
	ch_printf( ch, "MaxEver %5d    Topsn   %5d (%d)\r\n", sysdata.alltimemax, top_sn, MAX_SKILL );
	ch_printf( ch, "Stables %5d                    \r\n", top_stable);
	ch_printf( ch, "MaxEver time recorded at:   %s\r\n", sysdata.time_of_max );

	{
		int count = 0;
		ACCOUNT_DATA * pact;
		for ( pact = first_account; pact != NULL; pact = pact->next)
			count++;
		ch_printf(ch, "%d accounts in memory.\r\n", count);
	}

	{
		int count = 0;
		int badCount = 0;
		list<MemoryManager*>::iterator itor;
		for (itor = AllocatedMemory.begin(); itor != AllocatedMemory.end(); itor++)
		{
			count++;
			if ( !(*itor)->memoryIsAllocated() )
			{
				char buf[100];
				sprintf(buf, "%d", (*itor)->memoryGetReferences());
				ch->sendText( "Deallocated memory (" + string(buf) + " refs): " + (*itor)->codeGetClassName() );
				ch->sendText( " " + (*itor)->codeGetBasicInfo() + "\r\n");
				badCount++;
			}
		}
		ch_printf(ch, "%d memory units exist, %d of which are no longer allocated.\r\n", count, badCount);
	}

	if ( !str_cmp( arg, "check" ) )
	{
#ifdef HASHSTR
		send_to_char( check_hash(argument), ch );
#else
		send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
		return;
	}
	if ( !str_cmp( arg, "showhigh" ) )
	{
#ifdef HASHSTR
		show_high_hash( atoi(argument) );
#else
		send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
		return;
	}
	if ( argument[0] != '\0' )
		hash = atoi(argument);
	else
		hash = -1;
	if ( !str_cmp( arg, "hash" ) )
	{
#ifdef HASHSTR
		ch_printf( ch, "Hash statistics:\r\n%s", hash_stats() );
		if ( hash != -1 )
			hash_dump( hash );
#else
		send_to_char( "Hash strings not enabled.\r\n", ch );
#endif
	}

	// List the count of each class instance that is managed.
	if ( !str_cmp( arg, "classes" ) )
	{
		map<string, int> ClassCounts;

		list<MemoryManager*>::iterator itor;

		for (itor = AllocatedMemory.begin(); itor != AllocatedMemory.end(); itor++)
		{
			string name = (*itor)->codeGetClassName();

			if ( ClassCounts.find(name) == ClassCounts.end() )
			{
				// Element does not exist, so create it with count == 1.
				ClassCounts[name] = 1;
			}
			else
			{
				// Element exists, so increment count by 1.
				ClassCounts[name]++;
			}
		}

		map<string, int>::iterator itor2;

		// Print out the counts to the player
		for (itor2 = ClassCounts.begin(); itor2 != ClassCounts.end(); itor2++)
		{
			char buffer[MAX_INPUT_LENGTH];

			sprintf(buffer, "%5d %s\r\n", itor2->second, itor2->first.c_str());
			ch->sendText(buffer);
		}

		return;
	}

	return;
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 * Ooops was (number_mm() % to) + from which doesn't work -Shaddai
 */
int number_range( int from, int to )
{
    if ( (to-from) < 1 )
            return from;
    return ((number_mm() % (to-from+1)) + from);
}



/*
 * Generate a percentile roll.
 * number_mm() % 100 only does 0-99, changed to do 1-100 -Shaddai
 */
int number_percent( void )
{
    return (number_mm() % 100)+1;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm( ) & (16-1) ) > 9 )
	;

    return door;
/*    return number_mm() & 10; */
}



int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static	int	rgiState[2+55];

void init_mm( )
{
    int *piState;
    int iState;

    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;

    piState[0]	= ((int) secCurrentTime) & ((1 << 30) - 1);
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = (piState[iState-1] + piState[iState-2])
			& ((1 << 30) - 1);
    }
    return;
}



int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
      case 0: return 0;
      case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = '-';

    return;
}

const char* smash_tilde_static(const char* str)
{
    static char result[MAX_STRING_LENGTH];

    strcpy(result, str);
    smash_tilde(result);

    return result;
}

std::string SmashTilde(const std::string& str)
{
    return SmashTilde(str.c_str());
}

std::string SmashTilde(const char* str)
{
    std::string result;

    while (*str != '\0') {
        if (*str == '~') {
            result += '-';
        }
        else {
            result += *str;
        }
        str++;
    }

    return result;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = HIDDEN_TILDE;

    return;
}

char *show_tilde( char *str )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufptr;

    bufptr = buf;
    for ( ; *str != '\0'; str++, bufptr++ )
    {
	if ( *str == HIDDEN_TILDE )
	    *bufptr = '~';
	else
	    *bufptr = *str;
    }
    *bufptr = '\0';

    return buf;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
const char *aoran( const char *str )
{
    static char temp[MAX_STRING_LENGTH];

    if ( !str )
    {
	bug( "Aoran(): NULL str" );
	return "";
    }

    if ( isavowel(str[0])
    || ( strlen(str) > 1 && tolower(str[0]) == 'y' && !isavowel(str[1])) )
      strcpy( temp, "an " );
    else
      strcpy( temp, "a " );
    strcat( temp, str );
    return temp;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, const char *file, const char *str )
{
    FILE *fp;

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpLOG );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\r\n", ch );
    }
    else
    {
	fprintf( fp, "%d.%d [%11s] %s: %s\n", (int)time(NULL), rand(),
	    vnum_to_dotted(ch->GetInRoom() ? ch->GetInRoom()->vnum : 0), ch->getName().c_str(), str );
	fclose( fp );
    }

    fpLOG = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Append a string to a file.
 */
void append_to_file( const char *file, const char *str )
{
	FILE *fp;

	if ( ( fp = fopen( file, "a" ) ) == NULL )
		perror( file );
	else
	{
		fprintf( fp, "%s\n", str );
		fclose( fp );
	}

	return;
}
/*
 * Reports a bug for a cetain level.
 */
void lbug( int level, const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    struct stat fst;

    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
        long lPos;

        lPos = ftell(fpArea);
	    fseek( fpArea, 0, 0 );

        iLine = 0;

        while ( (iChar = getc(fpArea)) != EOF ) {
            if ( iChar == '\n' ) {
                iLine++;
            }
        }

        if ( iChar == EOF ) {
            fprintf(fpArea, "\n");
        }
    }

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string_plus( buf, LOG_NORMAL, level );

	if ( stat( SHUTDOWN_FILE, &fst ) != -1 )	/* file exists */
	{
	    if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
	    {
		fprintf( fp, "[*****] %s\n", buf );
		fclose( fp );
	    }
	}
    }

    strcpy( buf, "[*****] BUG: " );
    {
	va_list param;

	va_start(param, str);
	vsprintf( buf + strlen(buf), str, param );
	va_end(param);
    }
    log_string_plus( buf, LOG_NORMAL, level );

    fclose( fpLOG );
    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
	fclose( fp );
    }
    fpLOG = fopen( NULL_FILE, "r" );

    return;
}

/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
	char buf[MAX_STRING_LENGTH];
	FILE *fp;
	struct stat fst;

	if ( fpArea != NULL )
	{
		int iLine;
		int iChar;

		if ( fpArea == stdin )
		{
			iLine = 0;
		}
		else
		{
			iChar = ftell( fpArea );
			fseek( fpArea, 0, 0 );
			for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
			{
				while ( getc( fpArea ) != '\n' )
					;
			}
			fseek( fpArea, iChar, 0 );
		}

		sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
		log_string( buf );

		if ( stat( SHUTDOWN_FILE, &fst ) != -1 )	/* file exists */
		{
			if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
			{
				fprintf( fp, "[*****] %s\n", buf );
				fclose( fp );
			}
		}
	}

	strcpy( buf, "[*****] BUG: " );
	{
		va_list param;

		va_start(param, str);
		vsprintf( buf + strlen(buf), str, param );
		va_end(param);
	}
	log_string( buf );

	fclose( fpLOG );
	if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
	{
		fprintf( fp, "%s\n", buf );
		fclose( fp );
	}
	fpLOG = fopen( NULL_FILE, "r" );

	return;
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH*2];
    FILE *fp;
    va_list param;

    strcpy( buf, "[*****] BOOT: " );
    va_start(param, str);
	size_t size = MAX_STRING_LENGTH;
    int retval = vsnprintf( buf+strlen(buf), size, str, param );
	if ( retval >= int(size) )
	{
		log_string("Warning: line truncated\n\r");
	}
    va_end(param);
    log_string( buf );

    fclose( fpLOG );
    if ( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
 	fclose( fp );
    }
    fpLOG = fopen( NULL_FILE, "r" );

    return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA *ch, const char *filename )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int c;
    int num = 0;

    if ( (fp = fopen( filename, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
	while ((buf[num]=fgetc(fp)) != EOF
	&&      buf[num] != '\n'
	&&      buf[num] != '\r'
	&&      num < (MAX_STRING_LENGTH-2))
	  num++;
	c = fgetc(fp);
	if ( (c != '\n' && c != '\r') || c == buf[num] )
	  ungetc(c, fp);
	buf[num++] = '\n';
	buf[num++] = '\r';
	buf[num  ] = '\0';
	send_to_pager( buf, ch );
	num = 0;
      }
    }
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg(CHAR_DATA *ch, const char* argument)
{
    set_pager_color( AT_LOG, ch );
    show_file( ch, BOOTLOG_FILE );
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, sh_int log_type, sh_int level )
{
	char *strtime;
	int offset;

	strtime 				   = ctime( &secCurrentTime);
	strtime[strlen(strtime)-1] = '\0';
	fprintf( stderr, "%s :: %s\n", strtime, str );
	if ( strncmp( str, "Log ", 4 ) == 0 )
		offset = 4;
	else
		offset = 0;

	// If we're booting, then there is no one to hear the channels...
	// So why send anything?    -Ksilyan
	if (!gBooting)
	{
		switch( log_type )
		{
		default:
			to_channel( str + offset, CHANNEL_LOG, "Log", level );
			break;
		case LOG_BUILD:
			to_channel( str + offset, CHANNEL_BUILD, "Build", level );
			break;
		case LOG_COMM:
			to_channel( str + offset, CHANNEL_COMM, "Comm", level );
			break;
		case LOG_ALL:
			break;
		}
	}
	return;
}

/*
 * Writes output to mplay log.
 * - Ksilyan
 */

void log_multiplaying( char * str )
{
	char * strtime;
	char mp_filename[MAX_INPUT_LENGTH];
	FILE * mp_fp;

	strtime = ctime( &secCurrentTime);
	strtime[strlen(strtime)-1] = '\0';

	strftime( mp_filename, MAX_INPUT_LENGTH, "../log/multiplaying/%Y.%m.%d.log", gmtime(&secCurrentTime) );

	mp_fp = fopen( mp_filename, "a" );

	if (mp_fp) {
		fputs( strtime, mp_fp );
		fputs( " :: ", mp_fp);
		fputs( str, mp_fp );
		fputs( "\n", mp_fp );
		fclose( mp_fp );
	}
}

/*
 * wizlist builder!						-Thoric
 */

void towizfile( const char *line )
{
   int filler, i, colorcodes;
   char outline[MAX_STRING_LENGTH];
   FILE *wfp;

   outline[0] = '\0';

   if ( line && line[0] != '\0' ) {
	i = 0; colorcodes = 0;

	for(i=0; line[i+1] != '\0'; i++)
	   if(line[i] == '&' && line[i+1] != '&') colorcodes++;

	filler = (78 - (strlen(line) - 2 * colorcodes));
	if (filler < 1) filler = 1;
	filler /= 2;

	for (i = 0; i < filler; i++)
	  strcat( outline, " " );

	strcat( outline, line );
   }

   strcat( outline, "\r\n" );
   wfp = fopen( WIZLIST_FILE, "a" );

   if (wfp) {
      fputs( outline, wfp );
      fclose( wfp );
   }
}

void add_to_wizlist( char *name, int level )
{
  WIZENT *wiz, *tmp;

#ifdef DEBUG
  log_string( "Adding to wizlist..." );
#endif

  CREATE( wiz, WIZENT, 1 );
  wiz->name	= str_dup( name );
  wiz->level	= level;

  if ( !first_wiz )
  {
    wiz->last	= NULL;
    wiz->next	= NULL;
    first_wiz	= wiz;
    last_wiz	= wiz;
    return;
  }

  /* insert sort, of sorts */
  for ( tmp = first_wiz; tmp; tmp = tmp->next )
    if ( level > tmp->level )
    {
      if ( !tmp->last )
	first_wiz	= wiz;
      else
	tmp->last->next = wiz;
      wiz->last = tmp->last;
      wiz->next = tmp;
      tmp->last = wiz;
      return;
    }

  wiz->last		= last_wiz;
  wiz->next		= NULL;
  last_wiz->next	= wiz;
  last_wiz		= wiz;
  return;
}

/*
 * Wizlist builder						-Thoric
 */
void make_wizlist( )
{
	DIR *dp;
	struct dirent *dentry;
	FILE *gfp;
	const char *word;
	int ilevel, iflags;
	WIZENT *wiz, *wiznext;
	char buf[MAX_STRING_LENGTH];

	first_wiz = NULL;
	last_wiz  = NULL;

	dp = opendir( GOD_DIR );

	ilevel = 0;
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
		{
			sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
			gfp = fopen( buf, "r" );
			if ( gfp )
			{
				word = feof( gfp ) ? "End" : fread_word( gfp );
				ilevel = fread_number( gfp );
				fread_to_eol( gfp );
				word = feof( gfp ) ? "End" : fread_word( gfp );
				if ( !str_cmp( word, "Pcflags" ) )
					iflags = fread_number( gfp );
				else
					iflags = 0;
				fclose( gfp );
				if ( IS_SET(iflags,PCFLAG_RETIRED)
						|| IS_SET(iflags,PCFLAG_GUEST)
						|| ilevel>=LEVEL_IMMORTAL
				   ) /* only these make it to wizlist */
				{
					if ( IS_SET( iflags, PCFLAG_RETIRED ) )
						ilevel = MAX_LEVEL - 15;
					if ( IS_SET( iflags, PCFLAG_GUEST ) )
						ilevel = MAX_LEVEL - 16;
					add_to_wizlist( dentry->d_name, ilevel );
				}
			}
		}
		dentry = readdir( dp );
	}
	closedir( dp );

	buf[0] = '\0';
	unlink( WIZLIST_FILE );
	towizfile( " &WGods of The Darkstone&w" );
	ilevel = 65535;
	for ( wiz = first_wiz; wiz; wiz = wiz->next )
	{
		if ( wiz->level < ilevel )
		{
			if ( buf[0] )
			{
				towizfile( buf );
				buf[0] = '\0';
			}
			towizfile( "" );
			ilevel = wiz->level;
			switch(ilevel)
			{
				case MAX_LEVEL -  0: towizfile("&YLord of the Darkstone&w"); break;
				case MAX_LEVEL -  1: towizfile("&YStone Master&w");       break;
				case MAX_LEVEL -  2: towizfile("&YStone Adept&w");	      break;
				case MAX_LEVEL -  3: towizfile("&YStone Initiate&w");        break;
				case MAX_LEVEL -  4: towizfile("&GStone Seeker&w");   break;
				case MAX_LEVEL -  5: towizfile("&GArtificer&w"); break;
				case MAX_LEVEL -  6: towizfile("&GTinkerer&w");   break;
				case MAX_LEVEL -  7: towizfile("&GCreator&w");	      break;
				case MAX_LEVEL -  8: towizfile("&CEngineer&w");    break;
				case MAX_LEVEL -  9: towizfile("&CWanderer&w");      break;
				case MAX_LEVEL - 10: towizfile("&CHeroic&w");      break;
				case MAX_LEVEL - 11: towizfile("&PHeroic&w");        break;
				case MAX_LEVEL - 12: towizfile("&PHeroic&w");       break;
				case MAX_LEVEL - 13: towizfile("&PHeroic&w");       break;
				case MAX_LEVEL - 14: towizfile("&PHeroic&w");      break;
				case MAX_LEVEL - 15: towizfile("&pMythic&w");        break;
				case MAX_LEVEL - 16: towizfile("&pEmissary&w");         break;
				default:	     towizfile("&pHeroic&w");       break;
			}
		}
		if ( strlen( buf ) + strlen( wiz->name ) > 76 )
		{
			towizfile( buf );
			buf[0] = '\0';
		}
		strcat( buf, " " );
		strcat( buf, wiz->name );
		if ( strlen( buf ) > 70 )
		{
			towizfile( buf );
			buf[0] = '\0';
		}
	}

	if ( buf[0] )
		towizfile( buf );

	for ( wiz = first_wiz; wiz; wiz = wiznext )
	{
		wiznext = wiz->next;
		DISPOSE(wiz->name);
		DISPOSE(wiz);
	}
	first_wiz = NULL;
	last_wiz = NULL;
}


void do_makewizlist(CHAR_DATA *ch, const char* argument)
{
  make_wizlist();
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

long long mprog_name_to_type ( char *name )
{
   if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
   if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
   if ( !str_cmp( name, "greet_dir_prog" ) )    return GREET_DIR_PROG;
   if ( !str_cmp( name, "yell_prog"      ) )    return YELL_PROG;
   if ( !str_cmp( name, "mpwalk_finished_prog" ) ) return MPWALK_FINISHED_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;
   if ( !str_cmp( name, "time_prog"     ) )	return TIME_PROG;
   if ( !str_cmp( name, "hour_prog"     ) )	return HOUR_PROG;
   if ( !str_cmp( name, "wear_prog"     ) )	return WEAR_PROG;
   if ( !str_cmp( name, "remove_prog"   ) )	return REMOVE_PROG;
   if ( !str_cmp( name, "sac_prog"      ) )	return SAC_PROG;
   if ( !str_cmp( name, "look_prog"     ) )	return LOOK_PROG;
   if ( !str_cmp( name, "exa_prog"      ) )	return EXA_PROG;
   if ( !str_cmp( name, "zap_prog"      ) )	return ZAP_PROG;
   if ( !str_cmp( name, "get_prog"      ) ) 	return GET_PROG;
   if ( !str_cmp( name, "drop_prog"     ) )	return DROP_PROG;
   if ( !str_cmp( name, "damage_prog"   ) )	return DAMAGE_PROG;
   if ( !str_cmp( name, "repair_prog"   ) )	return REPAIR_PROG;
   if ( !str_cmp( name, "greet_prog"    ) )	return GREET_PROG;
   if ( !str_cmp( name, "randiw_prog"   ) )	return RANDIW_PROG;
   if ( !str_cmp( name, "speechiw_prog" ) )	return SPEECHIW_PROG;
   if ( !str_cmp( name, "pull_prog"	) )     return PULL_PROG;
   if ( !str_cmp( name, "push_prog"	) )     return PUSH_PROG;
   if ( !str_cmp( name, "sleep_prog"    ) )	return SLEEP_PROG;
   if ( !str_cmp( name, "rest_prog"	) )	return REST_PROG;
   if ( !str_cmp( name, "rfight_prog"   ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "enter_prog"    ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "leave_prog"    ) )	return LEAVE_PROG;
   if ( !str_cmp( name, "rdeath_prog"	) )	return DEATH_PROG;
   if ( !str_cmp( name, "script_prog"	) )	return SCRIPT_PROG;
   if ( !str_cmp( name, "use_prog"	) )	return USE_PROG;
   if ( !str_cmp( name, "command_prog" ) ) return COMMAND_PROG;
   if ( !str_cmp( name, "steal_prog") ) return STEAL_PROG;
   if ( !str_cmp( name, "otherdeath_prog") ) return OTHERDEATH_PROG;
   if ( !str_cmp( name, "search_prog") ) return SEARCH_PROG;
   return( ERROR_PROG );
}

MPROG_DATA *mprog_file_read( char *f, MPROG_DATA *mprg, MOB_INDEX_DATA *pMobIndex )
{
	char        MUDProgfile[ MAX_INPUT_LENGTH ];
	FILE       *progfile;
	char        letter;
	MPROG_DATA *mprg_next, *mprg2;
	bool        done = FALSE;

	sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

	progfile = fopen( MUDProgfile, "r" );
	if ( !progfile )
	{
		bug( "Mob: %s couldn't open mudprog file", vnum_to_dotted(pMobIndex->vnum) );
		exit( 1 );
	}

	mprg2 = mprg;
	switch ( letter = fread_letter( progfile ) )
	{
		case '>':
			break;
		case '|':
			bug( "empty mudprog file." );
			exit( 1 );
			break;
		default:
			bug( "in mudprog file syntax error." );
			exit( 1 );
			break;
	}

	while ( !done )
	{
		mprg2->type = mprog_name_to_type( fread_word( progfile ) );
		switch ( mprg2->type )
		{
			case ERROR_PROG:
				bug( "mudprog file type error" );
				exit( 1 );
				break;
			case IN_FILE_PROG:
				bug( "mprog file contains a call to file." );
				exit( 1 );
				break;
			default:
				pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
				mprg2->arglist       = fread_string( progfile );
				mprg2->comlist       = fread_string( progfile );
				switch ( letter = fread_letter( progfile ) )
				{
					case '>':
						CREATE( mprg_next, MPROG_DATA, 1 );
						mprg_next->next = mprg2;
						mprg2 = mprg_next;
						break;
					case '|':
						done = TRUE;
						break;
					default:
						bug( "in mudprog file syntax error." );
						exit( 1 );
						break;
				}
				break;
		}
	}
	fclose( progfile );
	return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_mudprogs( AREA_DATA *tarea, FILE *fp )
{
  MOB_INDEX_DATA *iMob;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_mudprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iMob = get_mob_index( value ) ) == NULL )
      {
	bug( "Load_mudprogs: vnum %s doesnt exist", vnum_to_dotted(value) );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iMob->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iMob->mudprogs = working;
      working = mprog_file_read( fread_word( fp ), working, iMob );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file MUDprograms.
 */

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
	MPROG_DATA *mprg;
	char        letter;
	bool        done = FALSE;

	if ( ( letter = fread_letter( fp ) ) != '>' )
	{
		bug( "Load_mobiles: vnum %s MUDPROG char", vnum_to_dotted(pMobIndex->vnum) );
		exit( 1 );
	}
	CREATE( mprg, MPROG_DATA, 1 );
	pMobIndex->mudprogs = mprg;

	while ( !done )
	{
		mprg->type = mprog_name_to_type( fread_word( fp ) );
		switch ( mprg->type )
		{
			case ERROR_PROG:
				bug( "Load_mobiles: vnum %s MUDPROG type.", vnum_to_dotted(pMobIndex->vnum) );
				exit( 1 );
				break;
			case IN_FILE_PROG:
				mprg = mprog_file_read( fread_string( fp ), mprg,pMobIndex );
				fread_to_eol( fp );
				switch ( letter = fread_letter( fp ) )
				{
					case '>':
						CREATE( mprg->next, MPROG_DATA, 1 );
						mprg = mprg->next;
						break;
					case '|':
						mprg->next = NULL;
						fread_to_eol( fp );
						done = TRUE;
						break;
					default:
						bug( "Load_mobiles: vnum %s bad MUDPROG.", vnum_to_dotted(pMobIndex->vnum) );
						exit( 1 );
						break;
				}
				break;
			default:
				pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
				mprg->arglist        = fread_string( fp );
				fread_to_eol( fp );
				mprg->comlist        = fread_string( fp );
				fread_to_eol( fp );
				switch ( letter = fread_letter( fp ) )
				{
					case '>':
						CREATE( mprg->next, MPROG_DATA, 1 );
						mprg = mprg->next;
						break;
					case '|':
						mprg->next = NULL;
						fread_to_eol( fp );
						done = TRUE;
						break;
					default:
						bug( "Load_mobiles: vnum %s bad MUDPROG.", vnum_to_dotted(pMobIndex->vnum) );
						exit( 1 );
						break;
				}
				break;
		}
	}

	return;

}



/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA *oprog_file_read( char *f, MPROG_DATA *mprg,
			    OBJ_INDEX_DATA *pObjIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Obj: %s couldnt open mudprog file", vnum_to_dotted(pObjIndex->vnum) );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty objprog file." );
       exit( 1 );
     break;
    default:
       bug( "in objprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "objprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "objprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	pObjIndex->progtypes = pObjIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in objprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_objprogs( AREA_DATA *tarea, FILE *fp )
{
  OBJ_INDEX_DATA *iObj;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iObj = get_obj_index( value ) ) == NULL )
      {
	bug( "Load_objprogs: vnum %s doesnt exist", vnum_to_dotted(value) );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iObj->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iObj->mudprogs = working;
      working = oprog_file_read( fread_word( fp ), working, iObj );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file OBJprograms.
 */

void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_objects: vnum %s OBJPROG char", vnum_to_dotted(pObjIndex->vnum) );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pObjIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_objects: vnum %s OBJPROG type.", vnum_to_dotted(pObjIndex->vnum) );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = oprog_file_read( fread_string( fp ), mprg,pObjIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %s bad OBJPROG.", vnum_to_dotted(pObjIndex->vnum) );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pObjIndex->progtypes = pObjIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %s bad OBJPROG.", vnum_to_dotted(pObjIndex->vnum) );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
MPROG_DATA *rprog_file_read( char *f, MPROG_DATA *mprg,
			    ROOM_INDEX_DATA *RoomIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Room: %s couldnt open roomprog file", vnum_to_dotted(RoomIndex->vnum) );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty roomprog file." );
       exit( 1 );
     break;
    default:
       bug( "in roomprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "roomprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "roomprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	RoomIndex->progtypes = RoomIndex->progtypes | mprg2->type;
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in roomprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a ROOMprogram section from the area file.
 */
void load_roomprogs( AREA_DATA *tarea, FILE *fp )
{
  ROOM_INDEX_DATA *iRoom;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iRoom = get_room_index( value ) ) == NULL )
      {
	bug( "Load_roomprogs: vnum %s doesnt exist", vnum_to_dotted(value) );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iRoom->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iRoom->mudprogs = working;
      working = rprog_file_read( fread_word( fp ), working, iRoom );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_rooms: vnum %s ROOMPROG char", vnum_to_dotted(pRoomIndex->vnum) );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pRoomIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_rooms: vnum %s ROOMPROG type.", vnum_to_dotted(pRoomIndex->vnum) );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = rprog_file_read( fread_string( fp ), mprg,pRoomIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %s bad ROOMPROG.", vnum_to_dotted(pRoomIndex->vnum) );
	     exit( 1 );
	   break;
	}
      break;
     default:
	pRoomIndex->progtypes = pRoomIndex->progtypes | mprg->type;
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %s bad ROOMPROG.", vnum_to_dotted(pRoomIndex->vnum) );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
 *    Narn, May/96
 *    Don't ask me why they return bool.. :).. oh well.. -- Alty
 * */
bool delete_room( Room *room )
{
     int hash;
     Room *prev, *limbo = get_room_index(ROOM_VNUM_LIMBO);
     OBJ_DATA *o;
     CHAR_DATA *ch;
     ExtraDescData *ed;
     ExitData *ex;
     MPROG_ACT_LIST *mpact;
     MPROG_DATA *mp;

     while ((ch = room->first_person) != NULL)
     {
	    if (!IS_NPC(ch))
	  {
	           char_from_room(ch);
	           char_to_room(ch, limbo);
	  }
	    else
	        extract_char(ch, TRUE);
     }
     while ((o = room->first_content) != NULL)
     {
         extract_obj(o, TRUE);
     }
     while ((ed = room->first_extradesc) != NULL)
     {
	    room->first_extradesc = ed->next;
		delete ed;
	    --top_ed;
     }
     while ((ex = room->first_exit) != NULL)
         extract_exit(room, ex);
     while ((mpact = room->mpact) != NULL)
     {
	    room->mpact = mpact->next;
	    DISPOSE(mpact->buf);
	    DISPOSE(mpact);
     }
     while ((mp = room->mudprogs) != NULL)
     {
	    room->mudprogs = mp->next;
	    STRFREE(mp->arglist);
	    STRFREE(mp->comlist);
	    DISPOSE(mp);
     }
     if (room->map)
     {
	    MAP_INDEX_DATA *mapi;

	    if ((mapi = get_map_index(room->map->vnum)) != NULL)
	        if (room->map->x > 0 && room->map->x < 80 &&
		              room->map->y > 0 && room->map->y < 48)
	            mapi->map_of_vnums[room->map->y][room->map->x] = -1;
	    DISPOSE(room->map);
     }

     hash = room->vnum%MAX_KEY_HASH;
     if (room == room_index_hash[hash])
         room_index_hash[hash] = room->next;
     else
     {
	    for (prev = room_index_hash[hash]; prev; prev = prev->next)
	        if (prev->next == room)
	            break;
	    if (prev)
	        prev->next = room->next;
	    else
	        bug("delete_room: room %s not in hash bucket %d.", vnum_to_dotted(room->vnum), hash);
     }

	 DeleteMemoryAllocation(room->RoomSkryptContainer);
	 room->RoomSkryptContainer = NULL;

	 delete room;

     --top_room;
     return TRUE;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA *obj )
{
     int hash;
     OBJ_INDEX_DATA *prev;
     OBJ_DATA *o, *o_next;
     ExtraDescData *ed;
     AFFECT_DATA *af;
     MPROG_DATA *mp;

     /* Remove references to object index */
     for (o = first_object; o; o = o_next)
     {
	    o_next = o->next;
	    if (o->pIndexData == obj)
	        extract_obj(o, TRUE);
     }
     while ((ed = obj->first_extradesc) != NULL)
     {
	    obj->first_extradesc = ed->next;
		delete ed;
	    --top_ed;
     }
     while ((af = obj->first_affect) != NULL)
     {
	    obj->first_affect = af->next;
	    DISPOSE(af);
	    --top_affect;
     }
     while ((mp = obj->mudprogs) != NULL)
     {
	    obj->mudprogs = mp->next;
	    STRFREE(mp->arglist);
	    STRFREE(mp->comlist);
	    DISPOSE(mp);
     }

	 obj->skryptFileName_ = "";
	 obj->skryptProgramSource_ = "";

     hash = obj->vnum%MAX_KEY_HASH;
     if (obj == obj_index_hash[hash])
         obj_index_hash[hash] = obj->next;
     else
     {
	    for (prev = obj_index_hash[hash]; prev; prev = prev->next)
	        if (prev->next == obj)
	            break;
	    if (prev)
	        prev->next = obj->next;
	    else
	        bug("delete_obj: object %s not in hash bucket %d.", vnum_to_dotted(obj->vnum), hash);
     }
	 if (obj)
		 DeleteMemoryAllocation(obj);
     --top_obj_index;
     return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA *mob )
{
	int hash;
	MOB_INDEX_DATA *prev;
	CHAR_DATA *ch, *ch_next;
	MPROG_DATA *mp;

	for (ch = first_char; ch; ch = ch_next)
	{
		ch_next = ch->next;
		if (ch->pIndexData == mob)
			extract_char(ch, TRUE);
	}
	while ((mp = mob->mudprogs) != NULL)
	{
		mob->mudprogs = mp->next;
		STRFREE(mp->arglist);
		STRFREE(mp->comlist);
		DISPOSE(mp);
	}

	if (mob->pShop)
	{
		UNLINK(mob->pShop, first_shop, last_shop, next, prev);
		DISPOSE(mob->pShop);
		--top_shop;
	}

	/* matthew */
	if ( mob->pStable ) {
		UNLINK(mob->pStable, first_stable, last_stable, next, prev);
		DISPOSE(mob->pStable);
		--top_stable;
	}

	if (mob->rShop)
	{
		UNLINK(mob->rShop, first_repair, last_repair, next, prev);
		DISPOSE(mob->rShop);
		--top_repair;
	}

	mob->skryptProgramSource_ = "";
	mob->skryptFileName_ = "";

	hash = mob->vnum%MAX_KEY_HASH;
	if (mob == mob_index_hash[hash])
		mob_index_hash[hash] = mob->next;
	else
	{
		for (prev = mob_index_hash[hash]; prev; prev = prev->next)
			if (prev->next == mob)
				break;
		if (prev)
			prev->next = mob->next;
		else
			bug("delete_mob: mobile %s not in hash bucket %d.", vnum_to_dotted(mob->vnum), hash);
	}

	if (mob)
		DeleteMemoryAllocation(mob);
	--top_mob_index;
	return TRUE;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( int vnum )
{
	ROOM_INDEX_DATA *pRoomIndex;
	int	iHash;

	pRoomIndex = new Room();
	pRoomIndex->vnum          = vnum;
	pRoomIndex->name_         = "Floating in a void";
	pRoomIndex->description_  = "";
	pRoomIndex->room_flags    = ROOM_PROTOTYPE;
	pRoomIndex->sector_type   = 1;
	pRoomIndex->light		= 0;

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;

	return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, const char *name )
{
	OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
	char buf[MAX_STRING_LENGTH];
	int	iHash;

	if ( cvnum > 0 )
		cObjIndex = get_obj_index( cvnum );
	else
		cObjIndex = NULL;
	pObjIndex = new obj_index_data();
	pObjIndex->vnum             = vnum;
	pObjIndex->name_            = name;
	pObjIndex->first_affect		= NULL;
	pObjIndex->last_affect		= NULL;
	pObjIndex->first_extradesc	= NULL;
	pObjIndex->last_extradesc	= NULL;

	if ( !cObjIndex )
	{
		sprintf( buf, "a newly created %s", name );
		pObjIndex->shortDesc_	= buf;
		sprintf( buf, "Some god dropped a newly created %s here.", name );
		pObjIndex->longDesc_      = buf;
		pObjIndex->actionDesc_	= "";

		pObjIndex->item_type		= ITEM_TRASH;
		pObjIndex->extra_flags	= ITEM_PROTOTYPE;
		pObjIndex->extra_flags_2	= 0;
		pObjIndex->wear_flags		= 0;
		pObjIndex->value[0]		= 0;
		pObjIndex->value[1]		= 0;
		pObjIndex->value[2]		= 0;
		pObjIndex->value[3]		= 0;
		pObjIndex->weight		= 1;
		pObjIndex->cost		= 0;
		pObjIndex->max_condition = 1;
	}
	else
	{
		ExtraDescData *ed,  *ced;
		AFFECT_DATA	   *paf, *cpaf;

		pObjIndex->shortDesc_   = cObjIndex->shortDesc_;
		pObjIndex->longDesc_    = cObjIndex->longDesc_;
		pObjIndex->actionDesc_  = cObjIndex->actionDesc_;
		pObjIndex->item_type		= cObjIndex->item_type;
		pObjIndex->extra_flags	= cObjIndex->extra_flags
			| ITEM_PROTOTYPE;
		pObjIndex->extra_flags_2	= cObjIndex->extra_flags_2;
		pObjIndex->wear_flags		= cObjIndex->wear_flags;
		pObjIndex->value[0]		= cObjIndex->value[0];
		pObjIndex->value[1]		= cObjIndex->value[1];
		pObjIndex->value[2]		= cObjIndex->value[2];
		pObjIndex->value[3]		= cObjIndex->value[3];
		pObjIndex->weight		= cObjIndex->weight;
		pObjIndex->cost		= cObjIndex->cost;
		pObjIndex->max_condition = cObjIndex->max_condition;
		for ( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
		{
			ed = new ExtraDescData;
			ed->keyword_       =  ced->keyword_;
			ed->description_   = ced->description_;
			LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
					next, prev );
			top_ed++;
		}
		for ( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
		{
			CREATE( paf, AFFECT_DATA, 1 );
			paf->type		= cpaf->type;
			paf->duration		= cpaf->duration;
			paf->location		= cpaf->location;
			paf->modifier		= cpaf->modifier;
			paf->bitvector		= cpaf->bitvector;
			LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
					next, prev );
			top_affect++;
		}
	}
	pObjIndex->count		= 0;
	pObjIndex->total_count  = 0;
	iHash				= vnum % MAX_KEY_HASH;
	pObjIndex->next			= obj_index_hash[iHash];
	obj_index_hash[iHash]		= pObjIndex;
	top_obj_index++;

	return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( int vnum, int cvnum, const char *name )
{
	MOB_INDEX_DATA *pMobIndex, *cMobIndex;
	char buf[MAX_STRING_LENGTH];
	int	iHash;

	if ( cvnum > 0 )
		cMobIndex = get_mob_index( cvnum );
	else
		cMobIndex = NULL;

	pMobIndex = new mob_index_data();
	pMobIndex->vnum         = vnum;
	pMobIndex->count        = 0;
	pMobIndex->killed       = 0;
	pMobIndex->playerName_  = name;
	if ( !cMobIndex )
	{
		sprintf( buf, "a newly created %s", name );
		pMobIndex->shortDesc_	= buf;
		sprintf( buf, "Some god abandoned a newly created %s here.\r\n", name );
		pMobIndex->longDesc_		= buf;
		pMobIndex->description_	= "";
		pMobIndex->enterDesc_ = pMobIndex->exitDesc_ = "";

		pMobIndex->act		= ACT_IS_NPC | ACT_PROTOTYPE;
		pMobIndex->affected_by	= 0;
		pMobIndex->pShop		= NULL;
		pMobIndex->rShop		= NULL;
		pMobIndex->spec_fun		= NULL;
		pMobIndex->mudprogs		= NULL;
		pMobIndex->progtypes		= 0;
		pMobIndex->alignment		= 0;
		pMobIndex->level		= 1;
		pMobIndex->mobthac0		= 0;
		pMobIndex->ac			= 0;
		pMobIndex->hitnodice		= 0;
		pMobIndex->hitsizedice	= 0;
		pMobIndex->hitplus		= 0;
		pMobIndex->damnodice		= 0;
		pMobIndex->damsizedice	= 0;
		pMobIndex->damplus		= 0;
		pMobIndex->gold		= 0;
		pMobIndex->exp		= 0;
		pMobIndex->position		= 8;
		pMobIndex->defposition	= 8;
		pMobIndex->sex		= 0;
		pMobIndex->perm_str		= 13;
		pMobIndex->perm_dex		= 13;
		pMobIndex->perm_int		= 13;
		pMobIndex->perm_wis		= 13;
		pMobIndex->perm_cha		= 13;
		pMobIndex->perm_con		= 13;
		pMobIndex->perm_lck		= 13;
		pMobIndex->race		= 0;
		pMobIndex->Class		= 3;
		pMobIndex->xflags		= 0;
		pMobIndex->resistant		= 0;
		pMobIndex->immune		= 0;
		pMobIndex->susceptible	= 0;
		pMobIndex->numattacks		= 0;
		pMobIndex->attacks		= 0;
		pMobIndex->defenses		= 0;
	}
	else
	{
		pMobIndex->shortDesc_	= cMobIndex->shortDesc_;
		pMobIndex->longDesc_		= cMobIndex->longDesc_;
		pMobIndex->description_ = cMobIndex->description_;
		pMobIndex->exitDesc_ = cMobIndex->exitDesc_;
		pMobIndex->enterDesc_ = cMobIndex->enterDesc_;

		pMobIndex->act		= cMobIndex->act | ACT_PROTOTYPE;
		pMobIndex->affected_by	= cMobIndex->affected_by;
		pMobIndex->pShop		= NULL;
		pMobIndex->rShop		= NULL;
		pMobIndex->spec_fun		= cMobIndex->spec_fun;
		pMobIndex->mudprogs		= NULL;
		pMobIndex->progtypes		= 0;
		pMobIndex->alignment		= cMobIndex->alignment;
		pMobIndex->level		= cMobIndex->level;
		pMobIndex->mobthac0		= cMobIndex->mobthac0;
		pMobIndex->ac			= cMobIndex->ac;
		pMobIndex->hitnodice		= cMobIndex->hitnodice;
		pMobIndex->hitsizedice	= cMobIndex->hitsizedice;
		pMobIndex->hitplus		= cMobIndex->hitplus;
		pMobIndex->damnodice		= cMobIndex->damnodice;
		pMobIndex->damsizedice	= cMobIndex->damsizedice;
		pMobIndex->damplus		= cMobIndex->damplus;
		pMobIndex->gold		= cMobIndex->gold;
		pMobIndex->exp		= cMobIndex->exp;
		pMobIndex->position		= cMobIndex->position;
		pMobIndex->defposition	= cMobIndex->defposition;
		pMobIndex->sex		= cMobIndex->sex;
		pMobIndex->perm_str		= cMobIndex->perm_str;
		pMobIndex->perm_dex		= cMobIndex->perm_dex;
		pMobIndex->perm_int		= cMobIndex->perm_int;
		pMobIndex->perm_wis		= cMobIndex->perm_wis;
		pMobIndex->perm_cha		= cMobIndex->perm_cha;
		pMobIndex->perm_con		= cMobIndex->perm_con;
		pMobIndex->perm_lck		= cMobIndex->perm_lck;
		pMobIndex->race		= cMobIndex->race;
		pMobIndex->Class		= cMobIndex->Class;
		pMobIndex->xflags		= cMobIndex->xflags;
		pMobIndex->resistant		= cMobIndex->resistant;
		pMobIndex->immune		= cMobIndex->immune;
		pMobIndex->susceptible	= cMobIndex->susceptible;
		pMobIndex->numattacks		= cMobIndex->numattacks;
		pMobIndex->attacks		= cMobIndex->attacks;
		pMobIndex->defenses		= cMobIndex->defenses;
	}
	iHash				= vnum % MAX_KEY_HASH;
	pMobIndex->next			= mob_index_hash[iHash];
	mob_index_hash[iHash]		= pMobIndex;
	top_mob_index++;

	return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
ExitData *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door )
{
	ExitData *pexit, *texit;
	bool broke;

	pexit = new ExitData;
	pexit->vdir		= door;
	pexit->rvnum		= pRoomIndex->vnum;
	pexit->to_room		= to_room;
	pexit->distance		= 1;
	if ( to_room )
	{
		pexit->vnum = to_room->vnum;
		texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );
		if ( texit )	/* assign reverse exit pointers */
		{
			texit->rexit = pexit;
			pexit->rexit = texit;
		}
	}
	broke = FALSE;
	for ( texit = pRoomIndex->first_exit; texit; texit = texit->next )
	   if ( door < texit->vdir )
	   {
	     broke = TRUE;
	     break;
	   }
	if ( !pRoomIndex->first_exit )
	  pRoomIndex->first_exit	= pexit;
	else
	{
	  /* keep exits in incremental order - insert exit into list */
	  if ( broke && texit )
	  {
	    if ( !texit->prev )
	      pRoomIndex->first_exit	= pexit;
	    else
	      texit->prev->next		= pexit;
	    pexit->prev			= texit->prev;
	    pexit->next			= texit;
	    texit->prev			= pexit;
	    top_exit++;
	    return pexit;
	  }
	  pRoomIndex->last_exit->next	= pexit;
	}
	pexit->next			= NULL;
	pexit->prev			= pRoomIndex->last_exit;
	pRoomIndex->last_exit		= pexit;
	top_exit++;
	return pexit;
}

void fix_area_exits( AREA_DATA *tarea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    ExitData *pexit, *rev_exit;
    int rnum;
    bool fexit;

    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
	if ( (pRoomIndex = get_room_index( rnum )) == NULL )
	  continue;

	fexit = FALSE;
	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		fexit = TRUE;
		pexit->rvnum = pRoomIndex->vnum;
		if ( pexit->vnum <= 0 )
	       	  pexit->to_room = NULL;
		else
		  pexit->to_room = get_room_index( pexit->vnum );
	}
	if ( !fexit )
	  SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
    }


    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
	if ( (pRoomIndex = get_room_index( rnum )) == NULL )
	  continue;

	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room && !pexit->rexit )
		{
		   rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
		   if ( rev_exit )
		   {
			pexit->rexit	= rev_exit;
			rev_exit->rexit	= pexit;
		   }
		}
	}
    }
}

void load_area_file( AREA_DATA *tarea, const char *filename )
{
/*    FILE *fpin;
    what intelligent person stopped using fpArea?????
    if fpArea isn't being used, then no filename or linenumber
    is printed when an error occurs during loading the area..
    (bug uses fpArea)
      --TRI  */

    if ( fBootDb )
      tarea = last_area;
    if ( !fBootDb && !tarea )
    {
	bug( "Load_area: null area!" );
	return;
    }

    if ( ( fpArea = fopen( filename, "r" ) ) == NULL )
    {
	perror( filename );
	bug( "load_area: error loading file (can't open)" );
	bug( filename );
	return;
    }

   for ( ; ; ) {
      char *word;

      if ( fread_letter( fpArea ) != '#' ) {
	 bug( tarea->filename );
	 bug( "load_area: # not found." );
	 exit( 1 );
      }

      word = fread_word( fpArea );

      if ( word[0] == '$')
	break;

      else if ( !str_cmp( word, "AREA") ) {
	 if ( fBootDb ) {
	    load_area (fpArea);
	    tarea = last_area;
	 }
	 else {
	    DISPOSE( tarea->name );
	    tarea->name = fread_string_nohash( fpArea );
	 }
      }
      else if ( !str_cmp( word, "VERSION"  ) ) load_version (tarea, fpArea);
      else if ( !str_cmp( word, "AUTHOR"   ) ) load_author  (tarea, fpArea);
      else if ( !str_cmp( word, "FLAGS"    ) ) load_flags   (tarea, fpArea);
      else if ( !str_cmp( word, "RANGES"   ) ) load_ranges  (tarea, fpArea);
      else if ( !str_cmp( word, "ECONOMY"  ) ) load_economy (tarea, fpArea);
	  else if ( !str_cmp( word, "INSTALLDATE" ) ) load_installdate (tarea, fpArea);
      else if ( !str_cmp( word, "RESETMSG" ) ) load_resetmsg(tarea, fpArea);
      /* Rennard */
      else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (tarea, fpArea);
      else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (tarea, fpArea);
      else if ( !str_cmp( word, "MUDPROGS" ) ) load_mudprogs(tarea, fpArea);
      else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (tarea, fpArea);
      else if ( !str_cmp( word, "OBJPROGS" ) ) load_objprogs(tarea, fpArea);
      else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (tarea, fpArea);
      else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (tarea, fpArea);
      else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (tarea, fpArea);
      else if ( !str_cmp( word, "STABLES"  ) ) load_stables (tarea, fpArea);
      else if ( !str_cmp( word, "REPAIRS"  ) ) load_repairs (tarea, fpArea);
      else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(tarea, fpArea);
      else if ( !str_cmp( word, "TRAINERS" ) ) load_trainers(tarea, fpArea);
      else if ( !str_cmp( word, "TRAINERSKILLS" ) ) load_trainerskills(tarea, fpArea);
            /* KSILYAN */
      else if ( !str_cmp( word, "RANDOMDESCS" ) ) load_randomdescs(tarea, fpArea);

      else {
	 bug( tarea->filename );
	 bug( "load_area: bad section name." );
	 if ( fBootDb )
	   exit( 1 );
	 else {
	    fclose( fpArea );
	    fpArea = NULL;
	    return;
	 }
      }
   }

   fclose( fpArea );
   fpArea = NULL;

   if ( tarea) {
      if ( fBootDb ) {
	sort_area( tarea, FALSE );
      }

      /* TODO: fix this ! */
      fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d Mobs: %5d - %d\n",
	      tarea->filename,
	      tarea->low_r_vnum, tarea->hi_r_vnum,
	      tarea->low_o_vnum, tarea->hi_o_vnum,
	      tarea->low_m_vnum, tarea->hi_m_vnum );

      if ( !tarea->author )
	tarea->author = STRALLOC( "" );

      SET_BIT( tarea->status, AREA_LOADED );
   }
   else
     fprintf( stderr, "(%s)\n", filename );
}



/* Build list of in_progress areas.  Do not load areas.
 * define AREA_READ if you want it to build area names rather than reading
 * them out of the area files. -- Altrag */
void load_buildlist( void )
{
	DIR *dp;
	struct dirent *dentry;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	char line[81];
	char word[81];
	int low, hi;
	int mlow, mhi, olow, ohi, rlow, rhi;
	bool badfile = FALSE;
	char temp;

	dp = opendir( GOD_DIR );
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
		{
			sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
			if ( !(fp = fopen( buf, "r" )) )
			{
				bug( "Load_buildlist: invalid file" );
				perror( buf );
				dentry = readdir(dp);
				continue;
			}
			log_string( buf );
			badfile = FALSE;
			rlow=rhi=olow=ohi=mlow=mhi=0;
			while ( !feof(fp) && !ferror(fp) )
			{
				low = 0; hi = 0; word[0] = 0; line[0] = 0;
				if ( (temp = fgetc(fp)) != EOF )
					ungetc( temp, fp );
				else
					break;

				fgets(line, 80, fp);
				sscanf( line, "%s %d %d", word, &low, &hi );
				if ( !strcmp( word, "Level" ) )
				{
#if 0 /* Testaur allows non-immortal building */
					if ( low < LEVEL_IMMORTAL )
					{
						sprintf( buf, "%s: God file with level %d < %d",
							dentry->d_name, low, LEVEL_IMMORTAL );
						badfile = TRUE;
					}
#endif
				}
				if ( !strcmp( word, "RoomRange" ) )
					rlow = low, rhi = hi;
				else if ( !strcmp( word, "MobRange" ) )
					mlow = low, mhi = hi;
				else if ( !strcmp( word, "ObjRange" ) )
					olow = low, ohi = hi;
			}
			fclose( fp );
			if ( rlow && rhi && !badfile )
			{
				sprintf( buf, "%s%s.are", BUILD_DIR, dentry->d_name );
				if ( !(fp = fopen( buf, "r" )) )
				{
					bug( "Load_buildlist: cannot open area file for read" );
					perror( buf );
					dentry = readdir(dp);
					continue;
				}
#if !defined(READ_AREA)  /* Dont always want to read stuff.. dunno.. shrug */
				strcpy( word, fread_word( fp ) );
				if ( word[0] != '#' || strcmp( &word[1], "AREA" ) )
				{
					sprintf( buf, "Make_buildlist: %s.are: no #AREA found.",
						dentry->d_name );
					fclose( fp );
					dentry = readdir(dp);
					continue;
				}
#endif
				pArea = new Area();
				sprintf( buf, "%s.are", dentry->d_name );
				pArea->author = STRALLOC( dentry->d_name );
				pArea->filename = str_dup( buf );
#if !defined(READ_AREA)
				pArea->name = fread_string_nohash( fp );
#else
				sprintf( buf, "{PROTO} %s's area in progress", dentry->d_name );
				pArea->name = str_dup( buf );
#endif
				fclose( fp );
				pArea->low_r_vnum = rlow; pArea->hi_r_vnum = rhi;
				pArea->low_m_vnum = mlow; pArea->hi_m_vnum = mhi;
				pArea->low_o_vnum = olow; pArea->hi_o_vnum = ohi;
				pArea->low_soft_range = -1; pArea->hi_soft_range = -1;
				pArea->low_hard_range = -1; pArea->hi_hard_range = -1;
				pArea->first_reset = NULL; pArea->last_reset = NULL;
				LINK( pArea, first_build, last_build, next, prev );
				fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d "
								 "Mobs: %5d - %-5d\n",
					pArea->filename,
					pArea->low_r_vnum, pArea->hi_r_vnum,
					pArea->low_o_vnum, pArea->hi_o_vnum,
					pArea->low_m_vnum, pArea->hi_m_vnum );
				sort_area( pArea, TRUE );

				sprintf( buf, "%s%s.are", BUILD_DIR, dentry->d_name );
                load_area_file(pArea, buf);
                fix_area_exits(pArea);
			}
		}
		dentry = readdir(dp);
	}
	closedir(dp);
}


/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA *pArea, bool proto )
{
    AREA_DATA *area = NULL;
    AREA_DATA *first_sort, *last_sort;
    bool found;

    if ( !pArea )
    {
	bug( "Sort_area: NULL pArea" );
	return;
    }

    if ( proto )
    {
	first_sort = first_bsort;
	last_sort  = last_bsort;
    }
    else
    {
	first_sort = first_asort;
	last_sort  = last_asort;
    }

    found = FALSE;
    pArea->next_sort = NULL;
    pArea->prev_sort = NULL;

    if ( !first_sort )
    {
	pArea->prev_sort = NULL;
	pArea->next_sort = NULL;
	first_sort	 = pArea;
	last_sort	 = pArea;
	found = TRUE;
    }
    else
    for ( area = first_sort; area; area = area->next_sort )
	if ( pArea->low_r_vnum < area->low_r_vnum )
	{
	    if ( !area->prev_sort )
	      first_sort	= pArea;
	    else
	      area->prev_sort->next_sort = pArea;
	    pArea->prev_sort = area->prev_sort;
	    pArea->next_sort = area;
	    area->prev_sort  = pArea;
	    found = TRUE;
	    break;
        }

    if ( !found )
    {
	pArea->prev_sort     = last_sort;
	pArea->next_sort     = NULL;
	last_sort->next_sort = pArea;
	last_sort	     = pArea;
    }

    if ( proto )
    {
	first_bsort = first_sort;
	last_bsort  = last_sort;
    }
    else
    {
	first_asort = first_sort;
	last_asort  = last_sort;
    }
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA *ch, int low, int high, bool proto, bool shownl,
		 const char *loadst, const char *notloadst )
{
    AREA_DATA *pArea, *first_sort;
    int count, loaded;

    count = 0;	loaded = 0;
    set_pager_color( AT_PLAIN, ch );
    if ( proto )
      first_sort = first_bsort;
    else
      first_sort = first_asort;
    for ( pArea = first_sort; pArea; pArea = pArea->next_sort )
    {
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	if ( pArea->low_r_vnum < low )
	   continue;
	if ( pArea->hi_r_vnum > high )
	   break;
	if ( IS_SET(pArea->status, AREA_LOADED) )
	   loaded++;
	else
	if ( !shownl )
	   continue;

        pager_printf(ch, "%-20s| Rooms: ",
            (pArea->filename ? pArea->filename : "(invalid)"));
        pager_printf(ch, "%9s - ",  vnum_to_dotted(pArea->low_r_vnum));
        pager_printf(ch, "%9s ",    vnum_to_dotted(pArea->hi_r_vnum));
        pager_printf(ch, "Mobs: ");
        pager_printf(ch, "%9s - ",  vnum_to_dotted(pArea->low_m_vnum));
        pager_printf(ch, "%9s ",    vnum_to_dotted(pArea->hi_m_vnum));
        pager_printf(ch, "Objs: ");
        pager_printf(ch, "%9s - ",  vnum_to_dotted(pArea->low_o_vnum));
        pager_printf(ch, "%9s\r\n", vnum_to_dotted(pArea->hi_o_vnum));

        count++;
    }
    pager_printf( ch, "Areas listed: %d  Loaded: %d\r\n", count, loaded );
    return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */
void do_vnums(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1048575999;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1048575999;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, FALSE, TRUE, "", " X" );
}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_newzones(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1048575999;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}

/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
    FILE *fp;
    char filename[MAX_INPUT_LENGTH];

    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_sysdata: fopen" );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#SYSTEM\n" );
	fprintf( fp, "Highplayers    %d\n", sys.alltimemax		);
	fprintf( fp, "Highplayertime %s~\n", sys.time_of_max		);
	fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING	);
	fprintf( fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH		);
	fprintf( fp, "Readallmail    %d\n", sys.read_all_mail		);
	fprintf( fp, "Readmailfree   %d\n", sys.read_mail_free		);
	fprintf( fp, "Writemailfree  %d\n", sys.write_mail_free		);
	fprintf( fp, "Takeothersmail %d\n", sys.take_others_mail	);
	fprintf( fp, "Muse           %d\n", sys.muse_level		);
	fprintf( fp, "Think          %d\n", sys.think_level		);
	fprintf( fp, "Build          %d\n", sys.build_level		);
	fprintf( fp, "Log            %d\n", sys.log_level		);
	fprintf( fp, "Protoflag      %d\n", sys.level_modify_proto	);
    fprintf( fp, "ModProtoflag   %d\n", sys.level_modify_proto_flag );
	fprintf( fp, "Overridepriv   %d\n", sys.level_override_private	);
	fprintf( fp, "Msetplayer     %d\n", sys.level_mset_player	);
	fprintf( fp, "Stunplrvsplr   %d\n", sys.stun_plr_vs_plr		);
	fprintf( fp, "Stunregular    %d\n", sys.stun_regular		);
	fprintf( fp, "Damplrvsplr    %d\n", sys.dam_plr_vs_plr		);
	fprintf( fp, "Damplrvsmob    %d\n", sys.dam_plr_vs_mob		);
	fprintf( fp, "Dammobvsplr    %d\n", sys.dam_mob_vs_plr		);
	fprintf( fp, "Dammobvsmob    %d\n", sys.dam_mob_vs_mob		);
	fprintf( fp, "Forcepc        %d\n", sys.level_forcepc		);
	fprintf( fp, "Guildoverseer  %s~\n", sys.guild_overseer		);
	fprintf( fp, "Guildadvisor   %s~\n", sys.guild_advisor		);
	fprintf( fp, "Saveflags      %d\n", sys.save_flags		);
	fprintf( fp, "Savefreq       %d\n", sys.save_frequency		);
    fprintf( fp, "level_interrupt_buffer %d\n", sys.level_interrupt_buffer);
    fprintf( fp, "level_monitor %d\n", sys.level_monitor);
    fprintf( fp, "level_restore_all %d\n", sys.level_restore_all);
    fprintf( fp, "level_restore_on_player %d\n", sys.level_restore_on_player );
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void fread_sysdata( SYSTEM_DATA *sys, FILE *fp )
{
    const char *word;
    bool fMatch;

    sys->time_of_max = NULL;
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
	case 'B':
	    KEY( "Build",	   sys->build_level,	  fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Damplrvsplr",	   sys->dam_plr_vs_plr,	  fread_number( fp ) );
	    KEY( "Damplrvsmob",	   sys->dam_plr_vs_mob,	  fread_number( fp ) );
	    KEY( "Dammobvsplr",	   sys->dam_mob_vs_plr,	  fread_number( fp ) );
	    KEY( "Dammobvsmob",	   sys->dam_mob_vs_mob,	  fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !sys->time_of_max )
		    sys->time_of_max = str_dup("(not recorded)");
		return;
	    }
	    break;

	case 'F':
	    KEY( "Forcepc",	   sys->level_forcepc,	  fread_number( fp ) );
	    break;

	case 'G':
	    KEY( "Guildoverseer",  sys->guild_overseer,  fread_string( fp ) );
	    KEY( "Guildadvisor",   sys->guild_advisor,   fread_string( fp ) );
	    break;

	case 'H':
	    KEY( "Highplayers",	   sys->alltimemax,	  fread_number( fp ) );
	    KEY( "Highplayertime", sys->time_of_max,      fread_string_nohash( fp ) );
	    break;

	case 'L':
        KEY("level_interrupt_buffer", sys->level_interrupt_buffer, fread_number(fp));
        KEY("level_monitor", sys->level_monitor, fread_number(fp));
        KEY("level_restore_all", sys->level_restore_all, fread_number(fp));
        KEY("level_restore_on_player", sys->level_restore_on_player, fread_number(fp));                	    KEY( "Log",		   sys->log_level,	  fread_number( fp ) );
	    break;

	case 'M':
	    KEY( "Msetplayer",	   sys->level_mset_player, fread_number( fp ) );
	    KEY( "Muse",	   sys->muse_level,	   fread_number( fp ) );
        KEY( "ModProtoflag", sys->level_modify_proto_flag, fread_number(fp));
	    break;

	case 'N':
            KEY( "Nameresolving",  sys->NO_NAME_RESOLVING, fread_number( fp ) );
	    break;

	case 'O':
	    KEY( "Overridepriv",   sys->level_override_private, fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Protoflag",	   sys->level_modify_proto, fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Readallmail",	   sys->read_all_mail,	fread_number( fp ) );
	    KEY( "Readmailfree",   sys->read_mail_free,	fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Stunplrvsplr",   sys->stun_plr_vs_plr, fread_number( fp ) );
	    KEY( "Stunregular",    sys->stun_regular,	fread_number( fp ) );
	    KEY( "Saveflags",	   sys->save_flags,	fread_number( fp ) );
	    KEY( "Savefreq",	   sys->save_frequency,	fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Takeothersmail", sys->take_others_mail, fread_number( fp ) );
	    KEY( "Think",	   sys->think_level,	fread_number( fp ) );
	    break;


	case 'W':
	    KEY( "Waitforauth",	   sys->WAIT_FOR_AUTH,	  fread_number( fp ) );
	    KEY( "Writemailfree",  sys->write_mail_free,  fread_number( fp ) );
	    break;
	}


	if ( !fMatch )
	{
            bug( "Fread_sysdata: no match: %s", word );
	}
    }
}



/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA *sys )
{
    char filename[MAX_INPUT_LENGTH];
    FILE *fp;
    bool found;

    found = FALSE;
    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
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
		bug( "Load_sysdata_file: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "SYSTEM" ) )
	    {
	    	fread_sysdata( sys, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_sysdata_file: bad section." );
		break;
	    }
	}
	fclose( fp );
    }

    if ( !sysdata.guild_overseer ) sysdata.guild_overseer = str_dup( "" );
    if ( !sysdata.guild_advisor  ) sysdata.guild_advisor  = str_dup( "" );
    return found;
}


void load_banlist( void )
{
  BAN_DATA *pban;
  FILE *fp;
  char letter;
  char* site;

  if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "r" )) )
    return;

  for ( ; ; )
  {
    if ( feof( fp ) )
    {
      bug( "Load_banlist: no End found." );
      fclose( fp );
      return;
    }

    site = fread_string_nohash(fp);

    if ( !str_cmp(site, "End") ) {
        fclose(fp);
        return;
    }

    CREATE( pban, BAN_DATA, 1 );
    pban->site = site;
    if ( (letter = fread_letter(fp)) == '~' )
      pban->ban_time = fread_string_nohash( fp );
    else
    {
      ungetc(letter, fp);
      pban->ban_time = str_dup( "(unrecorded)" );
    }
    LINK( pban, first_ban, last_ban, next, prev );
  }
}

/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    bool room, mob, obj, all, area_conflict;
    int low_range, high_range;

    room = FALSE;
    mob  = FALSE;
    obj  = FALSE;
    all  = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0')
    {
      send_to_char("Please specify room, mob, object, or all as your first argument.\r\n", ch);
      return;
    }

    if(!str_cmp(arg1, "room"))
      room = TRUE;

    else if(!str_cmp(arg1, "mob"))
      mob = TRUE;

    else if(!str_cmp(arg1, "object"))
      obj = TRUE;

    else if(!str_cmp(arg1, "all"))
      all = TRUE;
    else
    {
#if 0
      send_to_char("Please specify room, mob, or object as your first argument.\r\n", ch);
      return;
#else
      all = TRUE;
#endif
    }

    if(arg2[0] == '\0')
    {
      send_to_char("Please specify the low end of the range to be searched.\r\n", ch);
      return;
    }

    if(argument[0] == '\0')
    {
      send_to_char("Please specify the high end of the range to be searched.\r\n", ch);
      return;
    }

    low_range = dotted_to_vnum(ch->GetInRoom()->vnum, arg2);
    high_range = dotted_to_vnum(ch->GetInRoom()->vnum, argument);

    if (low_range < 1 || low_range > 1048576000 )
    {
      send_to_char("Invalid argument for bottom of range.\r\n", ch);
      return;
    }

    if (high_range < 1 || high_range > 1048576000 )
    {
      send_to_char("Invalid argument for top of range.\r\n", ch);
      return;
    }

    if (high_range < low_range)
    {
      send_to_char("Bottom of range must be below top of range.\r\n", ch);
      return;
    }

    if (all)
    {
      sprintf(buf, "room %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "mob %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "object %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      return;
    }
    set_char_color( AT_PLAIN, ch );

    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	{
	  if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_r_vnum )
	  && ( low_range <= pArea->hi_r_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_r_vnum )
	  && ( high_range >= pArea->low_r_vnum ) )
	    area_conflict = TRUE;
	}

	if (mob)
	{
	  if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
	    area_conflict = TRUE;
	  if ( ( low_range >= pArea->low_m_vnum )
	  && ( low_range <= pArea->hi_m_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_m_vnum )
	  && ( high_range >= pArea->low_m_vnum ) )
	    area_conflict = TRUE;
	}

	if (obj)
	{
	  if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_o_vnum )
	  && ( low_range <= pArea->hi_o_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_o_vnum )
	  && ( high_range >= pArea->low_o_vnum ) )
	    area_conflict = TRUE;
	}

	if (area_conflict)
	{
	sprintf(buf, "Conflict:%-15s| ",
		(pArea->filename ? pArea->filename : "(invalid)"));
        if(room)
          sprintf( buf2, "Rooms: %5d - %-5d\r\n", pArea->low_r_vnum,
          pArea->hi_r_vnum);
        if(mob)
          sprintf( buf2, "Mobs: %5d - %-5d\r\n", pArea->low_m_vnum,
          pArea->hi_m_vnum);
        if(obj)
          sprintf( buf2, "Objects: %5d - %-5d\r\n", pArea->low_o_vnum,
          pArea->hi_o_vnum);

        strcat( buf, buf2 );
	send_to_char(buf, ch);
    	}
    }
    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	{
	  if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_r_vnum )
	  && ( low_range <= pArea->hi_r_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_r_vnum )
	  && ( high_range >= pArea->low_r_vnum ) )
	    area_conflict = TRUE;
	}

	if (mob)
	{
	  if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
	    area_conflict = TRUE;
	  if ( ( low_range >= pArea->low_m_vnum )
	  && ( low_range <= pArea->hi_m_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_m_vnum )
	  && ( high_range >= pArea->low_m_vnum ) )
	    area_conflict = TRUE;
	}

	if (obj)
	{
	  if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_o_vnum )
	  && ( low_range <= pArea->hi_o_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_o_vnum )
	  && ( high_range >= pArea->low_o_vnum ) )
	    area_conflict = TRUE;
	}

	if (area_conflict)
	{
	sprintf(buf, "Conflict:%-15s| ",
		(pArea->filename ? pArea->filename : "(invalid)"));
        if(room)
          sprintf( buf2, "Rooms: %5d - %-5d\r\n", pArea->low_r_vnum,
          pArea->hi_r_vnum);
        if(mob)
          sprintf( buf2, "Mobs: %5d - %-5d\r\n", pArea->low_m_vnum,
          pArea->hi_m_vnum);
        if(obj)
          sprintf( buf2, "Objects: %5d - %-5d\r\n", pArea->low_o_vnum,
          pArea->hi_o_vnum);

        strcat( buf, buf2 );
	send_to_char(buf, ch);
    	}
    }

/*
    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range)
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range)
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range)
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\r\n",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }

    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range)
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range)
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range)
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  sprintf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\r\n",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }
*/
    return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

void load_storeitems_rooms() {
	DIR *dp;
	struct dirent *dentry;

	dp = opendir(STOREITEMS_DIR);

	dentry = readdir(dp);

	while ( dentry ) {
		if ( dentry->d_name[0] != '.' )
		{
			char buf[255];
			sprintf(buf, "%s%s", STOREITEMS_DIR, dentry->d_name);
			load_storeitems_room(buf);
		}
		dentry = readdir(dp);
	}
	closedir(dp);
}

/* we take advantage of the fact stored items files are identical
   to buried item files
   */
void load_buried_items() {
	DIR *dp;
	struct dirent *dentry;

        dp = opendir(BURIEDITEMS_DIR);

	dentry = readdir(dp);

	while ( dentry ) {
		if ( dentry->d_name[0] != '.' )
		{
			char buf[255];
                        sprintf(buf, "%s%s", BURIEDITEMS_DIR, dentry->d_name);
			load_storeitems_room(buf);
		}
		dentry = readdir(dp);
	}
	closedir(dp);
}

void load_storeitems_room( const char* filename )
{
    FILE *fp;
	int vnum;
	ROOM_INDEX_DATA* storeroom;
	static  OBJ_DATA *  rgObjNest   [MAX_NEST];

	if ( ( fp = fopen( filename, "r" ) ) != NULL )
	{
	    int iNest;
	    bool found;
	    OBJ_DATA *tobj, *tobj_next;

	    for ( iNest = 0; iNest < MAX_NEST; iNest++ )
		rgObjNest[iNest] = NULL;

	    found = TRUE;
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
			char bugBuf[MAX_STRING_LENGTH];
			sprintf(bugBuf, "Load_storeitems_room: # not found in file %s.", filename);
		    bug( bugBuf, 0 );
		    break;
		}

		word = fread_word( fp );
		if ( !str_cmp(word, "VNUM" ) ) {
			vnum = fread_number(fp);
			storeroom = get_room_index(vnum);
			if ( !storeroom ) {
				bug("load_storeitems_room: Invalid vnum %s", vnum_to_dotted(vnum));
				break;
			}
			rset_supermob(storeroom);
		}
		else if ( !str_cmp( word, "OBJECT" ) ) {	/* Objects	*/
			if ( vnum == -1 ) {
				bug("Load_storeitems_room: OBJECT before vnum!");
				break;
			}
		  fread_obj  ( supermob, fp, OS_CARRY );
		}
		else
		if ( !str_cmp( word, "END"    ) )	/* Done		*/
		  break;
		else
		{
		    bug( "Load_storeitems_room: bad section.", 0 );
		    break;
		}
	    }
	    fclose( fp );
	    for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
	    {
		tobj_next = tobj->next_content;
		obj_from_char( tobj );
		obj_to_room( tobj, storeroom );
	    }
	    release_supermob();
	}
	else
	    log_string( "Cannot open storeitems file" );
}

void count_objs_in_pfile(char* path)
{
	OBJ_INDEX_DATA* pIdx;
	FILE *fp;
	char *buf;
	bool objFound = FALSE;
	int count = 1;

	buf = (char *)calloc(255, 1); // max linelen 255

	if ( !buf ) {
		log_string("count_objs_pfile: Can't allocate a buffer");
		exit(1);
	}

	if ( !(fp = fopen(path, "r")) ) {
		sprintf(buf, "count_objs_pfile: can't open %s", path);
		log_string(buf);
		return;
	}

	while(!feof(fp)) {
		fgets(buf, 255, fp);

		if(!strncmp("#OBJECT", buf, 7) & !objFound) {
			objFound = TRUE;
			count = 1;
		}
		else if(objFound) {
			if(!strncmp("Count", buf, 5))
				count = atoi(buf+13);
			else if(!strncmp("Vnum", buf, 4))
			{
				int vnum = atoi(buf+13);
				pIdx = get_obj_index(vnum);
				if ( ! pIdx ) continue; /* ignore, it happens -- normal */
				pIdx->total_count += count;
			}
			else if(!strncmp("End", buf, 3)) {
				objFound = FALSE;
				count = 1;
			}
		} else {
			if ( !strncmp("Deity", buf, 5) ) {
				DeityData* deity = first_deity;
				char * bufptr;
				char * ptr;
				char deityb[MAX_STRING_LENGTH] = "\0";

				bufptr = buf;
				ptr = deityb;

				bufptr+=5;
				while(isspace(*bufptr))         bufptr++;
				while(*bufptr!='~'  ) *ptr++ = *bufptr++;
				*ptr = '\0';
				for(;deity;deity=deity->next) {
					if ( deity->name_.ciEqual(deityb) )
					{
						deity->worshippers++;
					}
				}
			}
		}
	}
	free(buf);
	fclose(fp);
}

void count_pfile_objs() {
    DIR *dp;
    struct dirent *dentry;
    char ch;

	for ( ch = 'a'; ch <= 'z'; ch++ ) {
		char buf[255];

		sprintf(buf, "../player/%c", ch);

		dp = opendir(buf);

		while ( (dentry = readdir(dp)) ) {
			if ( dentry->d_name[0] < 'A' || dentry->d_name[0] > 'Z' ) {
				continue;
			}

			sprintf(buf, "../player/%c/%s", ch, dentry->d_name);

			count_objs_in_pfile(buf);
		}

		closedir(dp);
	}
}

void add_auctionroom(ROOM_INDEX_DATA *room) {
   AUCTION_ROOM* pAuctionTemp;

   CREATE(pAuctionTemp, AUCTION_ROOM, 1);
   pAuctionTemp->room = room;

   if(!first_auctionroom)
     first_auctionroom = pAuctionTemp;
   else
     last_auctionroom->next = pAuctionTemp;

   last_auctionroom = pAuctionTemp;
   last_auctionroom->next = NULL;
}

