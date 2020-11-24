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
 * Comments: 'notes' attached to players to keep track of outstanding       * 
 *           and problem players.  -haus 6/25/1995                          * 
 ****************************************************************************/

/*  INSTALLATION: 1) Add following stuff to mud.h
 *
 *  NOTE_DATA *      comments;   (* inside char_data definition *)
 *  DECLARE_DO_FUN( do_comment      );
 *
 *                2) Add do_comment to interp.c, as usual
 *
 *                3) in save.c the following must be added
 *
 * save.c:107:     if ( ch->comments )                                              (* saves comments *)
 * save.c:108:       fwrite_comments( ch, fp );                                           (*  in file *)
 *
 *
 * save.c:411:    ch->comments                        = NULL;                   (* nulls ch->comments *)
 *
 *
 * save.c:457:         else if ( !str_cmp( word, "COMMENT") ) fread_comment(ch, fp ); (*snags #COMMENT*)
 *
 *                4) That looks like it to these eyes.  Lemme know if i forgot anything.
 *
 *
 * EXAMPLE:  See end of this file.
 *
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "connection.h"

void note_attach(CHAR_DATA *ch);

void comment_remove( CHAR_DATA *ch, ACCOUNT_DATA *pact, NoteData *pnote )
{
    if ( !pact->comments )
    {
      bug( "comment remove: null board", 0 );
      return;
    }

    if ( !pnote )
    {
      bug( "comment remove: null pnote", 0 );
      return;
    }
    
    /*
     * Remove comment from linked list.
     */
    if ( !pnote->prev )
      pact->comments	= pnote->next;
    else
      pnote->prev->next = pnote->next;

    STRFREE( pnote->text    );
    STRFREE( pnote->subject );
    STRFREE( pnote->to_list );
    STRFREE( pnote->date    );
	delete pnote;

    /*
     * Rewrite entire list.
     */
    write_account_data(pact);

    return;
}

