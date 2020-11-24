
#include <stdlib.h>
#include <ctype.h>

#include "globals.h"
#include "connection_manager.h"
#include "character.h"
#include "connection.h"
#include "mud.h"
#include "World.h"
#include "mxp.h"

#include "utility_objs.hpp"

#include "paths.const.h"

#ifdef unix
   #include "unistd.h"
#else
   #include <io.h>
#endif

// Forward Declarations
void do_help(Character * ch, const char* argument);
void do_return(Character * ch, const char* argument);
void do_look(Character * ch, const char* argument);

/*
  _____                 __                   __               ____   
 / ___/___   ___   ___ / /_ ____ __ __ ____ / /_ ___   ____  / __/___
/ /__ / _ \ / _ \ (_-</ __// __// // // __// __// _ \ / __/  > _/_ _/
\___/ \___//_//_//___/\__//_/   \_,_/ \__/ \__/ \___//_/    |_____/                                                                       
   ___            __                   __            
  / _ \ ___  ___ / /_ ____ __ __ ____ / /_ ___   ____
 / // // -_)(_-</ __// __// // // __// __// _ \ / __/
/____/ \__//___/\__//_/   \_,_/ \__/ \__/ \___//_/   
                                                     
*/

ConnectionManager::ConnectionManager()
{

}


ConnectionManager::~ConnectionManager()
{
   itorSocketId itor;
   for ( itor = Sockets.begin(); itor != Sockets.end(); itor++ )
   {
      if ( SocketMap[(*itor)] )
      {
         this->RemoveSocket(SocketMap[(*itor)], true);
         delete SocketMap[(*itor)];
      }
   }
   Sockets.clear();
}

void ConnectionManager::ForceFlushOutput()
{
   FD_ZERO(&in_set);
   FD_ZERO(&exc_set);

   time_t startedTime = secCurrentTime;

   bool found = false;
   do
   {
      found = false;
      struct timeval timeoutDelay;
      timeoutDelay.tv_sec = 0;
      timeoutDelay.tv_usec = 0;
      FD_ZERO(&out_set);
      
      itorSocketId itor;

      short biggestDesc = 0;

      // Construct the list of descriptors to check.
      for (itor = Sockets.begin(); itor != Sockets.end(); itor++)
      {
         SocketGeneral * socket = SocketMap[*itor];
         if ( !socket )
            continue;

         short desc = socket->GetDescriptor();
         if (desc == -1) continue;

         if ( socket->IsA(estConnection) && socket->WantsToWrite() )
         {
            found = true;
            FD_SET(desc, &out_set);
         }

         #ifdef unix
            // On windows, we don't need to bother with this.
            biggestDesc = MAX(desc, biggestDesc);
         #endif
      }
   
      if ( select(biggestDesc + 1, NULL, &out_set, NULL, &timeoutDelay) < 0 )
      {
         Error = esmeCouldntSelect;
         return;
      }

      this->ProcessActiveSockets();

      struct timeval now_time;
      gettimeofday( &now_time, NULL );
      secCurrentTime = (time_t) now_time.tv_sec;

   } while (found && secCurrentTime - startedTime > 60); // only flush for a minute
   
}



