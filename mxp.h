

/* MXP Support Module
 *
 * Thanks go to Nick Gammon for providing code!
 *
 */

#ifndef __MXP_H_
#define __MXP_H_

#include "mud.h"

/* strings */

#define MXP_BEG "\x03"    /* becomes < */
#define MXP_END "\x04"    /* becomes > */
#define MXP_AMP "\x05"    /* becomes & */

/* characters */

#define MXP_BEGc '\x03'    /* becomes < */
#define MXP_ENDc '\x04'    /* becomes > */
#define MXP_AMPc '\x05'    /* becomes & */

/* constructs an MXP tag with < and > around it */

#define MXPTAG(arg) MXP_BEG arg MXP_END

#define ESC "\x1B"  /* esc character */

#define MXPMODE(arg) ESC "[" #arg "z"

#define  TELOPT_MXP        '\x5B'

extern const unsigned char echo_on_str[];

extern const unsigned char will_mxp_str[];
extern const unsigned char start_mxp_str[];
extern const unsigned char do_mxp_str    [];
extern const unsigned char dont_mxp_str  [];


int count_mxp_tags (const int bMXP, const char *txt, int length);
void convert_mxp_tags (const int bMXP, char * dest, const char *src, int length);

// moved to connection
//void turn_on_mxp (cPlayerConnection *d);


#endif


