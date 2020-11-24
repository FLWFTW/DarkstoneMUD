/* Account Code for Smaug */

#include <stdlib.h>
#include <string.h>

#ifdef unix
	#include <crypt.h>
#endif

#include "character.h"
#include "connection.h"
#include "connection_manager.h"
#include "paths.const.h"

#include "java_iterator.hpp"

#include <sstream>
#include <iterator>

const char * const    account_flags [] =
{
    "banned", "canmplay", "immortal", "logged", "noautoallow", "waiting", "notify", "canwho"

};


void    load_account_banlist (void);

ACCOUNT_BAN_DATA* first_account_ban;
ACCOUNT_BAN_DATA* last_account_ban;

unsigned visflg(unsigned flags)
{
  return(flags & ~(ACCOUNT_NOTIFY | ACCOUNT_CANWHO));
}

void do_autologin(Character * ch, const char* argument)
{
	PlayerConnection * d = ch->GetConnection();

	if ( !d )
	{
		ch->sendText("You have no connection!\r\n");
		return;
	}
	if ( !d->Account )
	{
		ch->sendText("Your connection has no account!\r\n");
		return;
	}

	d->Account->autoLoginName_ = argument;

	ch->sendText("Will now automatically log in to " + string(argument) + ".\r\n");

	write_account_data(d->Account);
}