void do_comment(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    NoteData  *pnote;
    ACCOUNT_DATA * pact;
    int vnum;
    int anum;

    if ( IS_NPC(ch) )
    {
	send_to_char("Mobs can't use the comment command.\n\r", ch);
	return;
    }

    if ( !ch->GetConnection() )
    {
	bug( "do_comment: no descriptor", 0 );
	return;
    }

    /* Put in to prevent crashing when someone issues a comment command
       from within the editor. -Narn */
    if ( ch->GetConnection()->ConnectedState == CON_EDITING )
    {
	send_to_char("You can't use the comment command from within the editor.\n\r", ch);
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_WRITING_NOTE:
	  if ( !ch->pnote )
	  {
	     bug( "do_comment: note got lost?", 0 );
	     send_to_char( "Your note got lost!\n\r", ch );
	     stop_editing(ch);
	     return;
	  }
	  if ( ch->dest_buf != ch->pnote )
	     bug( "do_comment: sub_writing_note: ch->dest_buf != ch->pnote", 0 );
	  STRFREE( ch->pnote->text );
	  ch->pnote->text = copy_buffer( ch );
	  stop_editing( ch );	   
	  return;
    }

    set_char_color( AT_NOTE, ch );
    argument = one_argument( argument, arg );
    argument = smash_tilde_static( argument );


    if ( !str_cmp( arg, "list" ) )
    {
        pact = get_account(ch, argument);
	
        if (!pact)
        {
            send_to_char("No such account/player!\n\r", ch);   /* maybe fix this? */
            return;
        }

/*        if ( get_trust(victim) >= get_trust( ch ) )
        {
            send_to_char( "You're not of the right caliber to do this...\n\r", ch );
            return;
        } TODO: NOT VALID WITH ACCOUNTS */

        if ( !pact->comments )
        {
            send_to_char( "There are no relevant comments.\n\r", ch );
            free_account_data(pact);
            return;
        }

        vnum = 0;
        for ( pnote = pact->comments; pnote; pnote = pnote->next )
        {
            vnum++;
            sprintf( buf, "%2d) %-10s [%s] %s\n\r",
            vnum,
            pnote->sender_.c_str(),
            pnote->date, 
            pnote->subject );

            /* Brittany added date to comment list and whois with above change */
            send_to_char( buf, ch );
        }

        free_account_data(pact);
        return;
    }

    if ( !str_cmp( arg, "read" ) )
    {
        bool fAll;

        argument = one_argument( argument, arg1 );
        
        pact = get_account(ch, arg1);

        if (!pact)
        {
            send_to_char("No such player/account!\n\r", ch);   /* maybe fix this? */
            return;
        }


/*      if ( get_trust(victim) >= get_trust( ch ) )
        {
            send_to_char( "You're not of the right caliber to do this...\n\r", ch );
            return;
        }*/

        if ( !pact->comments )
        {
            send_to_char( "There are no relevant comments.\n\r", ch );
            free_account_data(pact);
            return;
        }



        if ( !str_cmp( argument, "all" ) )
        {
            fAll = TRUE;
            anum = 0;
        } else if ( is_number( argument ) ) {
            fAll = FALSE;
            anum = atoi( argument );
        } else {
            send_to_char( "Comment read which number?\n\r", ch );
            free_account_data(pact);
            return;
        }

        vnum = 0;

        for ( pnote = pact->comments; pnote; pnote = pnote->next )
        {
            vnum++;
            if ( vnum == anum || fAll )
            {
                sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                    vnum,
                    pnote->sender_.c_str(),
                    pnote->subject,
                    pnote->date,
                    pnote->to_list);
                send_to_char( buf, ch );
                send_to_char( pnote->text, ch );
                free_account_data(pact);
                return;
            }
        }

        send_to_char( "No such comment.\n\r", ch );
        free_account_data(pact);
        return;
    }

    if ( !str_cmp( arg, "write" ) )
    {
        note_attach( ch );
        ch->substate = SUB_WRITING_NOTE;
        ch->dest_buf = ch->pnote;
        start_editing( ch, ch->pnote->text );
        return;
    }

    if ( !str_cmp( arg, "subject" ) )
    {
        note_attach( ch );
        STRFREE( ch->pnote->subject );
        ch->pnote->subject = STRALLOC( argument );
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "to" ) )
    {
        note_attach( ch );
        STRFREE( ch->pnote->to_list );
        ch->pnote->to_list = STRALLOC( argument );
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "clear" ) )
    {
        if ( ch->pnote )
        {
            STRFREE( ch->pnote->text );
            STRFREE( ch->pnote->subject );
            STRFREE( ch->pnote->to_list );
            STRFREE( ch->pnote->date );
			delete pnote;
        }
        ch->pnote = NULL;

        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "show" ) )
    {
        if ( !ch->pnote )
        {
            send_to_char( "You have no comment in progress.\n\r", ch );
            return;
        }

        sprintf( buf, "%s: %s\n\rTo: %s\n\r",
            ch->pnote->sender_.c_str(),
            ch->pnote->subject,
            ch->pnote->to_list);
        send_to_char( buf, ch );
        send_to_char( ch->pnote->text, ch );
        return;
    }

    if ( !str_cmp( arg, "post" ) )
    {
        char *strtime;

        if ( !ch->pnote )
        {
            send_to_char( "You have no comment in progress.\n\r", ch );
            return;
        }

        argument = one_argument(argument, arg1);
        pact = get_account(ch, arg1);

        if (!pact)
        {
            send_to_char("No such player/account.\n\r", ch);   /* maybe fix this? */
            return;
        }

/*      if (  get_trust(victim) > get_trust( ch ) )
        {
            send_to_char( "You're not of the right caliber to do this...\n\r", ch );
            return;
        }*/

        strtime				= ctime( &secCurrentTime );
        strtime[strlen(strtime)-1]	= '\0';
        ch->pnote->date			= STRALLOC( strtime );

        pnote		= ch->pnote;
        ch->pnote	= NULL;


        /* LIFO to make life easier */
        pnote->next = pact->comments;
        if (pact->comments)
            pact->comments->prev = pnote;
        pnote->prev = NULL;
        pact->comments = pnote;

        write_account_data(pact);
        free_account_data(pact);

        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "remove" ) )
    {
        argument = one_argument(argument, arg1);
        pact = get_account(ch, arg1);

        if (!pact)
        {
            send_to_char("No such player/account.\n\r", ch);   /* maybe fix this? */
            return;
        }

/*      if (  (get_trust(victim) >= get_trust( ch ) )
            || ( get_trust( ch ) < LEVEL_STONE_ADEPT                ) )   
        {
            send_to_char( "You're not of the right caliber to do this...\n\r", ch );
            return;
        }*/

        if ( !is_number( argument ) )
        {
            send_to_char( "Comment remove which number?\n\r", ch );
            free_account_data(pact);
            return;
        }

        anum = atoi( argument );
        vnum = 0;
        for ( pnote = pact->comments; pnote; pnote = pnote->next )
        {
            vnum++;
            if ( ( LEVEL_STONE_ADEPT <= get_trust( ch ) )    /* switch to some LEVEL_ thingie */
            &&   ( vnum == anum ) )
            {
                comment_remove( ch, pact, pnote );
                write_account_data(pact);
                free_account_data(pact);
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }

        send_to_char( "No such comment.\n\r", ch );
        return;
    }

    send_to_char( "Huh?  Type 'help comment' for usage (i hope!).\n\r", ch );
    return;
}


