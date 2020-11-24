/* new do_last, a bit like last in unix */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mud.h"

#include "paths.const.h"

#define  DIR_NAME PLAYER_DIR	/* mud home player directory here! */
#define  MAX_NAME_LENGTH 13
#define  MAX_SITE_LENGTH 16
#define  LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))

static time_t	now_time;

typedef struct last_data LAST_DATA;

struct last_data {
	char name [MAX_NAME_LENGTH];
	short trust;
	char site [MAX_SITE_LENGTH];
	long juliantime;
	char lasttime [MAX_STRING_LENGTH];
	LAST_DATA *next;
};

//typedef struct last_data LAST_DATA;
char *get_arg( char *argument, char *arg_first )
{
    int count=0;

    while ( isspace(*argument) ) argument++;
    while ( *argument != '\0' && *argument != 10 && ++count <= 255 )
    {
       if ( *argument == ' ' ) {argument++; break;}
       *arg_first = LOWER(*argument);
       arg_first++;
       argument++;
    }
    *arg_first = '\0';
    while ( isspace(*argument) ) argument++;
    return argument;
}
LAST_DATA *bubble(LAST_DATA *first) {
	/* A very kludgy sorting algorhithm for one-way linked lists.
	 * Creates a new, sorted list and returns pointer to that.
	 * Warp
	 */

	LAST_DATA *a = NULL;
	LAST_DATA *b = NULL;
	LAST_DATA *cur = NULL;
	LAST_DATA *tmp = NULL;
	long smallest, count=0;

	if ( first == NULL )
		return NULL;

	a = first;
	cur = first;

	while(a) {
		a = a->next;
		count++;
	}

	CREATE(a, LAST_DATA, 1);

	strcpy(a->name, first->name);
	a->trust = first->trust;

	strcpy(a->site, first->site);
	a->juliantime = first->juliantime;

	strcpy(a->lasttime, first->lasttime);
	a->next = NULL;

	smallest = 0;

	while(count) {
		while(cur) {
			if(cur->juliantime < a->juliantime && cur->juliantime > smallest) {

				strcpy(a->name, cur->name);
				a->trust = cur->trust;

				strcpy(a->site, cur->site);
				a->juliantime = cur->juliantime;

				strcpy(a->lasttime, cur->lasttime);
			}
			cur = cur->next;
		}
		if(!b)
			b = a;
		CREATE(tmp, LAST_DATA, 1);
		smallest = a->juliantime;
		tmp->juliantime = 999999999;
		tmp->next = NULL;
		a->next = tmp;
		a = tmp;
		count--;
		cur = first;
	}
	return b;
}

void last_read_pfile (char *dirname, char *filename, LAST_DATA *r) {
	FILE *fp;
	char b[MAX_STRING_LENGTH], s[MAX_STRING_LENGTH], *ps;
	char fname[MAX_STRING_LENGTH];
	struct stat fst;
	
	sprintf( fname, "%s/%s", dirname, filename );

	if ( stat( fname, &fst ) != -1 ) {
		r->juliantime = fst.st_mtime;

		strcpy(r->lasttime, ctime(&fst.st_mtime));
	        r->lasttime[strlen(r->lasttime)-1] = '\0';
	}
	else
	  r->juliantime = 0;
		
	if ( ( fp = fopen( fname, "r" ) ) == NULL )
     return;

	fgets(s, 2048, fp);
	
	while (!feof(fp)) {
		if (ferror(fp)) {
			bug("do_last::file error reading %s\n",fname);
			break;
		}

		ps = s;
		
		/* a veeeeeery kludgy way to search for the right field in the pfile,
		 * but this is how it is done in grux.c. Maybe I'll change it sometime.
		 * -- Warp
		 */
		
	   if ( s[0]=='N' && s[1]=='a' && s[2]=='m' && s[3]=='e' ) {
	      ps = get_arg (ps, b); ps = get_arg (ps, b);
	      if ( b[ strlen(b) - 1] == '~' )
		b[ strlen(b) - 1] = '\0';

	      strcpy (r->name, b);
	   }

		else if ( s[0]=='T' && s[1]=='r' && s[2]=='u' && s[3]=='s' && s[4]=='t' ) {
			ps = get_arg (ps, b); ps = get_arg (ps, b);
			r->trust = atoi(b);
		}
		
		else if ( s[0]=='S' && s[1]=='i' && s[2]=='t' && s[3]=='e' ) {
			ps = get_arg (ps, b); ps = get_arg (ps, b);
			if ( !isdigit(b[0]) )
		     {

			strcpy(r->site, "(unknown)");
		     }
			else
		     {

           strcpy(r->site, b);
		     }
			break;
		}
     fgets(s, 2048, fp);
	}
  fclose (fp);
}
	
