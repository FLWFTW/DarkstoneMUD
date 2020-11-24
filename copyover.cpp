
#include <stdio.h>
#include <unistd.h>

#include "mud.h"
#include "connection.h"
#include "connection_manager.h"
#include "World.h"

#include "paths.const.h"

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */
void do_copyover(CHAR_DATA *ch, const char* argument)
{
	extern int ListenerDescriptor, ListenerPort;
	
	FILE *fp;
	//DESCRIPTOR_DATA *d, *d_next;
	AREA_DATA *tarea;
	char buf [255], buf2[100], buf3[100];
	
	fp = fopen (COPYOVER_FILE, "w");
	/*	  fpMob = fopen(COPYOVER_DIR "npcs", "w");	*/
	
	if (!fp )
	{
		if ( ch ) {
			send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
			log_printf ("Could not write to copyover file: %s", COPYOVER_FILE);
			perror ("do_copyover:fopen");
			return;
		}
	}/* else if ( !fpMob ) {
	    send_to_char("Npcs file not writeable, aborted.\r\n", ch);
	    log_printf("Could not write to copyover file: %s", COPYOVER_DIR "npcs");
	    perror("do_copyover:fopen");
	    return;
	}*/
	
	fCopyOver = true;
	
	/* Consider changing all saved areas here, if you use OLC */
	
	for ( tarea = first_build; tarea; tarea = tarea->next )
	{
		char buf[MAX_STRING_LENGTH];
		
		if ( !IS_SET(tarea->status, AREA_LOADED) )
			continue;
		
		sprintf(buf, "%s%s", BUILD_DIR, tarea->filename);
		fold_area(tarea, buf, FALSE);
	}
	
	
	{
		if ( ch ) {
			sprintf (buf, "\n\r%s twists the fabric of the universe into a large knot, causing everything to come to a screeching halt.\n\r", ch->getName().c_str());
		} else {
			sprintf(buf, "\r\nSome fool with shell access twists the fabric of the memory chips into a large knot, causing everything to come to a twisting halt.\r\n");
		}
	}
	
	/*	  for ( ich = first_char; ich; ich = ich->next ) {
	if ( IS_NPC(ich) && ich->pIndexData->vnum != 3 )
	fwrite_char(ich, fpMob);
	}
	
	fprintf(fpMob, "\n\n\n#$");*/
	
	/* For each playing descriptor, save its state */
	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		PlayerConnection * d = (PlayerConnection*) SocketMap[*itor];
		if ( !d )
			continue;

		CHAR_DATA * och = d->GetOriginalCharacter();
		//d_next = d->next; /* We delete from the list , so need to save this */
		
		if (!d->GetCharacter() || d->ConnectedState < 0) /* drop those logging on */
		{
			d->SendText("\n\rSorry, we are rebooting. Come back in a few minutes.\n\r");
			gConnectionManager->RemoveSocket(d, false);
			//close_socket (d, FALSE); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->GetDescriptor(), och->getName().c_str(), d->GetHost());
			
#if 0
			if (och->level == 1)
			{
				d->SendText("Since you are level one, and level one characters do not save, you gain a free level!\n\r",
					0);
				
				advance_level (och);
				
				och->level++; /* Advance_level doesn't do that */
			}
#endif
			
			save_char_obj (och);
			d->SendText(buf);
		}
	}
	
	fprintf (fp, "-1\n");
	fclose (fp);

	log_string("Flushing output.");
	gConnectionManager->ForceFlushOutput();
	log_string("Output flushed.");
	
	/* Close reserve and other always-open files and release other resources */
	fclose (fpReserve);
	fclose (fpLOG);
	
	/* exec - descriptors are inherited */
	sprintf (buf, "%d", ListenerPort);
	sprintf (buf2, "%d", ListenerDescriptor);
	sprintf (buf3, "%d", -1/*control2*/);
	
	execl("../src/dark", "dark", buf, "copyover", buf2, buf3,
		(char *) NULL);
	
	fCopyOver = false;
	
	/* Failed - sucessful exec will not return */
	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);
	
	echo_to_all(AT_MAGIC, "Something went fooey, and the universe never recovered, leaving you all tied in a knot.\r\n", ECHOTAR_ALL);
	
	/* Here you might want to reopen fpReserve */
	/* Since I'm a neophyte type guy, I'll assume this is
	a good idea and cut and past from main()  */
	if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
	{
		perror( NULL_FILE );
		exit( 1 );
	}
	
	if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
	{
		perror( NULL_FILE );
		exit( 1 );
	}
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
	//DESCRIPTOR_DATA *d;
	FILE *fp;
	char name [100];
	char host[MAX_STRING_LENGTH];
	int desc;
	bool fOld;
	
	log_string ("Copyover recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
		log_string("Warning: Copyover file not found. Aborting copyover_recover.\n\r");
		return;
	}
	
	/* copyover_reload_npcs(); */
	
	unlink (COPYOVER_FILE); /* In case something crashes
	- doesn't prevent reading */
	for (;;)
	{
		int retval = fscanf (fp, "%d %s %s\n", &desc, name, host);

                if (retval == EOF)
                {
                    log_string("Copyover file corrupted; halting recovery.\n\r");
                    break;
                }


		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */
		/*if (!write_to_descriptor (desc, "The universe quivers as time begins to flow once more.\n\r", 0))
		{
			close (desc); // nope
			continue;
		}*/
		
		PlayerConnection * d = new PlayerConnection(desc);

		//CREATE(d, DESCRIPTOR_DATA, 1);
		//init_descriptor (d, desc); /* set up various stuff */
		
		d->SetHost(host);
		//d->host = STRALLOC( host );
		
		//LINK( d, first_descriptor, last_descriptor, next, prev );
		d->ConnectedState = CON_COPYOVER_RECOVER; /* negative so close_socket
		will cut them off */
		
		/* Now, find the pfile */
		
		fOld = load_char_obj (d, name, FALSE);
		
		if (!fOld) /* Player file not found?! */
		{
			d->SendText("Somehow you didn't make it through the copyover.  Just reconnect.\r\n");
			gConnectionManager->RemoveSocket(d, false);
			//close_socket (d, FALSE);
		}
		else /* ok! */
		{
			d->Account = load_account_data(d->GetCharacter()->pcdata->email);
			
			send_to_char("The universe returns to some facsimile of normality, but something seems different.\n\r",
				d->GetCharacter());
			
			/* Just In Case,  Someone said this isn't necassary, but _why_
			do we want to dump someone in limbo? */
			if (!d->GetCharacter()->GetInRoom())
				d->GetCharacter()->InRoomId = get_room_index (ROOM_VNUM_TEMPLE)->GetId();
			
			/* Insert in the char_list */
			LINK( d->GetCharacter(), first_char, last_char, next, prev );
			
			char_to_room (d->GetCharacter(), d->GetCharacter()->GetInRoom());
			/*			 do_look (d->character, "auto noprog");
			act (AT_ACTION, "$n materializes!", d->character, NULL, NULL, TO_ROOM);*/
			d->ConnectedState = CON_PLAYING;
		}
		
	}
	if ( !fp ) {
		fprintf(stderr, "fp null?\r\n");
	}
        else {
	    fclose (fp);
        }
}