void fwrite_comments( ACCOUNT_DATA *pact, FILE *fp )
{
	NoteData *pnote;

	if(!pact->comments)
		return;

	for(pnote=pact->comments;pnote;pnote=pnote->next)
	{
		fprintf( fp,"#COMMENT\n" );
		fprintf( fp,"sender	%s~\n",pnote->sender_.c_str());
		fprintf( fp,"date  	%s~\n",pnote->date);
		fprintf( fp,"to     	%s~\n",pnote->to_list);
		fprintf( fp,"subject	%s~\n",pnote->subject);
		fprintf( fp,"text\n%s~\n",pnote->text);
	}
	return;
}

void fread_comment( ACCOUNT_DATA* pact, FILE *fp )
{
	NoteData *pnote;

	for ( ; ; )
	{
		char letter;

		do
		{
			letter = getc( fp );
			if ( feof(fp) )
			{
				fclose( fp );
				return;
			}
		}
		while ( isspace(letter) );
		ungetc( letter, fp );

		pnote = new NoteData();

		if ( str_cmp( fread_word( fp ), "sender" ) )
		{
			delete pnote;
			break;
		}
		pnote->sender_	= fread_string_nohash( fp );

		if ( str_cmp( fread_word( fp ), "date" ) )
		{
			delete pnote;
			break;
		}
		pnote->date	= fread_string( fp );

		if ( str_cmp( fread_word( fp ), "to" ) )
		{
			STRFREE( pnote->date );
			delete pnote;
			break;
		}
		pnote->to_list	= fread_string( fp );

		if ( str_cmp( fread_word( fp ), "subject" ) )
		{
			STRFREE( pnote->date );
			STRFREE( pnote->to_list );
			delete pnote;
			break;
		}
		pnote->subject	= fread_string( fp );

		if ( str_cmp( fread_word( fp ), "text" ) )
		{
			STRFREE( pnote->date );
			STRFREE( pnote->to_list );
			STRFREE( pnote->subject );
			delete pnote;
			break;
		}
		pnote->text	= fread_string( fp );

		pnote->next		= pact->comments;
		pnote->prev		= NULL;
		pact->comments		= pnote;
		return;
	}

	bug( "fread_comment: bad key word. strap in!", 0 );
	/* exit( 1 ); */
}




/*
<758hp 100m 690mv> <#10316> loadup boo
Log: Haus: loadup boo
Log: Reading in player data for: Boo
Done.
<758hp 100m 690mv> <#10316> poke boo
You poke him in the ribs.
<758hp 100m 690mv> <#10316> comment subject boo's a nutcase!
Ok.
<758hp 100m 690mv> <#10316> comment to all
Ok.
<758hp 100m 690mv> <#10316> comment write
Begin entering your text now (/? = help /s = save /c = clear /l = list)
-----------------------------------------------------------------------
> He transed shimmy to temple square!
> /s
Done.
<758hp 100m 690mv> <#10316> comment post boo
Ok.
<758hp 100m 690mv> <#10316> comment list boo
 1) Haus: boo's a nutcase!
<758hp 100m 690mv> <#10316> comment read boo 1
[  1] Haus: boo's a nutcase!
Sun Jun 25 18:26:54 1995
To: all
He transed shimmy to temple square!
<758hp 100m 690mv> <#10316> comment remove boo 1
Ok.
<758hp 100m 690mv> <#10316> comment list boo
There are no relevent comments.
*/
