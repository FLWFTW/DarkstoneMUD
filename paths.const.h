


#ifndef __PATHS_H_
#define __PATHS_H_

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define ACCOUNTS_DIR "../accounts/"
#define COPYOVER_DIR "../copyover/"
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define BACKUP_DIR	"../backup/"    /* Backup Player files		*/
#define GOD_DIR		"../gods/"	/* God Info Dir			*/
#define BOARD_DIR	"../boards/"	/* Board data dir		*/
#define CLAN_DIR	"../clans/"	/* Clan data dir		*/
#define STOREITEMS_DIR "../storeditems/"
#define BURIEDITEMS_DIR "../burieditems/"
#define COUNCIL_DIR  	"../councils/"  /* Council data dir		*/
#define GUILD_DIR       "../guilds/"    /* Guild data dir               */
#define DEITY_DIR	"../deity/"	/* Deity data dir		*/
#define BUILD_DIR       "../building/"  /* Online building save dir     */
#define SYSTEM_DIR	"../system/"	/* Main system files		*/
#define PROG_DIR	"mudprogs/"	/* MUDProg files		*/
#define SKRYPT_DIR  "skrypt/" // Skrypt files
#define CORPSE_DIR	"../corpses/"	/* Corpses			*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define	CLASS_DIR	"../classes/"	/* Classes			*/

#define AREA_LIST				"area.lst"	/* List of areas		*/
#define ACCOUNT_BAN_LIST 		"account.ban.lst"
#define BAN_LIST        		"ban.lst"       /* List of bans                 */
#define CLAN_LIST				"clan.lst"	/* List of clans		*/
#define COUNCIL_LIST			"council.lst"	/* List of councils		*/
#define GUILD_LIST      		"guild.lst"     /* List of guilds               */
#define GOD_LIST				"gods.lst"	/* List of gods			*/
#define DEITY_LIST				"deity.lst"	/* List of deities		*/
#define	CLASS_LIST				"class.lst"	/* List of classes		*/

#define BOARD_FILE				"boards.txt"		/* For bulletin boards	 */
#define SHUTDOWN_FILE			"shutdown.txt"		/* For 'shutdown'	 */

#define RIPSCREEN_FILE			SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE			SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE			SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE			SYSTEM_DIR "mudtitle.asc"
#define BOOTLOG_FILE			SYSTEM_DIR "boot.txt"	  	/* Boot up error file	 */
#define BUG_FILE				SYSTEM_DIR "bugs.txt"	  	/* For bug( )*/

/* Added PBUG_FILE for do_bugs - Zoie */
#define PBUG_FILE				SYSTEM_DIR "pbugs.txt"  	/* For 'bug' command */
#define IDEA_FILE				SYSTEM_DIR "ideas.txt"	  	/* For 'idea'		 */
#define TYPO_FILE				SYSTEM_DIR "typos.txt"	  	/* For 'typo'		 */

/* These aren't used currently, future planned recent players online list - Zoie */
#define LAST_LIST       		SYSTEM_DIR "last.lst" 		/* For 'last'        */
#define LAST_TEMP_LIST  		SYSTEM_DIR "ltemp.lst" 		/* Temp file for the last list so the data can be copyover over */
#define LOG_FILE				SYSTEM_DIR "log.txt"	  	/* For talking in logged rooms */
#define WIZLIST_FILE			SYSTEM_DIR "WIZLIST"	  	/* Wizlist		 */
#define WHO_FILE				SYSTEM_DIR "WHO"	  		/* Who output file	 */
#define WEBWHO_FILE				SYSTEM_DIR "WEBWHO"	  		/* WWW Who output file */
#define REQUEST_PIPE			"../REQUESTS"	  			/* Request FIFO	 */
#define SKILL_FILE				SYSTEM_DIR "skills.dat"   	/* Skill table	 */
#define HERB_FILE				SYSTEM_DIR "herbs.dat"	  	/* Herb table		 */
#define SOCIAL_FILE				SYSTEM_DIR "socials.dat"  	/* Socials		 */
#define COMMAND_FILE			SYSTEM_DIR "commands.dat" 	/* Commands		 */
#define USAGE_FILE				SYSTEM_DIR "usage.txt"    	/* How many people are on every half hour - trying to determine best reboot time */
#define COPYOVER_FILE			SYSTEM_DIR "copyover.dat" 	/* for warm reboots	 */
#define EXE_FILE 				"/home/darkstone/newmud/src/dark"		  // executable path
#define CLASSDIR				"../classes/"

#define NEW_ACCOUNT_FILE       	SYSTEM_DIR "new_account.txt"
#define ACCOUNT_REJECTED_FILE	SYSTEM_DIR "account_rejected.txt"
#define NEW_PASSWORD_FILE      	SYSTEM_DIR "new_password.txt"
#define AUTO_REJECT_FILE       	SYSTEM_DIR "account_waiting_for_auth.txt"

// Ksilyan:
#define ARENA_RANK_FILE SYSTEM_DIR "arenarank.dat"
#define MOVEMENT_MESSAGE_FILE SYSTEM_DIR "movementmessages.dat"
#define QUESTS_FILE SYSTEM_DIR "quests.lua"
#define LUA_DIR "../lua/"

// Home directory
#define HOME_DIR "/home/darkstone/mud/"


#endif

