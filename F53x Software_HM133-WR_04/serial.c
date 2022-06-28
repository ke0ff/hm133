/*************************************************************************
 *********** COPYRIGHT (c) 2020 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: serial.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the serial I/O module for the F360 MCU
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    03-03-21 jmh:  Removed "percnt" from ISR as the result of the disambigution of the "%" response quoting character.
 *
 *    08-23-20 jmh:  modified to support bluetooth interface
 *    05-12-13 jmh:  creation date
 *
 *******************************************************************/

#include "c8051F520.h"
#include "typedef.h"
#include "init.h"
//#include "stdio.h"
#include "serial.h"

//------------------------------------------------------------------------------
// local defines
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Variable Declarations
//-----------------------------------------------------------------------------

#define RXD_ERR 0x01
//#define RXD_CR 0x02					// CR rcvd flag
#define RXD_BS 0x04					// BS rcvd flag
#define RXD_ESC 0x40				// ESC rcvd flag
#define RXD_CHAR 0x80				// CHAR rcvd flag (not used)
#define RXD_BUFF_END 4
/*idata S8	rxd_buff[RXD_BUFF_END];		// rx data buffer
	  U8	rxd_hptr;					// rx buf head ptr = next available buffer input
	  U8	rxd_tptr;					// rx buf tail ptr = next available buffer output
	  U8	rxd_stat;					// rx buff status
	  U8	rxd_crcnt;					// CR counter*/
	  bit	qTI0B;						// UART TI0 reflection (set by interrupt)
//	  bit	siplfl;						// UART ISR ipl init flag
//------------------------------------------------------------------------------
// local fn declarations
//------------------------------------------------------------------------------
char eolchr(char c);
char wait_cmd(void);

//-----------------------------------------------------------------------------
// init_serial() initializes serial port vars
//-----------------------------------------------------------------------------
//
void init_serial(void){
	
	init_buff();
//	getch00();					// init the fns
//	cleanline();
//	gotch00();
//	anych00();
//	gotcr();
	qTI0B = 1;					// UART TI0 reflection (set by interrupt)
}
//
//-----------------------------------------------------------------------------
// init_buff() initializes serial port buffer
//-----------------------------------------------------------------------------
//
void init_buff(void){

//	siplfl = 1;					// set IPL init for ISR
//	rxd_hptr = 0;				// rx buf head ptr
//	rxd_tptr = 0;				// rx buf tail ptr
//	rxd_stat = 0;				// rx buff status
//	rxd_crcnt = 0;				// init cr counter
}
//
//-----------------------------------------------------------------------------
// putch, UART0
//-----------------------------------------------------------------------------
//
// SFR Paged version of putch, no CRLF translation
//
char putch (char c)  {

	// output character
	while(!qTI0B){				// wait for tx buffer to clear
		continue;
	}
	qTI0B = 0;
	SBUF0 = c;
	return (c);
}

//
//-----------------------------------------------------------------------------
// getch00 checks for input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// SFR Paged, waits for a chr and returns
// Processor spends most of its idle time waiting in this fn
/*
char getch00(void)
{
	char c = '\0';		// default to null return

	if(rxd_tptr != rxd_hptr){
		c = rxd_buff[rxd_tptr];				// pull chr and update pointer
		rxd_buff[rxd_tptr++] = '~';			// backfill for debug
		if(rxd_tptr == RXD_BUFF_END){
			rxd_tptr = 0;
		}
	}
	return c;
}
*/
//
//-----------------------------------------------------------------------------
// cleanline() cleans buffer up to and including 1st CR.  
//	Pull CHRS from buffer and discard until first CR is encountered.  Then, exit.
//-----------------------------------------------------------------------------
//
/*void cleanline(void)
{
data	char c;	// temp char

	if(gotch00()){							// skip if buffer empty
		do{
			c = getch00();					// pull chr and update pointer
		}while(gotch00());					// repeat until CR or buffer empty
	}
	rxd_crcnt = 0;
	return;
}
*/
//
//-----------------------------------------------------------------------------
// gotch00 checks for input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no chr in buffer or if current chr is '\r'
/*
char gotch00(void)
{
	char c = 0;

	if((rxd_tptr != rxd_hptr) && (rxd_buff[rxd_tptr] != '\r')){
		c = 1;						// set buffer has data
	}
	if(rxd_stat & RXD_BS){				// process backspace
		rxd_stat &= ~RXD_BS;			// clear flag
		putss("\b \b");					// echo clearing BS to terminal
	}
	return c;
}
*/
//
//-----------------------------------------------------------------------------
// anych00 checks for any input @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no chr in buffer or if current chr is '\r'
/*
char anych00(void)
{
	char c = 0;

	if((rxd_tptr != rxd_hptr) && (rxd_buff[rxd_tptr] != '\0')){
		c = 1;						// set buffer has data
	}
	return c;
}
*/
//
//-----------------------------------------------------------------------------
// gotcr checks for '\r' @ RX0.  If no chr, return '\0'.
//-----------------------------------------------------------------------------
//
// returns 0 if no cr rcvd
/*
char gotcr(void)
{
	char c = 0;

	if(rxd_crcnt){
		c = 1;							// set buffer has data
	}
	return c;
}
*/
//-----------------------------------------------------------------------------
// cpy_str() copies src to dest
//-----------------------------------------------------------------------------
/*
void cpy_str (char* src, char* dest)
{
	char c;

	do{
		c = *src;
		*dest = c;
		src += 1;
		dest += 1;
	}while(c);
	return;
}
*/
//-----------------------------------------------------------------------------
// char hiasc() returns hi nybble as ASCII
//-----------------------------------------------------------------------------

