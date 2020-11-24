/****************************************************************************
 *  *     Color Module -- Allow user customizable Colors.        *
 *  *                      Matthew  & Golias                                   *
 *  ****************************************************************************/
 

/*
 *  * To add new color types:
 *  * 
 *  * 1.  Edit mud.h, and:
 *  *     1.  Add a new AT_ define.
 *  *     2.  Increment MAX_PCCOLORS by however AT_'s you added.
 *  * 2.  Edit color.c and:
 *  *     1.  Add the name(s) for your new color(s) to the end of the pc_displays
 *  *         array.
 *  *     2.  Add the default color(s) to the end of the default_set array.
 *  */

#include <stdio.h>
#include "globals.h"
#include "mud.h"
#include "connection.h"
#include "World.h"
#include <stdlib.h>
#include <string.h>

void show_colors  (CHAR_DATA *ch);
void reset_colors (CHAR_DATA *ch);
void color_scheme (CHAR_DATA *ch, const char * argument);

const char * const pc_displays[MAX_PCCOLORS] =
{
	"black",	  "blood",	   "dgreen",	  "orange",   "dblue",
	"purple",	  "cyan",	   "grey",		  "dgrey",	  "red",
	"green",	  "yellow",    "blue",		  "pink",	  "lblue",
	"white",	  "blink",	   "plain", 	  "action",   "say",
	"proclaim",   "yell",	   "tell",		  "wacking",  "getting_wacked",
	"immortal",   "hurting",   "falling",	  "danger",   "magic",
	"consider",   "reporting", "poison",	  "socials",  "dying",
	"dead",       "skills",    "carnage",	  "damage",   "fleeing",
	"room_names", "room_desc", "objects",	  "pcs/mobs", "list",
	"bye",        "gold",      "group_tells", "note", "hungry",
	"thirsty",    "fire",      "sober",	   "wearoff", "exits",
	"score",      "reset",     "log",		  "die_msg", "wartalk",
	"arena",      "muse",      "think", "avtalk", "ramble",
	"pray", "clan", "council", "order", "evtalk", "sgtalk",
	"night"
};

sh_int const default_set [MAX_PCCOLORS] =
{
	AT_BLACK,	  AT_BLOOD,  AT_DGREEN, 	AT_ORANGE,
	AT_DBLUE,	  AT_PURPLE, AT_CYAN,		AT_GREY,
	AT_DGREY,	  AT_RED,	 AT_GREEN,		AT_YELLOW,
	AT_BLUE,	  AT_PINK,	 AT_LBLUE,		AT_WHITE,
	
	AT_RED+AT_BLINK, AT_GREY,	AT_GREY,	AT_ORANGE, /* blink, plain, action, say */
	AT_LBLUE,		 AT_WHITE,	AT_YELLOW,	AT_WHITE, /* chat, yell, tell, hit */
	AT_YELLOW,		 AT_LBLUE,	AT_RED, 	AT_WHITE+AT_BLINK, /* hitme, immort, hurting, falling */
	AT_RED+AT_BLINK, AT_BLUE,	AT_GREY,	AT_GREY, /* danger, magic, consider, reporting */
	AT_GREEN,		 AT_CYAN,	AT_YELLOW,	AT_RED, /* poison, socials, dying, dead */
	AT_GREEN,		 AT_BLOOD,	AT_WHITE,	AT_YELLOW, /* skill, carnage, damage, flee */
	AT_WHITE,		 AT_YELLOW, AT_DGREEN,	AT_PINK, /* rname, rdesc, obj, person */
	AT_BLUE,		 AT_GREEN,	AT_YELLOW,	AT_BLUE, /* list, bye, gold, gtell */
	AT_WHITE,		 AT_ORANGE, AT_BLUE,	AT_RED, /* note, hungry, thirsty, fire */
	AT_WHITE,		 AT_YELLOW, AT_WHITE,	AT_BLUE, /* sober, wearoff, exit, score */
	AT_DGREEN,		 AT_DGREEN, AT_WHITE,	AT_RED, /* reset, log, die, wartalk */
	AT_PURPLE,		 AT_LBLUE,	AT_LBLUE,	AT_PURPLE, /* arena, muse, think, avtalk */
	AT_LBLUE,		 AT_YELLOW, AT_DGREEN,	AT_GREEN, /* ooc, pray, clan, council */
	AT_RED, 		 AT_DGREY,	AT_CYAN,			   /* order, evtalk, sgtalk */
	AT_BLUE 											/* night */
};