void do_accounts(Character * ch, const char* argument)
{
	char cmd[MAX_INPUT_LENGTH];
	
	if ( !argument || argument[0] == '\0' ) {
		send_to_char("Usage: accounts <command> <arguments>\r\n\r\n", ch);
		send_to_char("Where <command> is one of the following:\r\n\r\n", ch);
		
		ch_printf(ch, "\t%10s %10s %10s %10s\r\n", "connected", "ban",	"banned", "waiting");
		ch_printf(ch, "\t%10s %10s %10s %10s\r\n", "approve",	"deny", "show",   "set");
		
		return;
	}
	
	argument = one_argument(argument, cmd);
	
	if ( !str_prefix(cmd, "connected") ) {
		ACCOUNT_DATA* act;
		
		ch_printf(ch, "%-40s | %-3s | %-30s\r\n", "Email", "Num", "Flags");
		ch_printf(ch, "-----------------------------------------+-----+-------------------------------\n");
		
		for ( act = first_account; act; act = act->next ) {
			ch_printf(ch, "%-40s | %3d | %s\r\n",
				act->email, act->iref, flag_string(visflg(act->flags), account_flags));
		}
		
		return;
	} else if ( !str_prefix(cmd, "banned") ) {
		ACCOUNT_BAN_DATA* pban;
		ACCOUNT_DATA*	  pact;
		Character *		  vch;
		
		if ( argument[0] != '\0' ) {
			char target[MAX_INPUT_LENGTH];
			
			argument = one_argument(argument, target);
			
			if ( NULL != (pact = load_account_data(target)) ) {
				SET_BIT(pact->flags, ACCOUNT_BANNED);
				write_account_data(pact);
				free_account_data(pact);
				
				send_to_char("Account banned.\r\n", ch);
				return;
			} else if ( NULL != (vch = get_char_world(ch, target)) ) {
				int x,y;
				
				if ( IS_NPC(vch) ) {
					send_to_char("You can not ban NPCs.\r\n", ch);
					return;
				}
				
				if ( !vch->pcdata->email[0] ) {
					send_to_char("That character doesn't seem to have an account to ban!\r\n", ch);
					return;
				}
				
				if ( !(pact = load_account_data(vch->pcdata->email) ) ) {
					send_to_char("Could not load that character's account data!\r\n", ch);
					return;
				}
				
				SET_BIT(pact->flags, ACCOUNT_BANNED);
				write_account_data(pact);
				
				vch->sendText("Your account has been banned from this MUD.\r\n", false);
				
				quitting_char = vch;
				save_char_obj(vch);
				saving_char = NULL;
				extract_char(vch, TRUE);
				
				for ( x = 0; x < MAX_WEAR; x++ )
					for ( y = 0; y < MAX_LAYERS; y++ )
						save_equipment[x][y] = NULL;
					
					send_to_char("Player account banned.  Player disconnected.\r\n", ch);
					return;
			} else {
				for ( pban = first_account_ban; pban; pban = pban->next ) {
					if ( !str_cmp(target, pban->address) ) {
						send_to_char("But, that address is already banned!\r\n", ch);
						return;
					}
				}
				
				if ( argument[0] == '\0' ) {
					send_to_char("Usage: accounts ban <account> <banned|noautoallow>\r\n", ch);
					return;
				}
				
				CREATE(pban, ACCOUNT_BAN_DATA, 1);
				LINK(pban, first_account_ban, last_account_ban, next, prev);
				pban->address = str_dup(target);
				pban->secBanTime= secCurrentTime;
				
				if ( !str_prefix(argument, "banned") ) {
					SET_BIT(pban->flags, ACCOUNT_BANNED);
				} else {
					SET_BIT(pban->flags, ACCOUNT_NOAUTOALLOW);
				}
				
				save_account_banlist();
				
				send_to_char("Address banned.\r\n", ch);
				return;
			}
		} else { /* no argument -- show banned accounts */
			int bnum;
			
			send_to_pager("Banned/Watched accounts:\r\n", ch);
			
			send_to_pager("[ #] Time                     Address                  Flags\r\n", ch);
			send_to_pager("---- ------------------------ ------------------------ ----------\r\n", ch);
			for ( pban = first_account_ban, bnum = 1; pban; pban = pban->next, bnum++ ) {
				if ( IS_SET(pban->flags, ACCOUNT_WAITING) ) {
					continue; /* don't show waiting accounts under the ban list */
				}
				strncpy(cmd, ctime(&pban->secBanTime), 24);
				cmd[24] = '\0';
				pager_printf(ch, "[%2d] %-24s %24s %s\r\n", bnum,
					cmd, pban->address, flag_string(visflg(pban->flags), account_flags));
			}
			return;
		}
	} else if ( !str_prefix(cmd, "allow" ) ) {
		int num = -1;
		int bnum;
		ACCOUNT_BAN_DATA* pban;
		
		if ( argument[0] == '\0' ) {
			send_to_char("Remove which address from the account ban list?\r\n", ch);
			return;
		}
		
		if ( is_number(argument) ) {
			num = atoi(argument);
		}
		
		for ( pban = first_account_ban, bnum = 1; pban; pban = pban->next, bnum++ ) {
			if ( num == bnum || !str_cmp(argument, pban->address) ) {
				UNLINK(pban, first_account_ban, last_account_ban, next, prev);
				
				STRFREE(pban->address);
				DISPOSE(pban);
				save_account_banlist();
				send_to_char("Address no longer banned.\r\n", ch);
				return;
			}
		}
		
		if ( num >= bnum ) {
			send_to_char("Invalid number.\r\n", ch);
			return;
		}
		
		send_to_char("Address not banned, is it?\r\n", ch);
		return;
	} else if ( !str_prefix(cmd, "show") ) {
		ACCOUNT_DATA* act;
		NoteData* pnote;
		int vnum;
		Character * vch;
		
		if ( !argument || argument[0] == '\0' ) {
			do_accounts(ch, "connected");
			return;
		}
		
		vch = get_char_world(ch, argument);
		act = NULL;
		
		if ( vch )
		{
			if ( (vch->GetConnection()) && !IS_NPC(vch) ) {
				act = load_account_data(vch->GetConnection()->Account->email);
			} else {
				if (vch->pcdata)
					act = load_account_data(vch->pcdata->email);
			}
		}
		else
		{
			act = load_account_data(argument);
		}
		
		if ( !act ) {
			send_to_char("No such account found.\r\n", ch);
			return;
		}
		
		ch_printf(ch, "Information for account %s:\r\n", act->email);
		ch_printf(ch, "\r\n");
		ch_printf(ch, "    Email:        %s\r\n",	   act->email	  );
		ch_printf(ch, "    Created On:   %s",		   ctime(&act->secCreatedDate));
		ch_printf(ch, "    Characters:   %s\r\n",	   act->characters);
		ch_printf(ch, "    Char History:");
                
                Iterator<std::string> it = MakeIterator(act->allCharacters_);
                while (it.hasNext()) {
                    ch_printf(ch, " %s", it.next().c_str());
                }
                ch_printf(ch,"\r\n");
		ch_printf(ch, "    Flags:        %d (%s)\r\n", visflg(act->flags), flag_string(visflg(act->flags), account_flags ));
		ch_printf(ch, "    Instances:    %d\r\n",	   act->iref-1	  ); /* -1, 'cause we loaded another one */
		ch_printf(ch, "    Autologin:    %s.\r\n",	   act->autoLoginName_.c_str()	  ); /* -1, 'cause we loaded another one */
		
		if ( act->comments ) {
			ch_printf(ch, "\r\nComments:\r\n");
			
			vnum = 0;
			
			for ( pnote = act->comments; pnote; pnote = pnote->next ) {
				++vnum;
				
				ch_printf(ch, "%2d) %-10s [%s] %s\r\n",
					vnum,
					pnote->sender_.c_str(),
					pnote->date,
					pnote->subject);
			}
		}
		
		free_account_data(act);
	} else if ( !str_prefix(cmd, "set" ) ) {
		ACCOUNT_DATA* pact;
		Character * vch;
		char address[MAX_INPUT_LENGTH];
		char cmd[MAX_INPUT_LENGTH];
		
		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Usage: accounts set <account> flags <flags>\r\n", ch);
			return;
		}
		
		argument = one_argument(argument, address);
		vch = get_char_world(ch, address);
		
		if ( vch && vch->GetConnection()->Account ) {
			strcpy(address, vch->GetConnection()->Account->email );
		}
		
		if ( !(pact = load_account_data(address)) ) {
			send_to_char("No such account.\r\n", ch);
			return;
		}
		
		if ( !argument || argument[0] == '\0' ) {
			do_accounts(ch, "set");
			free_account_data(pact);
		}
		
		argument = one_argument(argument, cmd);
		
		if ( str_prefix("flags", cmd) ) { /* flags is the only valid option atm */
			do_accounts(ch, "set");
			free_account_data(pact);
			return;
		}
		
		while ( argument[0] != '\0' ) {
			char flag[MAX_INPUT_LENGTH];
			int value;
			char prefix;
			
			argument = one_argument(argument, flag);
			
			if ( flag[0] && (flag[0] == '-' || flag[0] == '+' ) ) {
				char *a = flag; char* b = a;
				++b; prefix = *a;
				while((*a++ = *b++));
			}
			
			value = get_accountflag(flag);
			
			if ( value < 0 || value > 31 ) {
				ch_printf(ch, "Unknown account flag: %s\r\n", flag);
			} else {
				if ( prefix == '-' ) {
					REMOVE_BIT(pact->flags, 1<<value);
				} else if ( prefix == '+' ) {
					SET_BIT(pact->flags, 1<<value);
				} else {
					TOGGLE_BIT(pact->flags, 1<<value);
				}
			}
		}
		
		write_account_data(pact);
		free_account_data(pact);
		
		send_to_char("Done.\r\n", ch);
		return;
	} else if ( !str_prefix(cmd, "waiting" ) ) {
		ACCOUNT_BAN_DATA* pban;
		int bnum;
		
		send_to_pager("Waiting accounts:\r\n", ch);
		send_to_pager("[ #] Time                     Address                 \r\n", ch);
		send_to_pager("---- ------------------------ ------------------------\r\n", ch);
		for ( pban = first_account_ban, bnum = 1; pban; pban = pban->next ) {
			if ( !IS_SET(pban->flags, ACCOUNT_WAITING) ) {
				continue; /* only show waiting accounts */
			}
			strncpy(cmd, ctime(&pban->secBanTime), 24);
			cmd[24] = '\0';
			pager_printf(ch, "[%2d] %-24s %24s\r\n", bnum,
				cmd, pban->address);
			bnum++;
		}
		return;
	} else if ( !str_prefix(cmd, "allow") ) {
		ACCOUNT_BAN_DATA *pban;
		int num = -1;
		int bnum;
		
		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Allow which banned account?\r\n", ch);
			return;
		}
		
		if ( is_number(argument) ) {
			num = atoi(argument);
		}
		
		for ( bnum = 1, pban = first_account_ban; pban; pban = pban->next ) {
			if ( IS_SET(pban->flags, ACCOUNT_WAITING) ) {
				continue;
			}
			
			if ( bnum == num || !str_cmp(argument, pban->address) ) {
				UNLINK(pban, first_account_ban, last_account_ban, next, prev);
				save_account_banlist();
			}
			
			STRFREE(pban->address);
			DISPOSE(pban);
			
			send_to_char("Address unbanned.\r\n", ch);
			break;
		}
	} else if ( !str_prefix(cmd, "approve") ) {
		ACCOUNT_DATA	 *pact;
		ACCOUNT_BAN_DATA *pban;
		char			  email[MAX_INPUT_LENGTH];
		char			  pword[8];
		int num = -1;
		int bnum;
		
		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Approve which waiting account?\r\n", ch);
			return;
		}
		
		if ( is_number(argument) ) {
			num = atoi(argument);
		}
		
		for ( bnum = 1, pban = first_account_ban; pban; pban = pban->next ) {
			if ( !IS_SET(pban->flags, ACCOUNT_WAITING) ) {
				continue;
			}
			if ( bnum == num || !str_cmp(argument, pban->address) ) {
				UNLINK(pban, first_account_ban, last_account_ban, next, prev);				  
				save_account_banlist();
				break;
			}
			
			bnum++;
		}
		
		if ( !pban ) {
			if ( is_number(argument) ) {
				send_to_char("Invalid waiting number.\r\n", ch);
				return;
			} else {
				strcpy(email, argument);
			}
		} else {
			strcpy(email, pban->address);
			STRFREE(pban->address);
			DISPOSE(pban);
		}
		
		
		if ( (pact = load_account_data(email)) != NULL ) {
			send_to_char("Woah!  Baby!  That account already, like, exists!\r\n", ch);
			free_account_data(pact);
			return;
		}
		
		pact = new Account;
		pact->email 	   = STRALLOC(email);
		strncpy(pword, random_password(), 8);
		pword[7] = '\0';
		pact->password_    = crypt(pword, random_password());
		pact->characters   = STRALLOC("");
		pact->secCreatedDate = time(0);
		pact->flags 	   = 0;
		pact->iref		   = 1;
		
		
		/* EVIL HACK */
		/* mail_account_password wants a descriptor, both for the account */
		/* and to send failure messages to */
		/* so we give it both AND DISPOSE OF THEM */
		// adjusted by Ksilyan...
		
		ACCOUNT_DATA * oldAccount = ch->GetConnection()->Account;
		ch->GetConnection()->Account = pact;
		
		if ( !mail_account_password(ch->GetConnection(), pword, NEW_ACCOUNT_FILE) ) {
			send_to_char("Weird case.  Email failed.  You're screwed.\r\n", ch);
			free_account_data(pact);
			ch->GetConnection()->Account = oldAccount;
			return;
		}
		
		
		write_account_data(pact);
		free_account_data(pact);
		ch->GetConnection()->Account = oldAccount;
		
		send_to_char("Address authorized.  Account created.  Player mailed.\r\n", ch);
		return;
	} else if ( !str_prefix(cmd, "deny") ) {
		ACCOUNT_BAN_DATA *pban;
		ACCOUNT_DATA	 *pact;
		char			  email[MAX_INPUT_LENGTH];
		int bnum;
		int num = -1;
		
		if ( !argument || argument[0] == '\0' ) {
			send_to_char("Deny which waiting account?\r\n", ch);
			return;
		}
		
		if ( is_number(argument) ) {
			num = atoi(argument);
		}
		
		for ( bnum = 1, pban = first_account_ban; pban; pban = pban->next ) {
			if ( !IS_SET(pban->flags, ACCOUNT_WAITING) ) {
				continue;
			}
			
			if ( bnum == num || !str_cmp(argument, pban->address) ) {
				UNLINK(pban, first_account_ban, last_account_ban, next, prev);
				save_account_banlist();
				break;
			}
			
			bnum++;
		}
		
		if ( !pban ) {
			if ( is_number(argument) ) {
				send_to_char("Invalid waiting number.\r\n", ch);
				return;
			} else {
				send_to_char("That email address is not awaiting approval.\r\n", ch);
				return;
			}
		} else {
			strcpy(email, pban->address);
			STRFREE(pban->address);
			DISPOSE(pban);
		}
		
		/* EVIL HACK */
		/* mail_account_password wants a descriptor, both for the account */
		/* and to send failure messages to */
		/* so we give it both AND DISPOSE OF THEM */
		pact->email = email;

		ACCOUNT_DATA * oldAccount = ch->GetConnection()->Account;
		ch->GetConnection()->Account = pact;
		
		if ( !mail_account_password(ch->GetConnection(), NULL, ACCOUNT_REJECTED_FILE) ) {
			send_to_char("Weird case.  Email failed.  You're screwed.\r\n", ch);
			delete pact;
			ch->GetConnection()->Account = oldAccount;
			return;
		}
		
		delete pact;
		ch->GetConnection()->Account = oldAccount;
		
		send_to_char("Account denied.  Player emailed.\r\n", ch);
		return;
	} else {
		do_accounts(ch, "");
	}
}