void copyover_reload_npcs()
{
    char strfile[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    extern FILE *fpArea;
    extern char strArea[MAX_INPUT_LENGTH];

    sprintf(strfile, "%s/npcs", COPYOVER_DIR);

    if ( (fp = fopen(strfile, "r")) ) {
/*        unlink(strfile);*/

    	found = TRUE;
    	/* Cheat so that bug will show line #'s -- Altrag */
	    fpArea = fp;
    	strcpy(strArea, strfile);
    	for ( ; ; )
    	{
	        char letter;
    	    char *word;

            if ( feof(fp) ) {
                break;
            }
            
	        letter = fread_letter( fp );
    	    if ( letter == '*' )
    	    {
        		fread_to_eol( fp );
        		continue;
    	    }

    	    if ( letter != '#' )
	        {
	        	bug( "Load_char_obj: # not found.", 0 );
        		bug( strfile, 0 );
        		break;
    	    }

	        word = fread_word( fp );
    	    if ( !str_cmp( word, "MOB" ) )
	        {
                int vnum;
                MOB_INDEX_DATA *pMID;

                vnum = fread_number(fp);
                pMID = get_mob_index(vnum); 
            
                fprintf(stderr, "Reloading %s\n", vnum_to_dotted(vnum));
                
                if ( !pMID ) {
                    bug("%d: invalid vnum", vnum);
                    break;
                }
                
                ch = create_mobile(pMID);
                
                fread_char(ch, fp, FALSE);
    	    }
	        else if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
            {
                fread_obj( ch, fp, OS_CARRY );
            }
    	    else if ( !str_cmp( word, "END"    ) )	/* Done		*/
            {
                char_to_room(ch, ch->GetInRoom());
                
                continue;
            }
            else if ( !str_cmp(word, "$" ) ) {
                break;
            }
    	    else
    	    {
	        	bug( "Load_char_obj: bad section.", 0 );
        		bug( strfile, 0 );
        		break;
    	    }
    	}
    }

    fclose( fp );
    fpArea = NULL;
    strcpy(strArea, "$");
}