const char * const valid_color[] =
{
	"black", 
	"blood", 
	"dgreen", 
	"orange",
	"dblue", 
	"purple", 
	"cyan",
	"grey",
	"dgrey",
	"red", 
	"green", 
	"yellow", 
	"blue", 
	"pink", 
	"lblue", 
	"white",
	"\0"
}; 

void show_colors( CHAR_DATA *ch)
{
	int count;
	
	set_pager_color(AT_PLAIN, ch);
	send_to_pager( "Syntax: color <color type> <color>|default\n\r", ch );
	send_to_pager( "Syntax: color _reset_ (Resets all colors to default set)\n\r", ch );
	send_to_pager( "Syntax: color _all_ <color> (Sets all color types to <color>)\r\n", ch);
	send_to_pager( "Syntax: color _scheme_ <name> (Sets all colors to scheme <name>)\r\n", ch);
	send_to_pager("\r\n", ch);
	
	set_pager_color(AT_WHITE, ch);
	send_to_pager("********************************[ COLORS ]*********************************\r\n", ch);
	
	for ( count = 0; count < 16; ++count ) {
		   if ( (count % 4) == 0 && count != 0) {
			   send_to_pager("\r\n",ch);
		   }
		   set_pager_color(count, ch);
		   pager_printf(ch, "%-15s", pc_displays[count]);
	}
	
	set_pager_color(AT_WHITE, ch);
	send_to_pager("\r\n\r\n******************************[ COLOR TYPES ]******************************\r\n", ch);
	
	for ( count = 16; count < MAX_PCCOLORS; ++count ) {
		if ( (count % 4) == 0 && count != 16) {
			send_to_pager("\r\n",ch);
		}
		set_pager_color(count, ch);
		pager_printf(ch, "%-15s", pc_displays[count]);
	}
	
	set_pager_color(AT_YELLOW, ch);
	send_to_pager("\r\n\r\nAvailable colors are:\r\n", ch);
	
	set_pager_color(AT_PLAIN, ch);
	for ( count = 0; valid_color[count][0] != '\0'; ++count) {
		if ( (count%4) == 0 && count != 0 )
			send_to_pager("\r\n", ch);
		
		pager_printf(ch, "%-10s", valid_color[count]);
	}
	
	return;
}

