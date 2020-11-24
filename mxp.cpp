

#include "mud.h"
#include "mxp.h"


/*
* Count number of mxp tags need converting
*    ie. < becomes &lt;
*        > becomes &gt;
*        & becomes &amp;
*/

int count_mxp_tags (const int bMXP, const char *txt, int length)
{
	char c;
	const char * p;
	int count;
	int bInTag = FALSE;
	int bInEntity = FALSE;

	for (p = txt, count = 0; length > 0; p++, length--)
	{
		c = *p;
		
		if (bInTag)  /* in a tag, eg. <send> */
		{
			if (!bMXP)
				count--;	 /* not output if not MXP */   
			if (c == MXP_ENDc)
				bInTag = FALSE;
		} /* end of being inside a tag */
		else if (bInEntity)  /* in a tag, eg. <send> */
		{
			if (!bMXP)
				count--;	 /* not output if not MXP */   
			if (c == ';')
				bInEntity = FALSE;
		} /* end of being inside a tag */
		else
		{
			switch (c)
			{
			case MXP_BEGc:
				bInTag = TRUE;
				if (!bMXP)
					count--;	   /* not output if not MXP */	 
				break;
				
			case MXP_ENDc:	 /* shouldn't get this case */
				if (!bMXP)
					count--;	   /* not output if not MXP */	 
				break;
				
			case MXP_AMPc:
				bInEntity = TRUE;
				if (!bMXP)
					count--;	   /* not output if not MXP */	 
				break;
				
			default:
				if (bMXP)
				{
					switch (c)
					{
					case '<':	  /* < becomes &lt; */
					case '>':	  /* > becomes &gt; */
						count += 3;  
						break;
						
					case '&':
						count += 4;  /* & becomes &amp; */
						break;
						
					case '"':        /* " becomes &quot; */
						count += 5;  
						break;
						
					} /* end of inner switch */
				}	/* end of MXP enabled */
			} /* end of switch on character */
			
		}	/* end of counting special characters */
	}
	
	return count;
} /* end of count_mxp_tags */

void convert_mxp_tags (const int bMXP, char * dest, const char *src, int length)
{
	char c;
	const char * ps;
	char * pd;
	int bInTag = FALSE;
	int bInEntity = FALSE;
	
	for (ps = src, pd = dest; length > 0; ps++, length--)
	{
		c = *ps;
		if (bInTag)  /* in a tag, eg. <send> */
		{
			if (c == MXP_ENDc)
			{
				bInTag = FALSE;
				if (bMXP)
					*pd++ = '>';
			}
			else if (bMXP)
				*pd++ = c;	/* copy tag only in MXP mode */
		} /* end of being inside a tag */
		else if (bInEntity)  /* in a tag, eg. <send> */
		{
			if (bMXP)
				*pd++ = c;	/* copy tag only in MXP mode */
			if (c == ';')
				bInEntity = FALSE;
		} /* end of being inside a tag */
		else
		{
			switch (c)
			{
				case MXP_BEGc:
					bInTag = TRUE;
					if (bMXP)
						*pd++ = '<';
					break;
				
				case MXP_ENDc:	/* shouldn't get this case */
					if (bMXP)
						*pd++ = '>';
					break;
					
				case MXP_AMPc:
					bInEntity = TRUE;
					if (bMXP)
						*pd++ = '&';
					break;
		  
				default:
					if (bMXP)
					{
						switch (c)
						{
						case '<':
							memcpy (pd, "&lt;", 4);
							pd += 4;	  
							break;
							
						case '>':
							memcpy (pd, "&gt;", 4);
							pd += 4;	  
							break;
							
						case '&':
							memcpy (pd, "&amp;", 5);
							pd += 5;	  
							break;
							
						case '"':
							memcpy (pd, "&quot;", 6);
							pd += 6;	  
							break;
							
						default:
							*pd++ = c;
							break;	/* end of default */
							
						} /* end of inner switch */
					}
					else
						*pd++ = c;	/* not MXP - just copy character */
					break;	
					
			} /* end of switch on character */
		}
		
	}	/* end of converting special characters */
} /* end of convert_mxp_tags */


/* set up MXP */
//void turn_on_mxp (PlayerConnection *d)
//{
//	d->EnableMXP();
	/*d->mxp = TRUE;	// turn it on now
	write_to_buffer( d, (const char*) start_mxp_str, 0 );
	write_to_buffer( d, MXPMODE (6), 0 );	// permanent secure mode
	write_to_buffer( d, MXPTAG ("!-- Set up MXP elements --"), 0);
	// Exit tag
	write_to_buffer( d, MXPTAG ("!ELEMENT Ex '<send>' FLAG=RoomExit"), 0);
	// Room description tag
	write_to_buffer( d, MXPTAG ("!ELEMENT rdesc '<p>' FLAG=RoomDesc"), 0);
	// Get an item tag (for things on the ground)
	write_to_buffer( d, MXPTAG 
		("!ELEMENT Get \"<send href='"
		"get &#39;&name;&#39;|"
		"examine &#39;&name;&#39;|"
		"drink &#39;&name;&#39;"
		"' "
		"hint='RH mouse click to use this object|"
		"Get &desc;|"
		"Examine &desc;|"
		"Drink from &desc;"
		"'>\" ATT='name desc'"), 
		0);
	// Drop an item tag (for things in the inventory)
	write_to_buffer( d, MXPTAG 
		("!ELEMENT Drop \"<send href='"
		"drop &#39;&name;&#39;|"
		"examine &#39;&name;&#39;|"
		"look in &#39;&name;&#39;|"
		"wear &#39;&name;&#39;|"
		"eat &#39;&name;&#39;|"
		"drink &#39;&name;&#39;"
		"' "
		"hint='RH mouse click to use this object|"
		"Drop &desc;|"
		"Examine &desc;|"
		"Look inside &desc;|"
		"Wear &desc;|"
		"Eat &desc;|"
		"Drink &desc;"
		"'>\" ATT='name desc'"), 
		0);
	// Bid an item tag (for things in the auction)
	write_to_buffer( d, MXPTAG 
		("!ELEMENT Bid \"<send href='bid &#39;&name;&#39;' "
		"hint='Bid for &desc;'>\" "
		"ATT='name desc'"), 
		0);
	// List an item tag (for things in a shop)
	write_to_buffer( d, MXPTAG 
		("!ELEMENT List \"<send href='buy &#39;&name;&#39;' "
		"hint='Buy &desc;'>\" "
		"ATT='name desc'"), 
		0);
	// Player tag (for who lists, tells etc.)
	write_to_buffer( d, MXPTAG 
		("!ELEMENT Player \"<send href='tell &#39;&name;&#39; ' "
		"hint='Send a message to &name;' prompt>\" "
		"ATT='name'"), 
		0);*/
//} /* end of turn_on_mxp */