void do_last(CHAR_DATA *ch, const char* argument) {
	char buf [MAX_STRING_LENGTH];
	char arg [MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];

	struct stat fst;
	DIR *dp;

	struct dirent *dentry;
	char dir_name[100];
	int alpha_loop;

	LAST_DATA *cur_last = NULL;
	LAST_DATA *first_last = NULL;
	LAST_DATA *temp = NULL;

	LAST_DATA last_file;

	last_file.trust = 0;

	CREATE(cur_last, LAST_DATA, 1);

	now_time = time(0);
	nice(20);

	one_argument( argument, arg );
	if ( arg[0] == '\0' ) {
		/* KSILYAN
		   This seems to be ... not so useful and really lags up the MUD.
		   Don't want it to be used...
		 */
		return;
		for (alpha_loop=0; alpha_loop<=25; alpha_loop++)
		{
			sprintf (dir_name, "%s%c", DIR_NAME, 'a' + alpha_loop);
			dp = opendir( dir_name );
			dentry = readdir( dp );

			while ( dentry )
			{
				if ( dentry->d_name[0] != '.' )
				{
					last_read_pfile (dir_name, dentry->d_name, cur_last);
					if( cur_last->trust < 50 || get_trust(ch) <= 62 ||
							(cur_last->trust <= get_trust(ch) ) )
					{
						cur_last->next = first_last;
						first_last = cur_last;
						CREATE(cur_last, LAST_DATA, 1);
					}
				}
				dentry = readdir( dp );
			}
			closedir( dp );
		}
		DISPOSE(cur_last);

		/* Now we have everyone's name, trust, site and last times in
		 * a linked list. Now the info needs to be sorted and sent to
		 * the char.
		 */

		cur_last = bubble(first_last);

		/* Sorted (hopefully). Since bubble() made a new list, lets dispose
		 * the old one.
		 */

		while(first_last) {
			temp = first_last->next;
			DISPOSE(first_last);
			first_last = temp;
		}

		/* The output part. Only immortals see the site. */
		set_pager_color(AT_GREEN, ch);

		if(IS_IMMORTAL(ch)) 
			send_to_pager("Name        Last seen                  From\n\r",ch);
		else
			send_to_pager("Name        Last seen\n\r",ch);

		set_pager_color(AT_GREY, ch);

		first_last = cur_last;

		while(first_last) {
			sprintf(buf, "%-11s %-24s   %-20s\n\r",first_last->name,first_last->lasttime,
					IS_IMMORTAL(ch)?first_last->site:"\0");
			send_to_pager(buf, ch);
			temp = first_last->next;

			DISPOSE(first_last);
			first_last = temp;
		}

	}

	else {
		strcpy( name, capitalize(arg) );
#if defined( MACINTOSH )
		sprintf( buf, "%s%c:%s", PLAYER_DIR, tolower(arg[0]), name );
#else
		sprintf(buf, "%s%c", PLAYER_DIR, tolower(arg[0]) );
		last_read_pfile(buf, name, &last_file);
		sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name );
#endif
		if (( get_trust(ch) < LEVEL_IMMORTAL  && last_file.trust >= LEVEL_IMMORTAL ) )
		{
			sprintf( buf, "You cannot view %s's last connection.\n\r", name);
		}
		else
		{
			if ( stat( buf, &fst ) != -1 )
				sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
			else
				sprintf( buf, "%s was not found.\n\r", name );
		}
		send_to_char( buf, ch );
	}
}