void do_color(CHAR_DATA *ch, const char* argument)
{
	bool dMatch, cMatch;
	int count, y;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	
	dMatch = FALSE;
	cMatch = FALSE;
	
	if (IS_NPC(ch)) {
		send_to_pager( "Only PC's can change colors.\n\r", ch );
		return;
	}
	
	if ( !argument || argument[0] == '\0' ) {
		show_colors(ch);
		return;
	}
	
	argument = one_argument( argument, arg );
	
	if ( !str_prefix(arg, "_reset_" ) ) {
		reset_colors(ch);
		send_to_pager( "All color types reset to default colors.\n\r", ch );
		return;
	}
	if ( !str_prefix(arg, "_scheme_" )) {
		   color_scheme(ch, argument);
		   return;
	}
	
	argument = one_argument( argument, arg2 );
	
	if ( arg[0] == '\0' )
	{
		send_to_char("Change which color type?\r\n", ch);
		return;
	}
	
	argument = one_argument( argument, arg3 );
	
	if ( !str_prefix(arg, "_all_") )
	{
		dMatch = TRUE;
		count = -1;
		
		/* search for a valid color setting*/
		for ( y = 0; y < 16 ; y++ ) {
			if (!str_cmp(arg2, valid_color[y])) {
				cMatch = TRUE;
				break;
			}
		}
	}
	else if ( arg2[0] == '\0' )
	{
		cMatch = FALSE;
	}
	else
	{
		/* search for the display type and strcmp*/
		for ( count = 0; count < MAX_PCCOLORS ; count++ ) {
			if (!str_prefix (arg, pc_displays[count])) {
				dMatch = TRUE;
				break;
			}
		}
		
		if(!dMatch) {
			send_to_pager( "Invalid Color Type\n\r", ch );
			send_to_pager( "Type color with no arguments to see available options.\n\r", ch );
			return;
		}
		
		if (!str_cmp (arg2, "default")) {
			ch->pcdata->pc_colors[count] = default_set[count];
			sprintf(buf, "Display %s set back to default.\n\r", pc_displays[count] );
			send_to_pager( buf, ch );
			return;
		}
		
		/* search for a valid color setting*/
		for ( y = 0; y < 16 ; y++ ) {
			if (!str_cmp(arg2, valid_color[y])) {
				cMatch = TRUE;
				break;
			}
		}
	}
	
	if(!cMatch)
	{
		if ( arg[0] )
		{
			ch_printf(ch, "Invalid color for type %s.\n", arg);
		}
		else
		{
			send_to_pager( "Invalid color.\n\r", ch );
		}
		
		send_to_pager( "Choices are:\n\r", ch );
		
		for ( count = 0; count < 16; count++ )
		{
			if ( count % 4 == 0 && count != 0 )
				send_to_pager("\r\n", ch);
			
			pager_printf(ch, "%-10s", valid_color[count]);
		}
		
		pager_printf(ch, "%-10s\r\n", "default");
		return;
	}
	else
	{
		sprintf(buf, "Color type %s set to color %s.\n\r",count==-1?"_all_":pc_displays[count], valid_color[y] );
	}
	
	if (!str_cmp (arg3, "blink"))
	{
		y += AT_BLINK;
	}
	
	if ( count == -1 )
	{
		int count;
		
		for ( count = 0; count < MAX_PCCOLORS; ++count )
		{
			ch->pcdata->pc_colors[count] = y;
		}
		
		set_pager_color(0, ch);
		
		sprintf(buf, "All color types set to color %s%s.\r\n",
			valid_color[y>AT_BLINK?y-AT_BLINK:y], y>AT_BLINK ? " [BLINKING]" : "");
		
		send_to_pager(buf, ch);
	}
	else
	{
		ch->pcdata->pc_colors[count] = y;
		
		set_pager_color( count , ch);
		
		if (!str_cmp (arg3, "blink"))
			sprintf(buf, "Display %s set to color %s [BLINKING]\n\r", pc_displays[count], valid_color[y-AT_BLINK] );
		else
			sprintf(buf, "Display %s set to color %s.\n\r", pc_displays[count], valid_color[y] );
		
		send_to_pager( buf, ch );
	}
	set_pager_color( AT_PLAIN , ch);
	
	return;
}


void reset_colors(CHAR_DATA *ch)
{
      memcpy(&ch->pcdata->pc_colors, &default_set, sizeof(default_set));
}

/* copied from set_char_color */
const char* get_char_color( sh_int AType, CHAR_DATA *ch )
{
	static char buf[16];
	CHAR_DATA *och;
	
	sh_int ccolor;
	
	if ( !ch || !ch->GetConnection() )
		return "";
	
	och = ch->GetConnection()->GetOriginalCharacter();
	if (IS_SET(och->act, PLR_ANSI) )
	{
		och->pcdata->last_color = AType;
		ccolor = AType;   
		if (!IS_NPC(och))
		{
			AType = och->pcdata->pc_colors[ccolor];
		}
		
		
		if ( AType == 7 )
			strcpy( buf, "\033[m" );
		else
		{
			bool bBlink = AType >= AT_BLINK;
			
			if ( AType >= AT_BLINK )
				AType -= AT_BLINK;
			sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
				(bBlink ? "5;" : ""), (AType & 7)+30);
		}
		return buf;
	}
	return "";
}