/*
* Check if already playing.
*/
bool ConnectionManager::CheckPlaying( PlayerConnection * d, const char * name, bool kick )
{
   itorSocketId itor;

   for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
   {
      Character *ch;
      PlayerConnection *dold;
      eConnectedStates cstate;

      dold = (PlayerConnection*) SocketMap[*itor];

      if ( !dold )
      {
         gTheWorld->LogBugString("A bad socket was found in ConnectionManager::CheckPlaying! Removing.");
         this->RemoveSocket(*itor, true);
         continue;
      }

      // Check to see if the connection is not the same, but
      // the player it points to is
      if ( dold != d
         && dold->GetOriginalCharacter()
         && dold->GetOriginalCharacter()->getName().ciEqual(name) )
      {
         cstate = dold->ConnectedState;
         ch = dold->GetOriginalCharacter();
         if ( ch->getName().length() == 0
            || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
         {
            d->SendText("Already connected - try again.\n\r");
            sprintf( log_buf, "%s already connected.", ch->getName().c_str() );
            log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
            throw eAlreadyPlaying();
         }
         if ( !kick )
            return true;
         d->SendText( "Already playing... Kicking off old connection.\n\r");
         dold->SendText( "Kicking off old connection... bye!\n\r" );
         RemoveSocket(dold, false);
         
         /* clear descriptor pointer to get rid of bug message in log */
         d->GetCharacter()->SetConnection(NULL);

         free_char( d->GetCharacter() );
         d->CurrentCharId = ch->GetId();
         ch->SetConnection(d);

         ch->timer    = 0;
         if ( ch->SwitchedCharId != 0 )
            do_return( CharacterMap[ch->SwitchedCharId], "" );
         ch->SwitchedCharId = 0;
         send_to_char( "Reconnecting.\n\r", ch );
         act( AT_ACTION, "$n has reconnected, kicking off old link.",
            ch, NULL, NULL, TO_ROOM );
         sprintf( log_buf, "%s@%s reconnected, kicking off old link.",
            ch->getName().c_str(), d->GetHost() );
         log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
         /*
         if ( ch->level < LEVEL_SAVIOR )
         to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );
         */
         d->SetInputReceiver(ch);
         d->ConnectedState = cstate;
         return true;
      }
   }
   
   return false;
}

/*
 * Look for link-dead player to reconnect.
 */
bool ConnectionManager::CheckReconnect( PlayerConnection *d, const char *name, bool fConn )
{
   Character *ch;
   
   // Loop through all players, seeing if one is
   // the person trying to reconnect
   for ( ch = first_char; ch; ch = ch->next )
   {
      if ( !IS_NPC(ch)
         && ( !fConn || !ch->GetConnection() )
         &&    ch->getName().ciEqual(name) )
      {
         if ( fConn && ch->SwitchedCharId != 0 )
         {
            d->SendText( "Already playing.\n\rName: ");
            d->ConnectedState = CON_GET_NEW_NAME;
            if ( d->GetCharacter() )
            {
               /* clear descriptor pointer to get rid of bug message in log */
               d->GetCharacter()->SetConnection( NULL );

               free_char( d->GetCharacter() );
               d->CurrentCharId = 0;
            }
            throw eAlreadyPlaying();
         }
         if ( fConn == false )
         {
            DISPOSE( d->GetCharacter()->pcdata->pwd );
            d->GetCharacter()->pcdata->pwd = str_dup( ch->pcdata->pwd );
         }
         else
         {
            /* clear descriptor pointer to get rid of bug message in log */
            d->GetCharacter()->SetConnection(NULL);

            free_char( d->GetCharacter() );

            d->CurrentCharId = ch->GetId();
            ch->SetConnection(d);

            ch->timer    = 0;
            send_to_char( "Reconnecting.\n\r", ch );
            act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
            if (d->User.length() > 0)
               sprintf( log_buf, "%s@%s(%s) reconnected.", ch->getName().c_str(), d->GetHost(), d->User.c_str() );
            else
               sprintf( log_buf, "%s@%s reconnected.", ch->getName().c_str(), d->GetHost() );
            
            log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
            /*
            if ( ch->level < LEVEL_SAVIOR )
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );
            */
            d->ConnectedState = CON_PLAYING;
            d->SetInputReceiver(ch);
            ch->SendPrompt();
         }
         return true;
      }
   }
   
   return false;
}

bool ConnectionManager::CheckParseEmail(const char *email, PlayerConnection * d)
{
   const char *p = email;
   char buf[MAX_STRING_LENGTH];
   
   /*
   //we don't care if the account is an email address anymore
   if ( NULL == strstr(email, "@") ) {
      d->SendText("Please enter a valid account name in the form of:\r\n"
         "    user@dom.ain\r\n");
      return false;
   }*/
   
   if ( strlen(email) > 80 ) {
      d->SendText("While that may be a valid account name, we do not allow\r\n"
         "account namees that long.\r\n");
      return false;
   }
   
   for ( p = email; *p != '\0'; p++ ) {
      switch (*p)
      {
         case '/':
         case '*':
         case '(':
         case ')':
         case '"':
         case '\'':
         case '\\':
         {
            sprintf(buf, "Although possibly a valid account name, we do not currently\r\n"
               "allow account namees with the '%c' character in them.\r\n", *p);
            d->SendText(buf);
            return false;
         }
      }
   }
   
   return true;
}

/*
* Parse a name for acceptability.
*/
bool ConnectionManager::CheckParseName( const char * name, PlayerConnection * d )
{
/*
* Reserved words.
   */
    if ( is_name( name, "all auto immortal self someone god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit guest new" ) )
    {
        if ( d ) 
            d->SendText("\r\nThat name is already taken.  Please choose another: ");
      return false;
    }
   
    /*
   * Length restrictions.
   */
    if ( strlen(name) <  3 )
    {
        if ( d )
            d->SendText("\r\nThat name is too short.  Please choose another: ");
        return false;
    }
    
    if ( strlen(name) > 12 )
    {
        if ( d )
            d->SendText("\r\nThat name is too long.  Please choose another: ");
        return false;
    }
   
    /*
   * Alphanumerics only.
   * Lock out IllIll twits.
   */
    {
        const char *pc;
        bool fIll;
      
        fIll = TRUE;
      
        for ( pc = name; *pc != '\0'; pc++ )
        {
            if ( !isalpha(*pc) )
            {
                if ( d )
                    d->SendText("\r\nYour name must contain only letters.  Please choose another: ");
                return false;
            }
            if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
                fIll = false;
        }
      
        if ( fIll ) {
            if ( d )
                d->SendText("\r\nPlease make some attempt at a reasonable name: ");
            return false;
        }
    }
   
    /*
   * Code that followed here used to prevent players from naming
   * themselves after mobs... this caused much havoc when new areas
   * would go in...
   */
   
    return true;
}

///////////////////////////////////////////////////////////
/// INTERNAL & MAINTENANCE FUNCTIONS

void ConnectionManager::RemoveSocket(idSocket id, bool removeNow)
{
   if ( id == 0 )
      return;

   // "Further" remove the socket only if it's a valid reference
   SocketGeneral * socket = SocketMap[id];

   if ( socket )
      this->RemoveSocket(socket, removeNow);
   else
   {
      toDelete.push_back( id );
      gTheWorld->RemoveConnection( id );
   }
}

void ConnectionManager::RemoveSocket(SocketGeneral * socket, bool removeNow)
{
   if ( !listContains(Sockets, socket->GetId() ) )
   {
      gTheWorld->LogBugString("Trying to remove a socket that is not in the list:");
      gTheWorld->LogBugString( socket->codeGetBasicInfo() );
      return; // abort if the list does not contain this socket.
   }

   if ( listContains(toDelete, socket->GetId()) )
   {
      // if we're trying to *force* removal and the socket is currently set to not force, then
      // change its status to force remove = yes
      if ( removeNow && !socket->RemoveNow )
      {
         socket->RemoveNow = true;
         return;
      }
      gTheWorld->LogBugString("Trying to remove a socket a second time:");
      gTheWorld->LogBugString( socket->codeGetBasicInfo() );
      return; // abort if we've already marked this socket for deletion
   }

   // Do whatever socket manager wants us to do
   SocketManager::RemoveSocket(socket, removeNow);

   if ( socket->IsA(estListener) )
      return; // we only want to deal further with connections

   PlayerConnection * toRemove = (PlayerConnection *) socket;

   // stop snooping everyone else
   toRemove->CancelAllSnoops();

   Character * ch;

   // Check for switched people who go link-dead. -- Altrag
   if ( toRemove->OriginalCharId != 0 )
   {
      if ( ( ch = toRemove->GetCharacter() ) != NULL )
         do_return(ch, "");
      else
      {
         bug( "RemoveSocket: toRemove->original without character %s",
            (toRemove->GetOriginalCharacter()->getName().length() > 0 ?
             toRemove->GetOriginalCharacter()->getName().c_str() : "unknown"
             )
            );
         toRemove->CurrentCharId = toRemove->OriginalCharId;
         toRemove->OriginalCharId = 0;
      }
   }
   ch = toRemove->GetCharacter();

   if ( ch )
   {
      sprintf( log_buf, "Closing link to %s.", ch->getName().c_str() );
      log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
      
      if ( toRemove->ConnectedState == CON_PLAYING
         ||    toRemove->ConnectedState == CON_EDITING )
      {
         act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );

         ch->SetConnection(NULL);
      }
      else
      {
         // clear descriptor pointer to get rid of bug message in log
         toRemove->GetCharacter()->SetConnection(NULL);
         free_char( toRemove->GetCharacter() );
      }
      toRemove->CurrentCharId = 0;
   }

   if ( toRemove->auth_fd != -1 ) 
      close( toRemove->auth_fd );
   toRemove->auth_fd = -1;

   gTheWorld->RemoveConnection(toRemove);

   return;
}



/////////////////////////////////


/*
 * Deal with sockets that haven't logged in yet.
 * (This used to be called 'nanny' in SMAUG).
 */