void save_account_banlist( void )
{
    ACCOUNT_BAN_DATA* pban;
    FILE* fp;

    fclose(fpReserve);

    if ( !(fp = fopen(SYSTEM_DIR ACCOUNT_BAN_LIST, "w" )) ) {
        bug("save_account_banlist: Cannot open " ACCOUNT_BAN_LIST, 0);
        perror(ACCOUNT_BAN_LIST);
        fpReserve = fopen(NULL_FILE, "r");
        return;
    }

    for ( pban = first_account_ban; pban; pban = pban->next )
        fprintf(fp, "%d %s~~%ld\n", pban->flags, pban->address, pban->secBanTime);
    fprintf(fp, "-1\n");
    fclose(fp);
    fpReserve = fopen(NULL_FILE, "r");
    return;
}

void load_account_banlist( void )
{
    ACCOUNT_BAN_DATA* pban;
    FILE* fp;
    char letter;
    int flags;

    if ( !(fp = fopen(SYSTEM_DIR ACCOUNT_BAN_LIST, "r" )) )
        return;
    
    for ( ; ; ) {
        if ( feof(fp) ) {
            bug("load_account_banlist: no -1 found.");
            fclose(fp);
            return;
        }

        flags = fread_number(fp);

        if ( flags == -1 ) {
            fclose(fp);
            return;
        }

        CREATE(pban, ACCOUNT_BAN_DATA, 1);
        pban->flags   = flags;
        pban->address = fread_string_nohash(fp);
        if ( (letter = fread_letter(fp)) == '~' )
            pban->secBanTime = fread_number(fp);
        else
        {
            ungetc(letter, fp);
            pban->secBanTime= -1;
        }

        LINK(pban, first_account_ban, last_account_ban, next, prev);
    }
}