/* Moved from comm.c */
void set_char_color( sh_int AType, CHAR_DATA *ch )
{
	char buf[16];
	CHAR_DATA *och;
	
	sh_int ccolor;
	
	if ( !ch || !ch->GetConnection() )
		return;
	
	och = ch->GetConnection()->GetOriginalCharacter();
	if (IS_SET(och->act, PLR_ANSI) )
	{
		och->pcdata->last_color = AType;
		ccolor = AType;   
		if (!IS_NPC(och))
		{
			AType = och->pcdata->pc_colors[ccolor];
		}
		
		
		if ( AType == 7 )
			strcpy( buf, "\033[m" );
		else
		{
			bool bBlink = false;
			if ( AType >= AT_BLINK )
			{
				bBlink = true;
				AType -= AT_BLINK;
			}

			// Specifying attributes:
			// <ESC>[{attr1};...;{attrn}m
			// 0: reset all attributes

			// (AType & 8) == 8 --> if AType is >= 8 (i.e. dgrey)
			// this will return 1 - which sets the bright bit.

			// AType & 7, + 30 will convert the color code to
			// its ANSI equivalent

			sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8,
				(bBlink ? "5;" : ""), (AType & 7)+30);
		}
		ch->GetConnection()->SendText(buf);
	}
	return;
}

// Ksilyan: take a color number,
// and translate it to its &c &Y etc.
// equivalent.

// This is for using in ostreams,
// that are sent to the player,
// and when it is sent to the connection
// the color is removed if the player
// is not using color.

/*char ColorNumberToCharacter(char clr)
{
	static const char colors[17] = "xrgObpcwzRGYBPCW";
	int r;
	
	for ( r = 0; r < 16; r++ )
		if ( clr == colors[r] )
			return r;
	return -1;
}*/

const char * ColorNumberToChars( short color )
{
	static char buf[4]; // one for &, one for letter, one for null, one extra
	
	if ( color >= AT_BLINK || color < 0)
	{
		gTheWorld->LogBugString("ColorNumberToChars called with invalid color!");
		return "";
	}

	static const char colors[17] = "xrgObpcwzRGYBPCW";

	strcpy(buf, "");

	for (int i = 0; i < AT_BLINK; i++)
	{
		if (color == i)
		{
			sprintf(buf, "&%c", colors[i]);
			break;
		}
	}
	
	return buf;
}



void set_pager_color( sh_int AType, CHAR_DATA *ch )
{
	char buf[16];
	CHAR_DATA *och;
	
	int ccolor;
	
	if ( !ch || !ch->GetConnection() )
		return;
	
	och = ch->GetConnection()->GetOriginalCharacter();
	if ( IS_SET(och->act, PLR_ANSI) )
	{
		ccolor = AType;
		if(!IS_NPC(och) )
		{
			if (ccolor >= MAX_PCCOLORS)
				return;
			AType = och->pcdata->pc_colors[ccolor];
		}
		
		if ( AType == 7 )
			strcpy( buf, "\033[m" );
		else
		{
			bool bBlink = false;
			
			if ( AType >= AT_BLINK )
			{
				bBlink = true;
				AType -= AT_BLINK;
			}
			
			sprintf(buf, "\033[0;%d;%s%dm", (AType & 8) == 8, (bBlink ? "5;" : ""), (AType & 7)+30);
		}
		
		send_to_pager( buf, ch );
		ch->GetConnection()->pagecolor = AType;
	}
	return;
}

#define MAX_SCHEMES 2

const char * const scheme_names [MAX_SCHEMES][2] =
{
     {"smaug_default", "The default color scheme for Smaug MUDs"},
     {"warp_1", "Warp's Colors"}
};