void ConnectionManager::ProcessLine(const string line, SocketGeneral * socket )
{
   PlayerConnection * connection = (PlayerConnection *) socket;
   
   CHAR_DATA * ch;
   ch = connection->GetCharacter();
   BAN_DATA * pban;
   
   bool fOld;
   bool chk;

   char buf[MAX_STRING_LENGTH];

   //printf("Received input line from %s\n\r", connection->GetHost());
   
   switch ( connection->ConnectedState )
   {
      default:
      {
         bug( "Nanny: Unknown connection state: %d.", connection->ConnectedState );
         RemoveSocket(socket, true);
         return;
      }
      case CON_GET_NEW_NAME:
      {
         if ( line.length() == 0 )
         {
            connection->SendText("\r\n");
            connection->WriteMainMenu();
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            break;
         }
         
         char Name[MAX_STRING_LENGTH];
         strcpy(Name, line.c_str());
         
         Name[0] = UPPER(Name[0]);
         
         if ( !CheckParseName(Name, connection) )
         { /* cpn sends messages */
            return;
         }
         
         try {
            chk = CheckPlaying( connection, Name, false );
         } catch (eAlreadyPlaying) {
            connection->SendText("\r\nName: ");
            return;
         }
         
         fOld = load_char_obj(connection, Name, true);
         
         for ( pban = first_ban; pban; pban = pban->next )
         {
            /* This used to use str_suffix, but in order to do bans by the 
             * first part of the ip, ie "ban 207.136.25" str_prefix must
             * be used. -- Narn
             */
            if ( (!str_prefix( pban->site, connection->GetHost() )||!str_suffix(pban->site, connection->GetHost() )) )
            {
               connection->SendText("\r\nYour site has been banned from this Mud.\r\n");
               this->RemoveSocket( connection, false );
               return;
            }
         }
         
         if ( !IS_SET(connection->Account->flags, ACCOUNT_CANMPLAY) && connection->Account->iref > 1 ) {
            connection->SendText(
               "\r\n"
               "Someone is already connected from this account.\r\n"
               "If you have recently connected, try waiting a few minutes before trying again."
               "\r\n\r\n");
            connection->WriteMainMenu();
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            return;
         }
         
         
         if ( ch && IS_SET(ch->act, PLR_DENY) )
         {
            connection->SendText( "\r\nThat name is already taken.  Please choose another: " );
            connection->ConnectedState = CON_GET_NEW_NAME;
            return;
         }
         
         if ( fOld )
         {
            connection->SendText( "\r\nThat name is already taken.  Please choose another: ");
            connection->ConnectedState = CON_GET_NEW_NAME;
            return;
         }
         else
         {
            char buf[MAX_INPUT_LENGTH+34];
            sprintf( buf, "\r\nDid I get that right, %s (Y/N)? ", Name );
            connection->SendText(buf);
            connection->ConnectedState = CON_CONFIRM_NEW_NAME;
            return;
         }
         break;
      }
      case CON_GET_EMAIL:
      {
         if ( ci_equal_to() (line, "new") )
         {
            if ( connection->newstate == 0 )
            {
               if (newbielock)
               {
                  connection->SendText("\r\nThe mud is currently not accepting new accounts. Please try again later.\r\n");
                  RemoveSocket(connection);
               }
               /* New player */
               /* Don't allow new accounts if DENY_NEW_PLAYERS is true */
               if ( sysdata.DENY_NEW_PLAYERS == TRUE)
               {
                  connection->SendText("The mud is currently preparing for a reboot.\r\n");
                  connection->SendText("New players are not accepted during this time.\r\n");
                  connection->SendText("Please try again in a few minutes.\r\n");
                  
                  RemoveSocket(connection);
               }
               
               connection->SendText(
                  "\r\n"
                  "\r\n"
                  "Welcome to Legends of the Darkstone!  We are confident you\r\n"
                  "will enjoy your visit.\r\n"
                  "\r\n"
                  "Before you can get down to playing the game, you need to\r\n"
                  "create an account on the MUD.  Once you have done this, you\r\n"
                  "will be able to create new characters, play existing ones,\r\n"
                  "and access some special areas on our website.\r\n"
                  "\r\n"
                  "Please enter a name for your account: "); 
               
               connection->ConnectedState = CON_GET_NEW_ACCOUNT;
               return; /* Go through this again */
            } else {
               connection->SendText("\r\nEnter your account name, or NEW if you are new here: ");
               return;
            }
         }
         
         //char Email[MAX_INPUT_LENGTH];
         
         if ( line.length() == 0 ) {
            RemoveSocket(connection);
            return;
         }
         
         if ( !CheckParseEmail(line.c_str(), connection) ) {
            connection->SendText("\r\nEnter your account name, or NEW if you are new here: ");
            return;
         }
         
         
         connection->Account = load_account_data( line.c_str() );
         
         if ( !connection->Account )
         {
            connection->SendText("\r\nNo such account exists.\r\n"
               "\r\nEnter your account name, or NEW if you are new here: ");
            return;
         }
         
         if ( connection->Account->password_.length() == 0 )
         {
            const char* pword;
            connection->SendText("\r\nHmm, this account has no password assigned.  Please enter a new password\r\n");
            connection->ConnectedState = CON_GET_NEW_PASSWORD;
            return;
            connection->Account->password_ = crypt(pword = random_password(), random_password());
            write_account_data(connection->Account);
            
            if ( mail_account_password(connection, pword, NEW_PASSWORD_FILE) ) {
               connection->SendText("Password sent.  Check your email.\r\n\r\n");
            } else {
               connection->SendText("Darkstone was unable to send the password.\r\nPlease contact an immortal.\r\n\r\n");
            }
            
            this->RemoveSocket(connection, false);
            return;
         }
         
         for ( pban = first_ban; pban; pban = pban->next )
         {
            /* This used to use str_suffix, but in order to do bans by the 
             * first part of the ip, ie "ban 207.136.25" str_prefix must
             * be used. -- Narn
             */
            if ( (!str_prefix( pban->site, connection->GetHost() )||!str_suffix(pban->site, connection->GetHost() )) )
            {
               connection->SendText( "\r\n"
                  "Your site has been banned from this Mud.\n\r");
               RemoveSocket(connection);
               return;
            }
         }

         // hacked together ban for Gelltor
         {
            string host = connection->GetHost();
            if ( host == "ip24-250-154-120.bc.dl.cox.net" )
            {
               connection->SendText("\r\n");
               connection->SendText("Your site has been banned from this Mud.\n\r");
               connection->SendText("If you believe this to be in error, please contact the administrators.\n\r");
               gTheWorld->LogString("Gelltor's host banned; removing.");
               RemoveSocket(connection);
               return;
            }
         }
         
         if ( connection->Account->flags & ACCOUNT_BANNED ) {
            connection->SendText("\r\n");
            connection->SendText("This account has been banned from the MUD.\r\n");
            this->RemoveSocket(connection);
            return;
         }
         
         
         //sprintf(log_buf, "Connect from account %s", connection->Account->email);
         //gTheWorld->LogCommString(log_buf);
         
         connection->SendText("\r\nEnter the password for this account: ");
         connection->SendText( (const char*) echo_off_str);
         connection->ConnectedState = CON_GET_OLD_PASSWORD;

         // Send MXP password request.
         connection->SendText( MXPTAG("password") );
         
         break;
      }
      case CON_GET_OLD_PASSWORD:
      {
         connection->SendText("\n\r");
         
         if ( !line.compare("resendxxx") ) {
            /* send a new password to the account */
            const char* pword;
            
            if(connection->Account->flags & ACCOUNT_IMMORTAL)
            {/* let hacker think attack succeeded */
               connection->SendText("\r\nPassword sent.  Check your email.\r\n\r\n");
               this->RemoveSocket(connection, false);
               return;
            }
            
            connection->Account->password_ = crypt(pword = random_password(), random_password());
            write_account_data(connection->Account);
            
            if ( mail_account_password(connection, pword, NEW_PASSWORD_FILE) ) {
               connection->SendText("\r\nPassword sent.  Check your email.\r\n\r\n");
            }
            this->RemoveSocket(connection, false);
            return;
         } 
         
         if ( strcmp( crypt( line.c_str(), connection->Account->password_.c_str() ), connection->Account->password_.c_str() ) )
         {
            connection->SendText("Wrong password.\n\r");
            gTheWorld->LogCommString("Kicking off connecting account " + string(connection->Account->email) + " (bad password)");
            this->RemoveSocket(connection, false );
            return;
         }
         
         connection->SendText( (const char*) echo_on_str );
         connection->SendText( "Account logged on.  " );

         gTheWorld->LogCommString("Connect from account " + string(connection->Account->email));

         // If the account has an auto-login, do that.
         // Otherwise, show the menu as normal.
         if ( connection->Account->autoLoginName_.length() == 0 )
            connection->WriteMainMenu();
         else
         {
            connection->InputBuffer.append("3\r\n"); // for character selection
            connection->InputBuffer.append(connection->Account->autoLoginName_.str() + "\r\n");
         }
         
         
         connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
         break;
      }
      case CON_GET_NEW_ACCOUNT:
      {
         char pword[8];
         ACCOUNT_BAN_DATA* pban;
            
         if ( line.length() == 0 ) {
            this->RemoveSocket(connection, false);
            return;
         }
            
         if ( !CheckParseEmail(line.c_str(), connection) ) {
            connection->SendText("\r\nEnter your account name, or 'new' if you are new here: ");
            return;
         }
            
         connection->Account = load_account_data(line.c_str());
            
         if ( connection->Account ) {
            connection->SendText("\r\nThat account has been taken already.\r\n");
            RemoveSocket(connection, false);
            return;
         }
            
         for ( pban = first_account_ban; pban; pban = pban->next )
         {
            if ( !str_prefix(pban->address, line.c_str()) || !str_suffix(pban->address, line.c_str()) )
            {
               if ( IS_SET(pban->flags, ACCOUNT_BANNED) )
               {
                  connection->SendText("\r\n\r\n"
                     "Your account has been automatically banned.\r\n"
                     "If you feel that this is in error, please contact the\r\n"
                     "immortals.  Be sure to let us know what account name\r\n"
                     "you attempted to register, so we have a chance to fix\r\n"
                     "the problem.  Email us at: immortals@darkstone.mudservices.com.\r\n\r\n");
                  this->RemoveSocket(connection, false);
                  return;
               }
               else if ( IS_SET(pban->flags, ACCOUNT_WAITING) )
               {
                  connection->SendText("\r\n\r\n"
                     "Your account is already awaiting automatic approval.\r\n"
                     "Please wait just a little bit longer before trying\r\n"
                     "back.  You will be emailed when your account has been\r\n"
                     "accepted.\r\n\r\n");
               }
               else if ( IS_SET(pban->flags, ACCOUNT_NOAUTOALLOW) )
               {
                  ACCOUNT_DATA* pact;
                     
                  connection->SendText("\r\n\r\n"
                     "Your account has been denied automatic acceptance.\r\n"
                     "There are a variety of reasons for this, but the most likely\r\n"
                     "Case is that your email provider has been known to cause\r\n"
                     "problems in the past.  Do not despair, for you can still\r\n"
                     "play here; you just have to wait for an immortal to approve\r\n"
                     "your account.\r\n"
                     "\r\n"
                     "An email has been sent to the address you provided, explaining\r\n"
                     "the situation in a little more detail.  If you do not receive\r\n"
                     "this email, please reconnect and try to register again.\r\n\r\n");
                     
                  pact = new Account;
                  pact->email = (char *) line.c_str();                  
                  connection->Account = pact;
                  connection->SendText( "\r\nNow you need to create a password for your new account.\r\nPlease enter a new password: " );
                  connection->ConnectedState = CON_GET_NEW_PASSWORD;
                  mail_account_password(connection, NULL, AUTO_REJECT_FILE);
                  connection->Account = NULL;
                  delete pact;
                  this->RemoveSocket(socket, false);
                     
                  CREATE(pban, ACCOUNT_BAN_DATA, 1);
                  LINK(pban, first_account_ban, last_account_ban, next, prev);
                  pban->address = str_dup(line.c_str());
                  pban->secBanTime = secCurrentTime;
                  pban->flags = ACCOUNT_WAITING;
                  save_account_banlist();
                     
                  sprintf(log_buf, "Account %s awaiting approval.", line.c_str());
                  log_string(log_buf);
                     
                  return;
               }
            }
         }
                  
         strcpy(pword, random_password());
            
         connection->Account = new Account;
         connection->Account->email       = STRALLOC((char *) line.c_str());
         connection->Account->password_       = crypt(pword, random_password());
         connection->Account->characters    = STRALLOC("");
         connection->Account->secCreatedDate = time(0);
         connection->Account->flags       = 0;
         connection->Account->iref       = 1;
         LINK(connection->Account, first_account, last_account, next, prev);
            
         /*
         if ( mail_account_password(connection, pword, NEW_ACCOUNT_FILE) ) {
            connection->SendText("\r\n"
               "Your password has been emailed to the account name you specified.\r\n"
               "If you do not receive the email, reconnect, log on (don't type new)\r\n"
               "with the same account name, and follow the instructions.\r\n");
         }*/
            
         write_account_data(connection->Account);
         connection->ConnectedState = CON_GET_NEW_PASSWORD;
         
         connection->SendText("\r\nEnter your new password:  ");
         connection->SendText( (const char*) echo_off_str);
         /*this->RemoveSocket(connection, false);*/
            
         break;
      }
      
      case CON_READ_MOTD_NOCHAR:
      {
         connection->WriteMainMenu();
         connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
         break;
      }
      case CON_GET_MAIN_MENU_CHOICE:
      {
         switch ( *line.c_str() )
         {
            case '1':
            {
               connection->SendText("\r\nEnter your new password:  ");
               connection->SendText( (const char*) echo_off_str);
               connection->ConnectedState = CON_GET_NEW_PASSWORD;
               break;
            }
            case '2':
            {
               connection->SendText( strip_color_codes( get_help( NULL, "motd" )->text+1 ) );
               connection->SendText( "\r\n\r\nPress <enter> to return to the main menu\r\n" );
               connection->ConnectedState = CON_READ_MOTD_NOCHAR;
               break;
            }
            case '3':
            {
               char buf[MAX_INPUT_LENGTH];
               char name[MAX_INPUT_LENGTH];
               char *characters = connection->Account->characters;
               int i = 0;
               
               connection->SendText(
                  "\r\n"
                  "The following characters are associated with this account.\r\n"
                  "If one of your characters is not listed here, choose option\r\n"
                  "2 from the main menu, and add that character.\r\n"
                  "\r\n");
               
               if ( !characters || characters[0] == '\0' ) {
                  connection->SendText("You don't have any characters associated with this account.\r\n");
                  connection->WriteMainMenu();
                  connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
                  break;
               }
               
               /* print out the characters listed for this account,
               4 per line */
               while ( characters[0] != '\0' ) {
                  characters = one_argument(characters, name);
                  
                  sprintf(buf, "%d) %-10s", 1+i, capitalize(name));
                  connection->SendText(buf);
                  
                  if ( (++i % 5) == 0 )
                     connection->SendText("\r\n");
               }
               
               if ( (i % 5) != 0 )
                  connection->SendText("\r\n");
               
               connection->SendText("\r\nWhich character would you like to play? ");
               
               connection->ConnectedState = CON_GET_CHAR_NAME;
               break;
            }

            case '4':
            {
               if(newbielock)
               {
                  connection->SendText (
                     "\r\nThe mud is currently not accepting new characters.  Please try again later\r\n");
                  connection->WriteMainMenu();
                  connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
                  break;
               }
               
               if ( sysdata.DENY_NEW_PLAYERS == TRUE) {
                  connection->SendText("\r\n");
                  connection->SendText("The MUD is currently preparing for a reboot.\r\n");
                  connection->SendText("New players are not accepted during this time.\r\n");
                  connection->SendText("Please try again in a few minutes.\r\n\r\n");
                  connection->WriteMainMenu();

                  connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
                  break;
               }
               
               connection->SendText(
                  "\r\n"
                  "Choosing a name is one of the most important parts of this game.\r\n"
                  "Your choice of a name affects your character, how you can role play\r\n"
                  "and, in general, how other players treat you.\r\n"
                  "\r\n"
                  "We ask that all names suit a medieval theme.  Names will not be\r\n"
                  "accepted if they are known to have come from a movie, book, or\r\n"
                  "popular story.  We also ask that you do not use common names,\r\n"
                  "like those of people you would know in real-life.  Please also\r\n"
                  "refrain from using anything you can find in a dictionary.\r\n"
                  "\r\n"
                  "If your name is not acceptable, you will be asked to change it.\r\n"
                  "\r\n"
                  "Enter the name you would you like to be known by: ");
               connection->newstate++;
               connection->ConnectedState = CON_GET_NEW_NAME;
               break;
            }
            case '5':
            {
               connection->SendText(
                  "\r\n\r\n"
                  "Currently, the only way to delete a character is to log on\r\n"
                  "with that character and use the 'delete' command.\r\n"
                  "\r\n");
               
               connection->WriteMainMenu();
               
               break;
            }
            
            case '6':
            {
               char buf[MAX_INPUT_LENGTH];

               connection->SendText("\r\n\r\nCome back real soon!\r\n");
               if (connection->Account)
                  sprintf(buf, "Account %s disconnected.", connection->Account->email);
               else
                  sprintf(buf, "For some reason, the last quit had no account. ??");
               gTheWorld->LogCommString(buf);

               this->RemoveSocket(connection, false);
               break;
            }

            default:
            {
               connection->SendText("\r\nUnknown option.  ");
               connection->WriteMainMenu();
               break; /* It'll come right back to this case */
            }
         }
         break;
      }
      case CON_GET_CHAR_NAME:
      {
         char name[MAX_INPUT_LENGTH];
         
         strcpy(name, line.c_str());
         
         if ( name[0] == '\0' ) {
            connection->SendText("\r\n");
            connection->WriteMainMenu();
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            break;
         }
         
         /* Before going anywhere, check to see if that name is even in the list */           
         if ( !is_name(name, connection->Account->characters) && !is_number(name)) {
            connection->SendText("\r\n");
            connection->SendText("That character isn't assigned to your account.\r\n");
            connection->SendText("Either you made a typo, or perhaps you wish to add\r\n");
            connection->SendText("it to this account (via option '2')?\r\n\r\n");
            connection->WriteMainMenu(); 
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            break;
         }
         
         if ( is_number(line.c_str() ) )
         {
            int i = 1;
            const char * characters = connection->Account->characters;
            
            while( characters[0] != '\0' ) {
               characters = one_argument((char *) characters, name);
               
               if ( i++ == atoi(line.c_str() ) ) {
                  i = -1;
                  break;
               }
            }
            
            if ( i == -1 ) {
               sprintf(log_buf, "Loading up %s.\r\n\r\n", name);
            } else {
               connection->SendText("\r\n");
               connection->SendText("Selection out of range.\r\n");
               connection->SendText("Please try again.\r\n\r\n");
               
               connection->WriteMainMenu();
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               break;
            }
         }
         
         name[0] = UPPER(name[0]);
         
         fOld = load_char_obj(connection, name, TRUE);
         ch = connection->GetCharacter();
         
         if ( !fOld )
         {
            char* New;
            
            connection->SendText("\r\nAlthough on the list, that character doesn't exist.\r\n");
            connection->SendText("Removing it from the list.\r\n\r\n");
            
            connection->GetCharacter()->SetConnection( NULL );

            free_char(connection->GetCharacter());
            connection->CurrentCharId = 0;
            
            New = remove_name(name, connection->Account->characters);
            STRFREE(connection->Account->characters);
            connection->Account->characters = STRALLOC(New);
            DISPOSE(New);
            write_account_data(connection->Account);
            
            connection->WriteMainMenu();
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            break;
         }
         
         if ( !connection->GetCharacter() )
         {
            sprintf( log_buf, "Bad player file %s@%s <%s>", name, connection->GetHost(), connection->Account->email);
            log_string( log_buf );
            connection->SendText( "Your playerfile is corrupt... Please notify darkstone-imms@bb13.betterbox.net .\r\n");
            this->RemoveSocket(connection, false);
         }
         
         /* Check to see if the character's account name matches that
         of the account.  If not, no go. */
         if ( ch->pcdata && ch->pcdata->email && ch->pcdata->email[0] != '\0' )
         {
            if ( str_cmp(ch->pcdata->email, connection->Account->email) ) { /* if no match */
               char* New;
               
               connection->SendText("\r\n");
               connection->SendText("The account name for that character and yours do not match.\r\n");
               connection->SendText("Removing that character from your list.\r\n\r\n");
               log_string("account email and player email do not match - rejecting");
               
               New = remove_name(ch->getName().c_str(), connection->Account->characters);
               STRFREE(connection->Account->characters);
               connection->Account->characters = STRALLOC(New);
               DISPOSE(New);
               write_account_data(connection->Account);
               
               connection->GetCharacter()->SetConnection( NULL );

               connection->CurrentCharId = 0;
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               connection->WriteMainMenu();
               break;
            }
         }
         else
         {
            if ( ch->pcdata )
            {
               if ( ch->pcdata->email )
                  DISPOSE(ch->pcdata->email);
               ch->pcdata->email = str_dup(connection->Account->email);
            }
         }
         
         
         
         if ( IS_SET(ch->act, PLR_DENY) ) /* TODO: check for ban in account file */
         {
            sprintf( log_buf, "Denying access to %s@%s.", name, connection->GetHost() );
            log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
            
            if (connection->newstate != 0)
            {
               connection->SendText("That name is already taken.  Please choose another: ");
               connection->ConnectedState = CON_GET_NEW_NAME;
               return;
            }
            
            connection->SendText("You are denied access.\n\r");
            this->RemoveSocket(connection, false);
            return;
         }
         
         try {
            chk = CheckReconnect( connection, ch->getName().c_str(), false );
         } catch (eAlreadyPlaying) {
            return;
         }
         
         if ( chk ) {
            fOld = true;
         } else {
            if ( wizlock && !IS_SET(connection->Account->flags, ACCOUNT_IMMORTAL) )
            {
               connection->SendText( "The game is wizlocked.  Only immortals can connect now.\n\r" );
               connection->SendText( "Please try back later.\n\r" );
               this->RemoveSocket(socket, false);
               return;
            }
            
            if ( !IS_SET(connection->Account->flags, ACCOUNT_CANMPLAY) && connection->Account->iref > 1 ) {
               connection->SendText(
                  "\r\n"
                  "Someone is already connected from this account.\r\n"
                  "If you have recently connected, try waiting a few minutes before trying again."
                  "\r\n\r\n");
               connection->WriteMainMenu();
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               return;
            }
         }
         
         
         if ( CheckPlaying( connection, ch->getName().c_str(), true ) )
            return;
         
         try {
            chk = CheckReconnect( connection, ch->getName().c_str(), true );
         } catch (eAlreadyPlaying) {
            connection->GetCharacter()->SetConnection( NULL );
            this->RemoveSocket(connection, false);
            return;
         }
         
         if ( chk == true )
            return;
         
         strcpy( buf, ch->getName().c_str() );
         connection->GetCharacter()->SetConnection( NULL );

         free_char( connection->GetCharacter() );
         fOld = load_char_obj( connection, buf, FALSE );
         
         connection->ShowMotd();
         connection->ConnectedState = CON_READ_MOTD;
         
         break;
      }
      /*case CON_GET_ADD_NAME:
      {
            if ( line.length() == 0 ) {
               connection->SendText("\r\n");
               connection->WriteMainMenu();
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               break;
            }
            
            if ( is_name(line.c_str(), connection->Account->characters) ) {
               connection->SendText("\r\nThat character is already on your account.\r\n\r\n");
               connection->WriteMainMenu();
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               break;
            }
            
            if ( !CheckParseName(line.c_str(), connection) ) {
               return;
            }
            
            switch ( CheckPlaying(connection, line.c_str(), FALSE) ) {
               case false: // this should be berr
                  connection->SendText("\r\nName: ");
                  return;

               case true:
                  connection->SendText("\r\nThat character is playing on the MUD as we type.");
                  connection->SendText("\r\n\r\n");
                  connection->WriteMainMenu();
                  connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
                  return;
            }
            
            fOld = load_char_obj(connection, line.c_str(), TRUE);
            ch = connection->GetCharacter();
            
            if ( !fOld ) {
               connection->SendText("\r\nNo such player exists.\r\n");
               connection->SendText("\r\n");
               connection->WriteMainMenu();
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               break;
            }
            
            // Check to see if the character's account name matches that
            //of the account.  If not, no go.  This has to be after the char
            //is fully loaded, so the pcdata is loaded. :(
            if ( ch->pcdata && ch->pcdata->email && ch->pcdata->email[0] != '\0' ) {
               if ( strcmp(ch->pcdata->email, connection->Account->email) ) { // if no match
                  connection->SendText("\r\n");
                  connection->SendText("The account name for that character and yours do not match.\r\n\r\n");
                  gTheWorld->LogCommString("account email and player email do not match - rejecting");
                  
                  connection->GetCharacter()->SetConnection( NULL );
                  connection->CurrentCharId = 0;
                  connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
                  connection->WriteMainMenu();
                  break;
               }
            }
            
            connection->SendText("\r\nPlease enter that character's password: ");
            connection->SendText( (const char *) echo_off_str);
            connection->ConnectedState = CON_GET_ADD_PASS;
            break;
      }
      case CON_GET_ADD_PASS:
      {
            if ( line.length() == 0 ) {
               connection->GetCharacter()->SetConnection( NULL );

               connection->CurrentCharId = 0;
               connection->SendText( (const char *) echo_on_str);
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               connection->WriteMainMenu();
               break;
            }
            
            if ( strcmp(crypt(line.c_str(), connection->GetCharacter()->pcdata->pwd), connection->GetCharacter()->pcdata->pwd) )
            {
               connection->SendText( (const char *) echo_on_str);
               connection->SendText("\r\nWrong password.\r\n\r\n");
               connection->WriteMainMenu();
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               break;
            }
            
            connection->SendText("\r\nReenter password to confirm: ");
            
            connection->ConnectedState = CON_CONFIRM_ADD_PASS;
            break;
      }
         
      case CON_CONFIRM_ADD_PASS:
      {
            if ( line.length() == 0 ) {
               connection->GetCharacter()->SetConnection( NULL );

               connection->CurrentCharId = 0;
               
               connection->SendText( (const char *) echo_on_str);
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               connection->WriteMainMenu();
               break;
            }
            
            if ( strcmp(crypt(line.c_str(), connection->GetCharacter()->pcdata->pwd), connection->GetCharacter()->pcdata->pwd) )
            {
               connection->SendText((const char *) echo_on_str);
               connection->SendText("\r\nWrong password.\r\n\r\n");
               connection->WriteMainMenu();
               connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
               break;
            }
            
            connection->SendText((const char *) echo_on_str);
            connection->SendText("\r\nAdding character to account.\r\n\r\n");
            
        
            
            sprintf(buf, "%s %s", connection->Account->characters, ch->getName().c_str());
            STRFREE(connection->Account->characters);
            connection->Account->characters = STRALLOC(buf);
            write_account_data(connection->Account);
            
            //save_char_obj(ch);
            connection->GetCharacter()->SetConnection( NULL );
            connection->CurrentCharId = 0;
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            connection->SendText((const char *) echo_on_str);
            connection->WriteMainMenu();
            break;
      }
         */
      case CON_CONFIRM_NEW_NAME:
      {
         switch ( line.c_str()[0] )
         {
            case 'y':
            case 'Y':
               connection->SendText(
                  "\r\n"
                  "It is now time to create your character.  You will be asked a\r\n"
                  "series of questions, each of which will help fine tune your\r\n"
                  "character.\r\n"
                  "\r\n"
                  "Would you like your character to be male or female? (M/F) ");
               connection->ConnectedState = CON_GET_NEW_SEX;
               break;
            case 'n':
            case 'N':
               connection->SendText( "\r\nWhat name would you like to be known by? " );
               /* clear descriptor pointer to get rid of bug message in log */
               connection->GetCharacter()->SetConnection(NULL);
               free_char( connection->GetCharacter() );
               connection->CurrentCharId = 0;
               connection->ConnectedState = CON_GET_NEW_NAME;
               break;
            default:
               connection->SendText( "\r\nPlease type Yes or No: " );
               break;
         }
         break;
      }
         
      case CON_GET_NEW_PASSWORD:
      {
         const char * p;
         const char * pwdnew;
         connection->SendText( "\n\r" );
            
         if ( line.length() < 5 )
         {
            connection->SendText( "Password must be at least five characters long.\n\rPassword: " );
            return;
         }
            
         pwdnew = crypt( line.c_str(), connection->Account->email );
         
         for ( p = pwdnew; *p != '\0'; p++ )
         {
            if ( *p == '~' )
            {
               connection->SendText( 
                  "New password not acceptable, try again.\n\rPassword: ");
               return;
            }
         }
            
         connection->Account->password_ = pwdnew;
         connection->SendText( "\n\rPlease retype the password to confirm: " );
         connection->ConnectedState = CON_CONFIRM_NEW_PASSWORD;
         break;
      }
         
      case CON_CONFIRM_NEW_PASSWORD:
      {
            connection->SendText( "\n\r" );
            
            if ( strcmp( crypt( line.c_str(), connection->Account->password_.c_str() ), connection->Account->password_.c_str() ) )
            {
               connection->SendText( "Passwords don't match.\n\rRetype password: " );
               connection->ConnectedState = CON_GET_NEW_PASSWORD;
               return;
            }
            
            write_account_data(connection->Account);
            connection->SendText( (const char *) echo_on_str );
            connection->WriteMainMenu();
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            break;
      }
         
      case CON_GET_NEW_SEX:
      {
         int i, iRace;
         char c = line.c_str()[0];
            
         switch ( c )
         {
            case 'm':
            case 'M':
               ch->sex = SEX_MALE;
               break;
            case 'f':
            case 'F':
               ch->sex = SEX_FEMALE;
               break;
            default:
               connection->SendText( "That's not a sex.\n\rWhat IS your sex? " );
               return;
         }
            
         connection->SendText( 
            "\r\n"
            "Next you must choose your character's race.  Your character's race\r\n"
            "can affect many parts of your character, including its fighting\r\n"
            "ability, how well it performs at magical tasks, and other important\r\n"
            "stats.  The classes available to you is affected by your race, as well.\r\n"
            "\r\n"
            "You can choose from one of the following races.  To learn more about\r\n"
            "any of the races listed below, type help [race].\r\n"
            "\r\n");
            
         for ( i = 0, iRace = 0; iRace < MAX_RACE; iRace++ )
         {
            if ( iRace != RACE_VAMPIRE
               && race_table[iRace].race_name && race_table[iRace].race_name[0] != '\0')
            {
               char buf[MAX_STRING_LENGTH];
               sprintf(buf, "%-15s", race_table[iRace].race_name);
               connection->SendText(buf);
                  
               if ( (++i % 5) == 0 && i != 0 )
                  connection->SendText("\r\n");
            }
         } 
            
         if ( (i % 5) != 0 )
            connection->SendText("\r\n");
            
         connection->SendText("\r\nWhich race would you like for your character? ");
         connection->ConnectedState = CON_GET_NEW_RACE;
         break;
      }
      case CON_GET_NEW_CLASS:
      {
         short iClass;
         const char * argument = line.c_str();
         char arg[MAX_STRING_LENGTH];
         argument = one_argument((char *) argument, arg);
            
         if (!str_cmp(arg, "help"))
         {
            for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
            {
               if ( toupper(argument[0]) == toupper(class_table[iClass]->whoName_.c_str()[0])
                  &&    !str_prefix( argument, class_table[iClass]->whoName_.c_str() ) )
               {
                  do_help(ch, (char *) argument);
                  connection->SendText( "Please choose a class: " );
                  return;
               }
            }  
            connection->SendText( "No such help topic.  Please choose a class: " );
            return;
         }
            
         for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
         {
            if ( toupper(arg[0]) == toupper(class_table[iClass]->whoName_.c_str()[0])
               &&    !str_prefix( arg, class_table[iClass]->whoName_.c_str() ) )
            {
               ch->Class =  iClass;
               break;
            }
         }
            
         if ( iClass == MAX_CLASS || IS_SET(race_table[ch->race].class_restriction, 1<<iClass))
         {
            connection->SendText( "That is not a valid class.  Which class would you like to play? " );
            return;
         }
            
         connection->SendText( "\r\nThis MUD offers ANSI color support, but it is not required."
            "\r\nMost programs can handle these, and you should probably choose"
            "\r\nANSI below (unless you don't want color).  If you don't want"
            "\r\ncolors any more, you can always type 'ansi OFF'."
            "\r\n"
            "\r\nAnsi, or No color support? (A/N) ");
         connection->ConnectedState = CON_GET_WANT_RIPANSI;
         break;
      }
         
      case CON_GET_NEW_RACE:
      {
            short i, iRace;
            const char * argument = line.c_str();
            char arg[MAX_STRING_LENGTH];
            argument = one_argument((char *) argument, arg);
            if (!str_cmp( arg, "help") )
            {
               for ( iRace = 0; iRace < MAX_RACE; iRace++ )
               {
                  if ( toupper(argument[0]) == toupper(race_table[iRace].race_name[0])
                     &&   !str_prefix( argument, race_table[iRace].race_name) )
                  {
                     do_help(ch, (char *) argument);
                     connection->SendText( "Please choose a race: ");
                     return;
                  }
               }
               connection->SendText( "No help on that topic.  Please choose a race: " );
               return;
            }
            
            
            for ( iRace = 0; iRace < MAX_RACE; iRace++ )
            {
               if ( toupper(arg[0]) == toupper(race_table[iRace].race_name[0])
                  &&    !str_prefix( arg, race_table[iRace].race_name ) )
               {
                  ch->race = iRace;
                  break;
               }
            }
            
            if ( iRace == MAX_RACE
               ||   !race_table[iRace].race_name || race_table[iRace].race_name[0] == '\0'
               ||    iRace == RACE_VAMPIRE )
            {
               connection->SendText( 
                  "That's not a race.\n\rWhat IS your race? " );
               return;
            }
            
            
            //buf[0] = '\0';
            connection->SendText("\r\n");
            short iClass;
            
            for ( i = 0, iClass = 0; iClass < MAX_CLASS; iClass++ )
            {
               if ( !IS_SET(race_table[ch->race].class_restriction, 1<<iClass) )
               {
                  char buf[MAX_STRING_LENGTH];
                  sprintf(buf, "%-15s", class_table[iClass]->whoName_.c_str());
                  connection->SendText(buf);
                  
                  if ( (++i % 5) == 0 && i != 0 )
                     connection->SendText("\r\n");
               }
            }
            
            if ( (i%5) != 0 )
               connection->SendText( "\r\n");
            
            connection->SendText("\r\nWhich class would you like play? ");
            connection->ConnectedState = CON_GET_NEW_CLASS;
            break;
            
      }
         
      case CON_GET_WANT_RIPANSI:
      {
         char c = line.c_str()[0];
         switch ( c )
         {
            case 'a':
            case 'A':
               SET_BIT(ch->act,PLR_ANSI);
               break;
            case 'n':
            case 'N':
               break;
            default:
               connection->SendText( "Invalid selection.\n\r[A]nsi or [N]one? " );
               return;
         }
            
         if ( ch->pcdata->email )
            DISPOSE(ch->pcdata->email);
         ch->pcdata->email = str_dup(connection->Account->email);       
            
         /* Save the new name to the account file */
         sprintf(buf, "%s %s", connection->Account->characters, ch->getName().c_str());
         STRFREE(connection->Account->characters);
         connection->Account->characters = STRALLOC(buf);
         
                        connection->Account->allCharacters_.insert(ch->getName().str());
            
         write_account_data(connection->Account);
            
         sprintf( log_buf, "%s@%s new %s %s.", ch->getName().c_str(), connection->GetHost(),
            race_table[ch->race].race_name,
            class_table[ch->Class]->whoName_.c_str() );
         log_string_plus( log_buf, LOG_COMM, sysdata.log_level);
         to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
         connection->SendText( "Press [ENTER] " );
         reset_colors(connection->GetCharacter());
         connection->ShowMotd();
         ch->level = 0;
         ch->position = POS_STANDING;
         break;
      }
         
      case CON_READ_MOTD:
      {
         connection->SendText( "\n\rWelcome to Legends of the Darkstone...\n\r" );
         ch = connection->GetCharacter();
         add_char( ch );
         connection->ConnectedState   = CON_PLAYING;
         
         if ( ch->level == 0 )
         {
            int iLang;
            
            ch->pcdata->clanName_ = "";
            ch->pcdata->clan     = NULL;
            
            switch ( class_table[ch->Class]->attr_prime )
            {
            case APPLY_STR: ch->perm_str = 16; break;
            case APPLY_INT: ch->perm_int = 16; break;
            case APPLY_WIS: ch->perm_wis = 16; break;
            case APPLY_DEX: ch->perm_dex = 16; break;
            case APPLY_CON: ch->perm_con = 16; break;
            case APPLY_CHA: ch->perm_cha = 16; break;
            case APPLY_LCK: ch->perm_lck = 16; break;
            }
            
            ch->perm_str    += race_table[ch->race].str_plus;
            ch->perm_int    += race_table[ch->race].int_plus;
            ch->perm_wis    += race_table[ch->race].wis_plus;
            ch->perm_dex    += race_table[ch->race].dex_plus;
            ch->perm_con    += race_table[ch->race].con_plus;
            ch->perm_cha    += race_table[ch->race].cha_plus;
            ch->affected_by   = race_table[ch->race].affected;
            ch->perm_lck    += race_table[ch->race].lck_plus;
            
            if ( (iLang = skill_lookup( "common" )) < 0 ) {
               bug( "Nanny: cannot find common language." );
            } else {
               ch->pcdata->learned[iLang] = 100;
            }
            
            for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ ) {
               if ( lang_array[iLang] == race_table[ch->race].language ) {
                  break;
               }
            }
            if ( lang_array[iLang] == LANG_UNKNOWN ) {
               bug( "Nanny: invalid racial language." );
            } else {
               if ( (iLang = skill_lookup( lang_names[iLang] )) < 0 ) {
                  bug( "Nanny: cannot find racial language." );
               } else {
                  ch->pcdata->learned[iLang] = 100;
               }
            }
            
            /* ch->resist         += race_table[ch->race].resist;    drats */
            /* ch->susceptible      += race_table[ch->race].suscept;    drats */
            
            name_stamp_stats( ch );
            
            ch->level   = 1;
            ch->exp    = 0;
            ch->hit    = ch->max_hit;
            ch->mana   = ch->max_mana;
            ch->hit    += race_table[ch->race].hit;
            ch->mana   += race_table[ch->race].mana;
            ch->move   = ch->max_move;
            sprintf( buf, "the %s",
               title_table [ch->Class] [ch->level]
               [ch->sex == SEX_FEMALE ? 1 : 0] );
            set_title( ch, buf );
            
            /* Added by Narn.  Start new characters with autoexit and autgold
            already turned on.   Very few people don't use those. */
            SET_BIT( ch->act, PLR_AUTOGOLD ); 
            SET_BIT( ch->act, PLR_AUTOEXIT ); 
            
            /* New players have to earn eq
            obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
            obj_to_char( obj, ch );
            equip_char( ch, obj, WEAR_LIGHT );
            
              obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
              obj_to_char( obj, ch );
              equip_char( ch, obj, WEAR_BODY );
              
               obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
               obj_to_char( obj, ch );
               equip_char( ch, obj, WEAR_SHIELD );
               
                 obj = create_object( get_obj_index(class_table[ch->Class]->weapon), 0 );
                 obj_to_char( obj, ch );
                 equip_char( ch, obj, WEAR_WIELD );
            */
            
            if (!sysdata.WAIT_FOR_AUTH) {
               char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
            } else {
               char_to_room( ch, get_room_index( ROOM_AUTH_START ) );
               ch->pcdata->auth_state = 0;
               SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
            }
            
            /* Display_prompt interprets blank as default */
            ch->pcdata->prompt_ = "";
         }
         else if ( !IS_IMMORTAL(ch) && ch->pcdata->secReleaseDate > secCurrentTime )
         {
            char_to_room( ch, get_room_index(358) );
         }
         else if ( ch->GetInRoom() && ( IS_IMMORTAL( ch ) 
            || !IS_SET( ch->GetInRoom()->room_flags, ROOM_PROTOTYPE ) ) )
         {
            char_to_room( ch, ch->GetInRoom() );
         }
         else if ( IS_IMMORTAL(ch) )
         {
            char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
         }
         else
         {
            char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
         }
         
         if ( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
         {
            remove_timer( ch, TIMER_SHOVEDRAG );
         }

         ch = connection->GetCharacter();
         if ( connection->User.length() > 0 )
         {
            sprintf( log_buf, "%s@%s(%s) has connected.\n\rRoom: [%s]  %s's account is: %s.",
            ch->getName().c_str(), connection->GetHost(), connection->User.c_str(), vnum_to_dotted(ch->GetInRoom()->vnum),
            ch->getName().c_str(), connection->Account->email );
         }
         else
         {
            sprintf( log_buf, "%s@%s has connected.\n\rRoom: [%s]  %s's account is: %s.",
            ch->getName().c_str(), connection->GetHost(), vnum_to_dotted(ch->GetInRoom()->vnum),
            ch->getName().c_str(), connection->Account->email );
         }
         
         if ( ch->level < sysdata.log_level )
         {
            /*to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );*/
            log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
         }
         else
         {
            log_string_plus( log_buf, LOG_COMM, ch->level );
         }
         
         save_char_obj(ch);

         // Redirect character input
         connection->SetInputReceiver(ch);
         act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
         do_look( ch, "auto" );
         mail_count(ch);
         ch->SendPrompt();

         add_timer( ch, TIMER_LOGON, 11, NULL, 0 ); /*prevents fast character switches to curb multiplaying.*/

         break;
      }
      case CON_DELETE_PROMPT:
         if ( !str_cmp(crypt(line.c_str(), connection->Account->password_.c_str()), connection->Account->password_.c_str()) ) {
            send_to_char((const char *) echo_on_str, ch);
            send_to_char("\r\n\r\nPassword correct.\r\n\r\n", ch);
            send_to_char("Please enter your password again, or anything else to cancel.\r\n\r\n", ch);
            send_to_char("Enter your password ==>  ", ch);
            send_to_char((const char *) echo_off_str, ch);
            connection->ConnectedState = CON_DELETE_PROMPT_2;
         } else {
            send_to_char((const char *) echo_on_str, ch);
            send_to_char("\r\nPassword incorrect.\r\n", ch);
            connection->ConnectedState = CON_PLAYING;
            // Reset the input receiver.
            connection->SetInputReceiver(connection->GetCharacter());
         }
         break;
      case CON_DELETE_PROMPT_2:
         send_to_char((const char *) echo_on_str, ch);
         if ( !str_cmp(crypt(line.c_str(), connection->Account->password_.c_str()), connection->Account->password_.c_str()) ) {
            char old[MAX_STRING_LENGTH];
            char New[MAX_STRING_LENGTH];
            
            sprintf(log_buf, "Character %s self-deleted.", ch->getName().c_str());
            log_string(log_buf);
            
            send_to_char("\r\n\r\nCharacter deleted.\r\n\r\n", ch);
            
            strcpy(New, remove_name(ch->getName().c_str(), ch->GetConnection()->Account->characters));
            STRFREE(ch->GetConnection()->Account->characters);
            ch->GetConnection()->Account->characters = STRALLOC(New);
            write_account_data(ch->GetConnection()->Account);
            
            sprintf(old, "%s%c/%s", PLAYER_DIR, tolower(ch->getName().c_str()[0]), capitalize(ch->getName().c_str()));
            sprintf(New, "%s%c/%s", BACKUP_DIR, tolower(ch->getName().c_str()[0]), capitalize(ch->getName().c_str()));
            
            ch->SetConnection( NULL );
            quitting_char = ch;
            save_char_obj(ch);
            saving_char = NULL;
            extract_char(ch, TRUE);
            
            rename(old, New);
            
            connection->CurrentCharId = 0;
            connection->ConnectedState = CON_GET_MAIN_MENU_CHOICE;
            connection->WriteMainMenu();
            
         } else {
            send_to_char("\r\nPassword incorrect.\r\n", ch);
            connection->ConnectedState = CON_PLAYING;

            // Reset the input receiver.
            connection->SetInputReceiver(connection->GetCharacter());
         }
         break;
   }
   return;
}