int get_accountflag( const char*flag) {
    int x;

    for ( x = 0; x < MAX_ACCOUNT_FLAGS; x++ ) {
        if ( !str_cmp(flag, account_flags[x] ) ) {
            return x;
        }
    }
    return -1;
}

void write_account_data(ACCOUNT_DATA* act)
{
    FILE *fp;
    char fname[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    sprintf(fname, "%s/%s", ACCOUNTS_DIR, strlower(act->email));

    if ( !(fp = fopen(fname, "w")) ) {        
        sprintf(buf, "write_account_data: cannot open %s for write", fname);
        bug(buf);
        return;
    }

    fprintf(fp, "Password      %s~\r\n", act->password_.c_str());
    fprintf(fp, "ForumPassword %s~\n\r", (act->forumPassword_.length() > 0
                                          ? act->forumPassword_.c_str()
                                          : act->password_.c_str()) );
    fprintf(fp, "Flags         %d\r\n",  act->flags       );
    fprintf(fp, "Characters    %s~\r\n", act->characters  );
    fprintf(fp, "AllCharacters ");
   
    Iterator<std::string> it = MakeIterator(act->allCharacters_);
    while (it.hasNext())
    {
        fprintf(fp, " %s", it.next().c_str());
    }
    fprintf(fp, "~\r\n");
    
    fprintf(fp, "Email         %s~\r\n", act->email       );
    fprintf(fp, "CreatedDate   %ld\r\n",  act->secCreatedDate);

    if ( act->autoLoginName_.length() > 0 )
        fprintf(fp, "AutoLogin     %s~\r\n", act->autoLoginName_.c_str() );

    fprintf(fp, "\r\n");

    fwrite_comments(act, fp);

    fprintf(fp, "End\r\n");

    fclose(fp);
    return;
}

/* Dispose of an account data if iref == 0 */
void free_account_data(ACCOUNT_DATA* act) {
    if ( !act ) {
        return;
    }

    if ( --act->iref > 0 ) {
        return;
    }

    UNLINK(act, first_account, last_account, next, prev);
    
    delete act;
}

Account::Account()
{
    this->flags        = 0;
    this->secCreatedDate = 0;
    this->comments     = NULL;
}
Account::~Account()
{
    STRFREE(this->email);
    STRFREE(this->characters);
}

/*
 * Load an account file and return a structure describing it
 */
ACCOUNT_DATA* load_account_data(const char* address)
{
    FILE *fp;
    char fname[MAX_STRING_LENGTH];
    ACCOUNT_DATA* act;
    bool fMatch;
    char buf[MAX_STRING_LENGTH];
    const char *word;

    /* Don't load another copy */
    if ( (act = find_account_data(address)) ) {
        act->iref++;
        return act;
    }

	if (!address)
	{
		sprintf(buf, "We're having trouble with an account data! ACK!");
		log_string(buf);
		return NULL;
	}
	
    if(address[0]=='\0')
       return NULL;

    sprintf(fname, "%s/%s", ACCOUNTS_DIR, strlower(address));
    
    if ( !(fp = fopen(fname, "r")) ) {
        return NULL;
    }

	act = new Account();
    
    for ( ;; ) {
        word = feof(fp) ? "End" : fread_word(fp);
        fMatch = FALSE;

        switch ( UPPER(word[0]) ) {
            case '#':
                if ( !str_cmp(word, "#COMMENT") ) {
                    fread_to_eol(fp);

                    fread_comment(act, fp);
                    fMatch = TRUE;
                }
                break;
            case '*':
                fMatch = TRUE;
                fread_to_eol(fp);
                break;
            case 'P':
                KEY("Password",    act->password_,        fread_string_noheap(fp));
                break;
            case 'F':
                KEY("Flags",       act->flags,        fread_number(fp));
                KEY("ForumPassword", act->forumPassword_, fread_string_noheap(fp));
                break;
			case 'A':
                                if (!str_cmp(word, "AllCharacters"))
                                {
                                    std::string allChars = fread_string_noheap(fp);
                                    std::istringstream is(allChars);

                                    act->allCharacters_ = std::set<std::string>(std::istream_iterator<std::string>(is), std::istream_iterator<std::string>());
                                    fMatch = true;
                                    break;
                                }
				KEY("AutoLogin", act->autoLoginName_, fread_string_noheap(fp));
                                break;
            case 'C':
                KEY("Characters",  act->characters,   fread_string(fp));
                KEY("CreatedDate", act->secCreatedDate, fread_number(fp));  
                break;
            case 'E':
                KEY("Email",      act->email,         fread_string(fp));
                if ( !str_cmp(word, "End") ) {
                    if ( !act->characters ) act->characters = STRALLOC("");
                    if ( !act->email      ) act->email      = STRALLOC("");
                    if ( act->forumPassword_.length() == 0 ) {
                        act->forumPassword_ = act->password_;
                    }
                    if ( act->secCreatedDate == 0 ) act->secCreatedDate = time(0);
                }
                
                act->iref = 1;

                fclose(fp);
                LINK(act, first_account, last_account, next, prev);
                return act;
        }

        if ( !fMatch ) {
            sprintf(buf, "load_account_data: no match: %s", word);
            bug(buf, 0);
        }
    }
}

ACCOUNT_DATA* get_account(Character *ch, const char* argument)
{
    Character * vch;

    if ( (vch = get_char_world(ch, argument)) && vch->GetConnection() && vch->GetConnection()->Account ) {
        vch->GetConnection()->Account->iref++;
        return vch->GetConnection()->Account;
    }

    return load_account_data(argument);
}
    