sh_int const schemes [MAX_SCHEMES][MAX_PCCOLORS] =
{
     {
	AT_BLACK,     AT_BLOOD,  AT_DGREEN,     AT_ORANGE,
	  AT_DBLUE,     AT_PURPLE, AT_CYAN,       AT_GREY,
	  AT_DGREY,     AT_RED,    AT_GREEN,      AT_YELLOW,
	  AT_BLUE,      AT_PINK,   AT_LBLUE,      AT_WHITE,
	  
	  AT_RED+AT_BLINK, AT_GREY,   AT_GREY,    AT_ORANGE, /* blink, plain, action, say */
	  AT_LBLUE,        AT_WHITE,  AT_YELLOW,  AT_WHITE, /* chat, yell, tell, hit */
	  AT_YELLOW,       AT_LBLUE,  AT_RED,     AT_WHITE+AT_BLINK, /* hitme, immort, hurting, falling */
	  AT_RED+AT_BLINK, AT_BLUE,   AT_GREY,    AT_GREY, /* danger, magic, consider, reporting */
	  AT_GREEN,        AT_CYAN,   AT_YELLOW,  AT_RED, /* poison, socials, dying, dead */
	  AT_GREEN,        AT_BLOOD,  AT_WHITE,   AT_YELLOW, /* skill, carnage, damage, flee */
	  AT_WHITE,        AT_YELLOW, AT_DGREEN,  AT_PINK, /* rname, rdesc, obj, person */
	  AT_BLUE,         AT_GREEN,  AT_YELLOW,  AT_BLUE, /* list, bye, gold, gtell */
	  AT_WHITE,        AT_ORANGE, AT_BLUE,    AT_RED, /* note, hungry, thirsty, fire */
	  AT_WHITE,        AT_YELLOW, AT_WHITE,   AT_BLUE, /* sober, wearoff, exit, score */
	  AT_DGREEN,       AT_DGREEN, AT_WHITE,   AT_RED, /* reset, log, die, wartalk */
	  AT_PURPLE,       AT_LBLUE,  AT_LBLUE,   AT_PURPLE, /* arena, muse, think, avtalk */
      AT_LBLUE,        AT_LBLUE,  AT_LBLUE,   AT_LBLUE,   /* ooc, pray, clan, council */
         AT_DGREY, AT_CYAN
     },
     {
         AT_BLACK,       AT_BLOOD,       AT_DGREEN,      AT_ORANGE,
        AT_DBLUE,       AT_PURPLE,      AT_CYAN,        AT_GREY,
        AT_DGREY,       AT_RED,         AT_GREEN,       AT_YELLOW,
        AT_BLUE,        AT_PINK,        AT_LBLUE,       AT_WHITE,

        AT_RED+AT_BLINK, AT_GREY,       AT_GREY,        AT_CYAN,
        AT_DGREEN,      AT_RED,         AT_WHITE,       AT_GREEN,
        AT_RED,         AT_YELLOW,      AT_PINK,        AT_PINK+AT_BLINK,
        AT_RED+AT_BLINK, AT_LBLUE,      AT_GREY,        AT_GREY,
        AT_PINK,        AT_GREY,        AT_RED,         AT_BLOOD,
        AT_GREY,        AT_BLOOD,       AT_RED,         AT_PURPLE,
        AT_WHITE,       AT_GREY,        AT_DGREEN,      AT_GREY,
        AT_GREY,        AT_GREEN,       AT_YELLOW,      AT_DGREEN,
        AT_GREY,        AT_ORANGE,      AT_BLUE,        AT_RED,
        AT_BLUE,        AT_LBLUE,       AT_GREY,        AT_GREY,
        AT_DBLUE,        AT_DGREY,      AT_BLOOD+AT_BLINK, AT_RED,
        AT_PURPLE,      AT_GREEN,       AT_LBLUE,       AT_DGREEN,
        AT_LBLUE,       AT_LBLUE,       AT_LBLUE,       AT_GREY,
        AT_DGREY,       AT_CYAN
     }
};

void color_scheme(CHAR_DATA* ch, const char* argument)
{
      char sname[MAX_INPUT_LENGTH];
      int i;
      
      if ( !argument || argument[0] == '\0' )
     {
	send_to_char("Usage: color _scheme_ <name>\r\n\r\n", ch);
	
	send_to_char("Where <name> is one of the following:\r\n", ch);
	
	for ( i = 0; i < MAX_SCHEMES; ++i )
	  {
	          ch_printf(ch, "%-20s %s\r\n", scheme_names[i][0], scheme_names[i][1]);
	  }
	return;
     }
   
      argument = one_argument(argument, sname);
      
      for ( i = 0; i < MAX_SCHEMES; ++i )
     {
	if ( !str_prefix(scheme_names[i][0], sname) )
	    break;
     }
      
      if ( i >= MAX_SCHEMES )
     {
	send_to_char("That scheme was not found!\r\n", ch);
	return;
     }
      
      memcpy(&ch->pcdata->pc_colors, &schemes[i], sizeof(schemes[i]));
      ch_printf(ch, "All colors set to those in the scheme %s.\r\n", scheme_names[i][0]);
}
