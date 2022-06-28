/*************************************************************************
 *********** COPYRIGHT (c) 2017 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: bluetooth.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the bluetooth control module for the F530 MCU
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    08-16-20 jmh:  creation date
 *
 *	03-07-21	jmh
 *				Moved response compare strings to file-global space.
 *				Added "S%,%,#" to init sequence.  Establishes a unique "%---#" quoting pair to disambiguate the
 *					start of a response.
 *
 *******************************************************************/

#include "c8051F520.h"
#include "typedef.h"
#include "init.h"
#include "serial.h"
#include "wired_rmt.h"

//------------------------------------------------------------------------------
// local defines
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Variable Declarations
//-----------------------------------------------------------------------------
						
idata	char	buf[10];			// general burpose char buffer

//------------------------------------------------------------------------------
// local fn declarations
//------------------------------------------------------------------------------

void txmsg(char* sptr, U8 key, U8 parm);

//-----------------------------------------------------------------------------
// process_WR() handles wired-remote subsystem
//-----------------------------------------------------------------------------
U8 process_WR(U8 cmd){
idata			char	tbuf[6];		// msg buffer
static data 	U8		bstate;			// process state machine vector
static data 	U8		check_state;	// check button memory
static volatile	U8		klast;			// last key
//			U8		r;				// temp


	if(cmd == STATE_INIT){							// initialize BT
		wr_timeout(0);
		bstate = STATE_0;
		check_state = 0;
		init_buff();								// clean out serial buffer
		klast = '\0';
		return 0xff;
	}
	//
	// wired-remote state machine
	//
	if(cmd == klast) cmd = K_HOLD;
	switch(bstate){
		default:									// error state ... init the machine
		case STATE_0:								// INIT $$$
			putss("#RC900_WRDv0.0%\r");
			wait(20);
			bstate = STATE_IDLE;
			break;

		case STATE_IDLE:							// IDLE state
			switch(cmd){
				default:
					klast = cmd;
					wr_timeout(WR_HOLD_PER);
					wra_timeout(WR_ABRT_PER);
					txmsg(tbuf, klast, 'p');
					putss(tbuf);
					break;

				case K_HOLD:
					if(wr_timeout(0)){
						wr_timeout(WR_HOLD_PER);
						wra_timeout(WR_ABRT_PER);
						txmsg(tbuf, klast, 'h');
						putss(tbuf);
					}
					break;

				case K_REL:
					if(klast){
						txmsg(tbuf, klast, 'r');
						putss(tbuf);
						klast = '\0';
						wr_clr();
					}
					break;

				case K_NULL:
					if(klast){
						if(wra_timeout(0)){
							txmsg(tbuf, klast, 'R');
							putss(tbuf);
							klast = '\0';
							wr_clr();
						}
					}
					break;
			}
			break;
		}
	return 0xff;
}

//
//-----------------------------------------------------------------------------
// txmsg() creates a remote command message
//	sptr is the string ptr, key is the cmd code, and parm is the parameter
//-----------------------------------------------------------------------------

void txmsg(char* sptr, U8 key, U8 parm)
{

	*sptr++ = PREAMB;
	*sptr++ = key;
	*sptr++ = parm;
	*sptr++ = (key ^ parm) | 0x80;
	*sptr++ = EOL;
	*sptr = '\0';
	return;
}