char hiasc (U8 num)
{
	char c;

	c = ((num>>4)& 0x0f) + '0';
	if(c > '9'){
		c += 'A' - '9' - 1;
	}
	return c;
}

//-----------------------------------------------------------------------------
// char lowasc() returns low nybble as ASCII
//-----------------------------------------------------------------------------

char lowasc (U8 num)
{
	char c;

	c = (num & 0x0f) + '0';
	if(c > '9'){
		c += 'A' - '9' - 1;
	}
	return c;
}

//-----------------------------------------------------------------------------
// putss() does puts w/o newline
//-----------------------------------------------------------------------------

void putss (char* string)
{
	while(*string){
		putch(*string++);
	}
	return;
}

//-----------------------------------------------------------------------------
// getss() copies rxd capture to string.  Returns ptr to next empty chr of string
//-----------------------------------------------------------------------------
/*
char* getss (char* string, U8 maxlen)
{
	char* pstr = string;
	char  c;
	U8	  ctr = 0;
	bit	EA_save;

	do{
		c = getch00();
		if(ctr < maxlen){
			*pstr = c;						// only capture the first "maxlen" chrs
			pstr += 1;
			ctr += 1;
		}
	}while(c != '\0');
	if(rxd_crcnt){
		EA_save = EA;					// prohibit intrpts
		EA = 0;
		rxd_crcnt -= 1;					// clear flag
		EA = EA_save;					// re-set intrpt enable
	}
	return *pstr;
}*/

//
//-----------------------------------------------------------------------------
// rxd_intr
//-----------------------------------------------------------------------------
//
// UART intr.  Captures RX data and places into circular buffer
//	For TX, if mode = buffered, the intr will process the TX buffer until empty.
//	To send data, fill the tx buffer, then kick-start the transmit process as follows:
//
//		c = txd_buff[txd_tptr++];		// get 1st chr of buffered data
//		if(txd_tptr >= TXD_BUFF_END){
//			txd_tptr = 0;				// roll-over headptr
//		}
//		SBUF0 = c;						// prime the pump (kick-start the TX output)
//		TI0B = 0;
//
//	If not in buffered mode, use reflection register to pass TXD empty status back to
//	putchar.
//
//
//-----------------------------------------------------------------------------
// uart1_intr
//-----------------------------------------------------------------------------
//
// UART1 rx intr.  Captures RX data and places into circular buffer
//	Uses "trap sentinels" to itentify when to start storing data to the buffer.
//	This allows the respones from the target device to occupy much less buffer space
//	by capturing only a small part of the responses and discarding the remaining characters.
//	Most of the BT responses start with "%".  The "S%,%,#" command sets the terminating
//	character to a "#".  This disambiguates "%" making it only signal the start of a response.
//
//	Three characters plus a terminating null are captured whenever a trap sentinel is detected
//	(includes the trap character).
//

void rxd_intr(void) interrupt 4
{
/* buffer registers repeated here for reference ... !! DO NOT UNCOMMENT !!
S8   rxd_buff[10];					// rx data buffer
U8   rxd_hptr = 0;					// rx buf head ptr
U8   rxd_tptr = 0;					// rx buf tail ptr
U8   rxd_stat = 0;					// rx buff status*/
	
//#define	TRAP_LEN	3				// length of responses to capture
	
volatile	char	c;
//			U8		i;
//	static	U8		ccnt;
//	static	U8		percnt;

//	if(siplfl){
//		ccnt = 0;									// IPL init
//		percnt = 0;
//		rxd_crcnt = 0;
//		siplfl = 0;
//	}
	if(TI0){
		qTI0B = 1;									// set TX reflection flag
		TI0 = 0;
	}
	if(RI0){
		c = SBUF0;
		RI0 = 0;									// clear intr flag
	}
	return;
}
