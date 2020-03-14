/****************************************************************************************
 ****************** COPYRIGHT (c) 2019 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: main.c
 *
 *  Module:    Control
 *
 *  Summary:   This is the main code file for the HM133 DTMF application
 *	License and other legal stuff:
 *			   This software, comprised of all files contained in the original distribution archive,
 *				are protected by US Copyright laws.  The files may be used and modified by the person
 *				receiving them under the following terms and conditions:
 *				1) The software, or any protion thereof may not be distributed to any 3rd party by
 *					the recipient or any agent or assign of the recipient.
 *				2) The recipient assumes all risks for the use (or mis-use) of this software.
 *
 *
 *  Project scope revision history:
 *	  *** REV 03 ***
 *    03-10-20 jmh:  Modified pulse() to accept U16 pulse timer (modified pulse timer also).  Rig array
 *						still stores U8, so it must be cast when the rig type is read on power-up.
 *					 Modified ICOM mode to use MR(TUNER) as scan up, and F-1(V/M) as scan down.  These
 *						keys instantiate an 800ms pulse on up or dn to start the scan mode.
 *					 Modified ICOM pulse period to 45 ms (was 40).
 *					 Added named rig types for Kenwood and generic 100 and 125.
 *					 Fixed several code snippets for "VERS == 1" build case.
 *	  *** REV 02 ***
 *    08-28-19 jmh:  Debug of HM151 mode.  Tested on IC-7000.  Found that the first key is ignored if no
 *						other keys are detected after.  This means that the F-2 key needn't be set to a
 *						null command (can't do this anyway).  Also verified that HM-151 keys are active
 *						during TX.
 *					 Modified code to trap leading and falling edge of 'G' key.  If entering pass-thru
 *						mode, delay DMUTE switch until release of 'G', else switch DMUTE immediately.
 *					 Revamped init code to make sure DMUTE is not set to pass-thru until just before the
 *						start of the MPL.
 *					 Added timer clear statements to init code to cover warm start case.
 *	  08-19-19 jmh:  Added pulse delay variable to radio_fbtn[][] array.  Kenwood needs at least 80ms,
 *						ICOM can take 40ms.  Other delays up to 125 are also included.
 *					 HW MOD: changed comparator voltage ref to a voltage divider driven from M8V to
 *						follow what the radios do.
 *					 HW MOD: changed how 5V is wired to reset circuit.  Debug adapter now works at
 *						least as well as with the 3.3V bypass Vreg.
 *	  08-18-19 jmh:  VERS 2 testing continues...
 *					 Tested HM151 mode & corrected LED functions.  *** Need to test with IC-7000.
 *					 Abandoned SPR as a strap, made Ro3 3rd strap.
 *					 *** need to test option straps.
 *	  08-17-19 jmh:  VERS 2 initial bring-up complete.  Issues:
 *						1) The dual FET footprint was wrong.  Had to re-map 4094 bitmap to correct.
 *							LED and DMUTE had to be re-wired, so no SW change for those signals.
 *							PCB rev 003 update complete.  Added Ro3 "gimmick" resistor to make P0.6
 *							the 3rd strap option.  *** Need to see if this works. ***
 *						2) Voltage reg either broke or some configuration issue.
 *							Flaky debug and operation modes - will look again at this on 2nd build.
 *							1st article has a 3V regulator piggy-backed on the 5V Vreg with a jumper to
 *							U2 Vdd net (Vregi pin is lifted).  U5 Vhi is about 2.9V...not sure this
 *							can be counted on to work all the time.
 *					 Made some code tweaks to get DTMFs to work correctly.  Not sure why they broke going
 *						to rev 002 HW.  Also, cleaned up some commented-out code (still some left).
 *					 Added "mic present" mirror.  If strap selects HM151, DN0 = MIC_DET_N any time MIC_DET_N
 *						changes state.  In addition to mirror, this code kills PTT and tone related registers
 *						and clears DMUTE if the mic is removed.
 *					 	Added restart capability to re-initialize system when mic is removed.  Code will
 *						continually restart until mic is re-connected.
 *					 Resticted all keys but "M" in HM151 mode.  DTMFs will generate if PTT pressed & DMUTE
 *						is active.
 *	  08-15-19 jmh:  HM133 DTMF now works (mostly).  PTT can NOT be pressed if HM133 DTMF-S is active.
 *						May not need HM133/HM151 strap-opt.
 *					 Strap support for revA *mostly* working.
 *					 1st key logic now traps "1st" 1st key and ignores following until button released.
 *	  08-12-19 jmh:  Added support for straps.  radio_fbtn[][] array holds transistor pulse patterns that
 *						correspond to key/rig combinations.
 *	  				 Added support for extended buttons (MC-44/MH-36) in the button operations switch{} and
 *	  				 	in the support Fns.
 *	  				 Changed pulse_up/pulse_dn to a single funtion which now supports all of the transistor
 *						outputs of the CD4094 SR port expander.
 *					 VERS 1 now support using DMUTE for a single expansion button (since it won't do the
 *						HM151 FT option correctly).
 *					 Added support for HM151 FT mode.  Fn LED = off if in FT mode, else, on for DTMF enabled mode.
 *    08-09-19 jmh:  Added #if directives to support PCB version 2.  Re-mapped PTT, UP, and DN bit controls
 *						to use a central Fn to set/clear bits.  Paves the way for port extender to be used
 *						in version 2 PCB.  bbSPI code is in place for port expander.  Waveforms verified on
 *						o'scope, but not tested.  Port update uses TIMER0 ISR to clock 8 bits and strobe
 *						CD4094 port expander I.C..  XFR plus STB takes about 45us.
 *	  08-07-19 jmh:  Added feature to interrupt U/D macros on any keypress
 *	  				 Added feature to change Fn LED brightness.  If no PTT, "*" = dim, "#" = brt.
 *
 *    08-04-19 jmh:  basic functionality working with HM-151.  HM-133 DTMFs need work.
 *					 added a DTMF activity timer for HM-151 so that the MIC is unmuted
 *						after an interval of no DTMF key being pressed.
 *					 The "Fn LED" now depicts mic mute status during tone cycles.  Off =
 *						muted (or macro digit pressed), ON = unmuted.  Blink = up/dn pulse.
 *
 *	  *** REV 01 ***
 *	  08-03-19 jmh:  project baselined at rev 01
 *    07-20-19 jmh:  Project origin, copied from Orion PLL
 *
 ***************************************************************************************/

/****************************************************************************************
 *  File scope revision history:
 *  04-09-16 jmh:	creation date
 *
 ***************************************************************************************/

//--------------------------------------------------------------------------------------
// main.c
//
//      The following resources of the F531 are used:
//      24.500 MHz internal osc
//
//      UART: n/u
//
//      Timer0: n/u
//      Timer1: DDS sample driver
//      Timer2: Application timer (1ms/tic)
//
//      ADC: n/u
//
//      PCA: 
//			 CEX0 = ICOM mic data decode ISR
//			 CEX1 = DDS PWM ISR
//			 CEX2 = Aux LED PWM out (Fixed PWM ratio, no ISR)
//
//      SYSTEM NOTES:
//
//		This project adapts an HM-133 (or HM-151) microphone so that it can produce DTMF signals.
//		The PCB features the F531 MCU, a 3.3V regulator, open-drain drivers for up/dn and PTT, and
//		switching circuits for the microphone/DTMF signals.  The small form-factor board is intended
//		to be an in-line module between the microphone and the transciever.  Additional features
//		such as up/down toggle commands are also possible allowing a specific number of pulses (up or
//		dn) to be issued to the connected radio.  DTMF memories are also possible using spare FLASH
//		to hold the sequences.
//
//		DTMF signals are generated using DDS code running the PCA CEX1 output in 8-bit PWM mode.  If
//		a keypress is recieved while the PTT is active (from either the PTT input discrete or the PTT
//		keycode), the system will turn on the appropriate DTMF pair until the key is released, or PTT
//		goes inactive.  Since PCA can't do an 8-bit regular interrupt, Timer1 is used to drive the sample
//		clock.  Thus, the DDS ISR code lives inside Timer1, and updates the PWM register.  Timer1 needs to
//		run at the same rate as the PCA to preserve update alignment.
//
//		The DDS is currently being driven at FSAMP = 23,925 Hz to ease the filtering requirements.  This
//		increases the Fstep to 0.365Hz, but this is still well below the worst-case 10.4Hz ETSI requirement
//		for DTMF.
//
//		PCA CEX2 is used to drive an "AUX" LED on the HM-133 (typically blue, and is user installed near
//		the u/d buttons on the HM-133/151).  It is a fixed PWM output to provide a fine adjustment to the
//		LED brightness.  LED on/off/brightness can be used to convey status (power on, DTMF output, PGM
//		mode enabled, etc...).
//
//		The software also interprets HM-133 key codes to drive the PTT and U/D open-drain discretes.
//		U/D also features a tiered pulse ramp whereby the initial keypress sends one pulse, then after
//		1 second of holding, the system begins pulsing at 5 Hz.  After 3 seconds of keypress, the
//		system begins pulsing at 10 Hz.  Pulsing stops imeediately upon release of the key.
//
//--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
// compile defines
#include "init.h"
#include "typedef.h"
#include "c8051F520.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------


#define	VERS	2				// PCB vers 2 uses 4094 port expander, else VERS = 1

//#define	IS_SIM				// enable this define if using simulator

//  see init.h for #defines


//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------

extern code const U8	SINE[];
volatile	U16		hmd_timer;						// HM151 data idle timeout timer

//-----------------------------------------------------------------------------
// Main Variables
//-----------------------------------------------------------------------------

// port assignments

// PCB version 1 bit defines
#if	VERS == 1
sbit MDATA		= P1^7;				// (i) mic data in
sbit MMUTE_N	= P1^6;				// (o) mic mute out (act low)
sbit PTTb		= P1^5;				// (o) PTT out to rig (act hi)
sbit DNb	    = P1^4;				// (o) mic DN button out to rig (act hi)
sbit UPb	    = P1^3;				// (o) mic UP button out to rig (act hi)
sbit PTTM_N     = P1^1;				// (i) mic PTT in (act low)
sbit DMUTE		= P1^0;				// (o) rig data mute out (act hi)
#endif
// PCB version 2 bbSPI defines
#if	VERS == 2
sbit MDATA		= P1^7;				// (a) comparator (-) input (mic serial data)
sbit STB		= P1^6;				// (o) port expander serial strobe
sbit MIC_DET_N    = P1^5;				// (i) HM133 mic present detect
sbit SCLK       = P1^4;				// (o) port expander bbSPI SCLK
sbit MOSI       = P1^3;				// (o) port expander bbSPI MOSI
sbit CMP_IN		= P1^2;				// (a) comparator (+) input (VREF with hysteresis)
sbit PTTM_N     = P1^1;				// (i) mic PTT in (act low)
sbit DMUTE		= P1^0;				// (o) rig data mute out (act hi)
sbit Ro2		= P1^0;				// (o) strap option 2 (bit 1)

sbit LED_PWM	= P0^7;				// (o) PWM out for LED
sbit Ro3		= P0^6;				// (o) strap option 3 (bit 2)
sbit MMUTE_N	= P0^5;				// (o) mic mute out (act low)
sbit Ro1		= P0^5;				// (o) strap option 1 (bit 0)
sbit SPR		= P0^4;				// (i) SW option bit input
sbit PWMDAC		= P0^3;				// (o) DDS PWMDAC output
sbit MDATA_IN	= P0^2;				// (i) limited serial data input (connected to comparator out)
sbit CMPOUT		= P0^1;				// (o) comparator putput
sbit VREF		= P0^0;				// (a) ext VREF in (NOT USED FOR GPIO IN THIS DESIGN)
#endif

//-----------------------------------------------------------------------------
// Local variables
//-----------------------------------------------------------------------------
#define	HM_START	0x01
#define	HM_ERROR	0x02
#define	HM_BUFMAX	3								// HM-133/151 buffers and working regs
volatile	U8		hm_hptr;						// save ptr in case we overflow
volatile	U8		hm_tptr;						// HM-133/151 key buffer tail ptr
volatile	U32		hm_buf[HM_BUFMAX];
volatile	U8		hm_status_buf[HM_BUFMAX];
volatile	U8		hm_ctptr;						// capture buf tail ptr
volatile	U8		hm_chptr;						// capture buf head ptr
volatile	U32		sys_error_flags;				// system error flags
volatile	char	curr_key;

volatile	U8		ipldds;							// dds ISR init flag
volatile	U16		delF1;							// phase (tone) register for tone 1
volatile	U16		delF2;							// phase (tone) register for tone 2

volatile	U16		hmkey_timer;					// key timer
volatile	U8	 	waittimer;		              	// wait() function timer
volatile	U8		dbounceHM_tmr;
volatile	U8		iplTMR;				            // timer IPL init flag
volatile	U16		press_timer;					// key press timer and flag
volatile	U8		press_flag;						// key-press timeout flag
			U16		pulse_delay;					// pulse delay value
#define	UD_PERIOD	10								// up/dn pulse period
volatile	U16		ud_timer;
volatile	U8		xport;							// expansion port data register

// bbSPI registers
volatile	U8		spdr;
volatile	U8		spmask;

// HM133/151 ISR data decode vars
volatile	U32	dmask;								// data mask
volatile	U32	hm_data;							// data register
volatile	U8	hm_status;							// status register
volatile	U16	last_edge;							// last edge capture (sign extended to 32 bits)
volatile	U8	bit_count;							// count of # bits rcvd

// HM-133/151 code LUT arrays:
#define	MAX_KEY	25									// # keys on HM-151
#define	MAX_KEY_133	23								// # keys on HM-133

//	HM-133: there are actually 25 keys, but two of them act as modifiers
//	  (func and dtmf) and do not send a key code.  The keys have different
//	  labeling, but each key is in roughly the same position with the same
//	  code.  However, many keys are labeled differently from the HM-151.
//	  HM-133 applications should use the following key map:
//		Key code	Function (key)						HM-151 function (key)
//		  M			F2									MR
//		  V			F1									V/M
//		  X			Band change							XFC
//		  T			MR (and MW with persistent press)	TUNER/CALL
//		  L			VFO									SPCH/LOCK
//		  F			n/a									F1
//		  G			n/a									F2
//
// HM-133 function key return set.  '%' and '!' are placekeepers to keep
//	alignment with the key_addr[] array.  These codes should never be sent
//	by an HM-133.  HM-151 doesn't have a FUNC mode, so it will never send
//	any of these codes.
// !! These codes don't mean anything to the adapter, so they are included as comments
//	for completeness. !!
/*code char fnkey_code[] = { 'p',   'o',   'n',   'k',   'm',   'l',   'j',  '%',
					  '!',   'a',   'b',   'c',   'q',   'd',   'e',   'f',
					  'r',   'g',   'h',   'i',   's',   '+',   '`',   '$',
					  't',   '\0' };*/
// Normal mode keycodes for HM-151 and HM-133 (see above for HM-133 notes)
code char key_code[] = { 'L',   'T',   'X',   '/',   'V',   'M',   '\\',  'F',
					'G',   '1',   '2',   '3',   'A',   '4',   '5',   '6',
					'B',   '7',   '8',   '9',   'C',   '*',   '0',   '#',
					'D',   '\0' };

#define	DTMF_OFFS	(9)		// offset to subtract from key_code index to become dtmf index
							// this offset is dependent on the placement of keycodes in the
							// "_code" tables above.  SO...don't re-arrange the key_code[] table!

// DTMF tone lookup tables for row and column
code U16 dtmf_row[] = { ROW1_TONE, ROW1_TONE, ROW1_TONE, ROW1_TONE,
					    ROW2_TONE, ROW2_TONE, ROW2_TONE, ROW2_TONE,
					    ROW3_TONE, ROW3_TONE, ROW3_TONE, ROW3_TONE,
					    ROW4_TONE, ROW4_TONE, ROW4_TONE, ROW4_TONE,
					  };
code U16 dtmf_col[] = { COL1_TONE, COL2_TONE, COL3_TONE, COL4_TONE,
					    COL1_TONE, COL2_TONE, COL3_TONE, COL4_TONE,
					    COL1_TONE, COL2_TONE, COL3_TONE, COL4_TONE,
					    COL1_TONE, COL2_TONE, COL3_TONE, COL4_TONE,
					  };
// serial key codes less the func/dtmf/1stkey modifier nybble.
code U16 key_addr[] =  {0x0b02,0x1302,0x2302,0x2202,0x0a02,0x1202,0x2002,0x1002,
				   0x0802,0x0b82,0x1382,0x2382,0x4382,0x0982,0x1182,0x2182,
				   0x4182,0x0a82,0x1282,0x2282,0x4282,0x0882,0x1082,0x2082,
				   0x4082,0x0000};

// The first index selects the radio type, the second index selects the button.
//	The array return is written to the port expansion SR to select the desired Fn.
code U8 radio_fbtn[][7] = {
//		HM133 button -->	UP  F1        VFO/LOCK     DN  MR/CALL       BAND/OPT	Pulse Delay
//							|   |         |            |   |             |			|
						  { UP, UP1,      UP2,         DN, DN1,          DN2,      MS45},		// 0	generic (ICOM)
						  { UP, MH36_ACC, MH36_P1,     DN, MH36_DMR,     MH36_P2, MS100},		// 1	Yaesu MH36
						  { UP, UP1,      UP2,         DN, DN1,          DN2,      MS80},		// 2	Kenwood
						  { UP, UP1,      UP2,         DN, DN1,          DN2,     MS100},		// 3	generic (ICOM/KW) <- placekeeper for new rig type
						  { UP, UP1,      UP2,         DN, DN1,          DN2,     MS125},		// 4	generic (ICOM/KW) <- placekeeper for new rig type
						  { UP, UP1,      UP2,         DN, DN1,          DN2,     MS125},		// 5	generic (ICOM/KW) <- placekeeper for new rig type
						  { UP, UP1,      UP2,         DN, DN1,          DN2,     MS125},		// 6	generic (ICOM/KW) <- placekeeper for new rig type
						  { 0,    0,        0,          0,   0,            0,         0}		// 7	HM-151 (IC-7000) -- no up/dn or other outputs supported
					      };																	//		for this mic

code U8 swopt_pattern_lut[] = {
	// "a" = 00000001, mask = 00000010
	0x01,
	// "b" = 00001000, mask = 00001000
	0x08,
	// "c" = 00001010, mask = 00001000
	0x0a,
	// "d" = 00000100, mask = 00000100
	0x04,
	// "e" = 00000000, mask = 00000001
	0x00,
	// "f" = 00000010, mask = 00001000
	0x02,
	// "g" = 00000110, mask = 00000100
	0x06,
	// "h" = 00000000, mask = 00001000
	0x00 };

code U8 swopt_mask_lut[] = {
	// "a" = 00000001, mask = 00000010
	0x02,
	// "b" = 00000001, mask = 00001000
	0x08,
	// "c" = 00000101, mask = 00001000
	0x08,
	// "d" = 00000100, mask = 00000100
	0x04,
	// "e" = 00000000, mask = 00000001
	0x01,
	// "f" = 00000010, mask = 00001000
	0x08,
	// "g" = 00000110, mask = 00000100
	0x04,
	// "h" = 00000000, mask = 00001000
	0x08 };

//-----------------------------------------------------------------------------
// Local Prototypes
//-----------------------------------------------------------------------------

void process_hm(U8 flag);
void flash_swvers(U8 pattern, U8 mask);
void pulse(U8 discr, U8 pcount, U16 pdly);
void outbit(U8 bitmap, U8 on);
void send8(U8 sdata);
U8 get_opt(void);
void wait(U8 wvalue);
U8 got_hmd(void);
S32 get_hmd(void);
char* get_hmcode(U32 keym);

//******************************************************************************
// main()
//  The main function inits I/O and process I/O.
//
//******************************************************************************
void main(void) //using 0
{
	volatile U8		i;				// temp uchar
	volatile U8		j;
			 U8		q;				// DTMF-S temp
			 U8		t;				// DTMF-S temp
			 U8		mdet_edge;		// mic det edge reg
			 U8		new_events;
			 U8		dmute_mem;		// reg to hold initial dmute status when "G" key detected
	volatile char	d;				// temp char
			 char*	dp;				// pointer to key code array
#define	MAX_ACCUM	99
	volatile U8		accum;			// u/d pulse-count accumulator
			 U32	si;				// temp 32
	static	 U8		hm_count;		// hm key repeat counter
	static	 U8		keyh_mem;		// current key (used to determine 1st keypress)
	static   U8		keyh_edg;		// 1st key edge
	static	 U8		ptt_edge;		// edge discrete
	static	 U8		aflag;			// accum repeat inhib flag
	static	 U8		softptt;		// soft ptt detected
	volatile U8		swopt;			// SW ipl strap options
	volatile U8		run;			// warm restart trigger

	// start of main
	while(1){									// outer loop is for soft-restart capability
		PCA0MD = 0x00;							// disable watchdog
		EA = 0;
		run = 1;								// enable run
		// init MCU system
		Init_Device();							// init MCU
		EA = 1;
		xport = 0;								// init expansion port
	#if VERS == 1
		// VERS 1 port init
		PTTb = 0;								// (o) PTT out to rig (act hi)
		DNb = 0;								// (o) mic DN button out to rig (act hi)
		UPb = 0;								// (o) mic UP button out to rig (act hi)
		swopt = 0x0;							// set version 1 SW strap
		DMUTE = 0;								// (o) rig data mute out (act hi)
	#endif
	#if	VERS == 2
		// VERS 2 port init
		swopt = get_opt();						// fetch SW option straps
		send8(xport);
		P0MDOUT   = 0xAE;						// update GPIO config, post option read
		P1MDOUT   = 0x59;
		DMUTE = 0;
	#endif
		swopt = swopt & 0x07;					// isolate rig options
//		swopt = STRAP_HM151;		// force HM151 debug
		pulse_delay = (U16)radio_fbtn[swopt][IDX_PLSDLY]; // fetch pulse delay
		MDATA = 1;								// (i) mic data in
		MMUTE_N	= UNMUTE_BIT;					// (o) mic mute out (act low)
		PTTM_N = 1;								// (i) mic PTT in (act low)
		DMUTE = 1;								// (o) rig data mute out (act hi)
	#if	VERS == 2
		MIC_DET_N = 1;							// (i) init mic det (act low)
	#endif
		// init module vars
		iplTMR = TMRIPL;                        // timer IPL init flag
		PCA0CPM1 &= 0xfe;						// disable CCF1
		dmask = 0x01;							// reset data regs
		hm_data = 0;
		bit_count = 0;
		hm_status = 0;
		last_edge = 0;
		press_flag = 0;
		keyh_edg = 0;
		delF1 = 0;								// cler the DDS tone regs
		delF2 = 0;
		hmkey_timer = 0;
		waittimer = 0;
		dbounceHM_tmr = 0;
		press_timer = 0;
		press_flag = 0;
		ud_timer = 0;
		ipldds = 1;								// ipl the DDS
		// process IPL init
		keyh_mem = KEY_NULL;
		hm_count = 0;
		aflag = 0;
		softptt = 0;							// init soft PTT
	#if	VERS == 2
		mdet_edge = ~MIC_DET_N;					// init mic det edge to trap on IPL
	#else
		mdet_edge = 1;
	#endif

		#ifndef	IS_SIM							// skip DDS wait for sim
		while(ipldds);							// wait for DDS to init
		#endif
		
		#ifdef	IS_SIM							// allows range-checking of big constants in simulator
		delF1 = HMT_SYNC;						// place constant expressions here for a quick check
		delF1 = COL4_TONE;
		#endif

		// flash version and radio select code:
		//	 "3" = 00000011 = 0x03
		//	mask = 00010000 = 0x10
		flash_swvers(0x03, 0x10);				// Version 3
		// radio select code: use LUT for pattern and mask
		flash_swvers(swopt_pattern_lut[swopt], swopt_mask_lut[swopt]);
		
		if(swopt == STRAP_HM151){				// if HM151 FT mode...
			DMUTE = 0;							// pass-thru MDATA
			PCA0CPM2 &= ~0x40;					// fn led off (ECOM);
		}else{
			DMUTE = 1;							// inhibit pass-thru
			PCA0CPM2 |= 0x40;					// fn led on (ECOM);
		}
		ptt_edge = ~PTTM_N;
		// main loop
		while(run){													// inner-loop runs the main application
			new_events = 0;
			d = PTTM_N;
			if(d != ptt_edge){										// process manual PTT input
				ptt_edge = d;
				if(d){												// PTT input is ground true
					MMUTE_N = MUTE_BIT;								// PTT released
					outbit(PTT, 0);
					delF1 = 0;
					delF2 = 0;
					if(swopt != STRAP_HM151){						// if NOT HM151 FT mode...
						PCA0CPM2 |= 0x40;							// fn led on (ECOM);
					}else{
						if(DMUTE){
							PCA0CPM2 |= 0x40;						// fn led on (ECOM);
						}
					}
					// turn off PWMO ISR
					PCA0CPM1 &= 0xfe;
				}else{
					outbit(PTT, 1);									// PTT pressed
					MMUTE_N = UNMUTE_BIT;
				}
			}
			d = '\0';
			if(got_hmd()){											// trap keys
				new_events = KEYHM_CHNG;
				dbounceHM_tmr = 150;
			}
			if(dbounceHM_tmr == 0){
				if(keyh_mem != KEY_NULL){							// trap debounce timeouts
					new_events |= KEYHM_CHNG;
				}
			}
			if(delF1){
				press_timer = MS1650;								// reset DTMF timer anytime a tone is being generated
			}
			if(press_flag){											// keypress timeout, enable mic
				press_flag = 0;
				if(swopt != STRAP_HM151){							// if NOT HM151 FT mode...
					PCA0CPM2 |= 0x40;								// fn led back on (ECOM);
				}else{
					if(DMUTE){
						PCA0CPM2 |= 0x40;							// fn led back on (ECOM);
					}
				}
				MMUTE_N = UNMUTE_BIT;								// unmute mic
			}
	#if	VERS == 2
			t = MIC_DET_N;											// grab the GPIO
	#else
			t = 0;
	#endif
			if(mdet_edge != t){										// take action if there has been a change
				mdet_edge = t;
				if(t){												// if mic removed, abort PTT and tones
					outbit(PTT, 0);
					if(run == 2){									// This is an interlock to prevent continuous restarts while the mic is disconnected
						run = 0;									// restart
					}else{
						run = 2;									// set first pass interlock
					}
				}
				if(swopt == STRAP_HM151){							// if HM151 FT mode... update mirror
					t = ~t & 0x01;									// need to invert to account for NFET inversion
					outbit(DN, t);
				}
			}
			// this branch handles key-driven events
			if(new_events & KEYHM_CHNG){							// process trapped events
				if(!softptt){
					if(dbounceHM_tmr == 0){							// process no-key from HM-133/151
						keyh_edg = 0;
						if(keyh_mem != KEY_NULL){
							// release key
							if(!delF1){
								if(swopt != STRAP_HM151){
									PCA0CPM2 |= 0x40;				// led back on if no tone (ECOM);
								}else{
									if(DMUTE){
										PCA0CPM2 |= 0x40;			// led back on if no tone (ECOM);
									}
								}
							}
							aflag = 0;								// release accum inhibit
							i = keyh_mem | 0x80;					// set release keypress
						}
						keyh_mem = KEY_NULL;						// if no key, clear current key mem.
					}
				}
				si = get_hmd();										// get hm-133/151 data word
				dp = get_hmcode(si);								// do key-code lookup, pointer is set to keycode array
				d = *(dp + HM_IDX_CODE);

				if(*(dp + HM_IDX_STAT) & HM_1STKEY){
					if(keyh_edg == 0){
						keyh_edg = 1;
					}else{
						*(dp + HM_IDX_STAT) &= ~HM_1STKEY;			// only allow 1 1stkey in a row
					}
				}else{
					keyh_edg = 0;
				}

				if(d != KEY_NULL){									// trap first press, and prevent false repeats
					if(d == PTT_CODE){
						i = d;
						keyh_mem = KEY_NULL;
					}else{
						dbounceHM_tmr = 150;						// reset loss-of-key detect timer
						if(d && (d != keyh_mem)){					// if mic data is not null AND key mem is null:
							keyh_mem = d;							// save key to mem (first key)
							i = d;
							hm_count = HMKEY_HOLD_CNT;				// set key-hold counter
						}else{
							if(hm_count != 0){						// processes key-hold counter (not used at the moment)
								if(--hm_count == 0){
									// Process key "HOLD"
									if((keyh_mem == '/') || (keyh_mem == '\\')){
										*(dp + HM_IDX_STAT) |= HM_1STKEY;	// recycle key
										i = keyh_mem;
									}
									hm_count = 2;
								}
							}
						}
					}
				}else{
					keyh_edg = 0;									// save null to key mem if debounce == 0
				}
				if(swopt == STRAP_HM151){							// if HM151 FT mode...
					if(i == ('G'|0x80)){
						if(dmute_mem){
							DMUTE = 0;
							PCA0CPM2 &= ~0x40;						// fn led off (ECOM);
							dmute_mem = 0;
						}
					}
					if(*(dp + HM_IDX_STAT) & HM_1STKEY){
						if(i == 'G'){								// toggle DMUTE when HM151-F2 key encountered
							dmute_mem = DMUTE;						// remember initial state of DMUTE
							if(!DMUTE){								// update Fn LED
								DMUTE = 1;
								PCA0CPM2 |= 0x40;					// fn led on (ECOM);
							}
						}
					}
					if(!DMUTE){										// ignore keypresses if data is NOT muted
						i = '\0';
					}
				}
				if(i & 0x80){
					delF1 = 0;
					delF2 = 0;
				}
				// process DTMF keys if PTT == 1, or into accum if PTT == 0
				if(((i>='0') && (i<='9')) || ((i>='A') && (i<='D')) || (i =='*') || (i == '#')){
					t = 1;
				}else{
					t = 0;
				}
				j = *(dp + HM_IDX_STAT) & HM_1STKEY;
				if(*(dp + HM_IDX_STAT) & (HM_PTT | HM_DTMF) == (HM_PTT | HM_DTMF) && !t){
					// DTMF-S PTT hold
					q = 1;
					delF1 = 0;
					delF2 = 0;
				}
	/*			if(((*(dp + HM_IDX_STAT) & HM_PTT) == HM_PTT) && q){
					// DTMF-S release
					q = 0;
					MMUTE_N	= UNMUTE_BIT;							// (o) mic unmute out (act low)
				}*/
				if(t){
					if(*(dp + HM_IDX_STAT) & HM_DTMF){
						press_timer = MS1650;						// reset DTMF timer
					}
					if(*(dp + HM_IDX_STAT) & HM_PTT){
						outbit(PTT, 1);
					}
					if(xport & PTT){
						if(j){
							PCA0CPM2 &= ~0x40;						// fn led off if dtmf (ECOM);
							MMUTE_N	= MUTE_BIT;						// (o) mic mute out (act low)
							delF1 = dtmf_row[*(dp+HM_IDX_DTMF)];
							delF2 = dtmf_col[*(dp+HM_IDX_DTMF)];
						}
					}else{
						if((i >= '0') && (i <= '9')){				// enter numbers into accumulator
							if(swopt != STRAP_HM151){				// only if NOT in HM151 FT mode...
								PCA0CPM2 &= ~0x40;	//ECOM;
								if(!aflag){
									aflag = 1;						// inhibit repeats
									j = i - '0';
									accum *= 10;
									accum += j;
									if(accum > MAX_ACCUM){
										accum = j;
									}
								}
							}
						}else{
							accum = 0;								// non-numbers in DTMF space clear accum
							if(i == '*'){
							PCA0CPH2  = LED_DIM;					// set dim LED
							PCA0CPL2  = 0;
							}
							if(i == '#'){
							PCA0CPH2  = LED_BRT;					// set brt LED
							PCA0CPL2  = 0;
							}
						}
					}
				}else{
					if(swopt == STRAP_HM151){						// if HM151 FT mode...
						if(i != 'M'){								// at this point, only "M" key is allowed in HM151 mode (other allowed
							i = '\0';								// ... keys have already been processed) all others are ignored
						}
					}
					switch(i){
						case '/':									// up key
							if(*(dp + HM_IDX_STAT) & HM_1STKEY){
								pulse(radio_fbtn[swopt][IDX_UP], accum, pulse_delay);
								accum = 0;
							}
							break;
						
						case '\\':									// down key
							if(*(dp + HM_IDX_STAT) & HM_1STKEY){
								pulse(radio_fbtn[swopt][IDX_DN], accum, pulse_delay);
								accum = 0;
							}
							break;
						
						case PTT_CODE:
							if(*(dp + HM_IDX_STAT) & HM_PTT){		// is PTT pressed keycode
								if(*(dp + HM_IDX_STAT) & HM_DTMF){	// if DTMF active, time to turn off tones if we see this code
									MMUTE_N = MUTE_BIT;				// stay muted...
									delF1 = 0;
									delF2 = 0;
								}else{
									if(!press_timer){
										MMUTE_N = UNMUTE_BIT;		// unmute mic
									}
								}
								outbit(PTT, 1);						// activate PTT out
								softptt = 1;						// set soft PTT active
							}else{
								MMUTE_N = MUTE_BIT;
								outbit(PTT, 0);						// de-activate PTT out
								delF1 = 0;
								delF2 = 0;
								if(swopt != STRAP_HM151){			// if NOT HM151 FT mode...
									PCA0CPM2 |= 0x40;				// fn led on (ECOM);
								}else{
									if(DMUTE){
										PCA0CPM2 |= 0x40;			// fn led on (ECOM);
									}
								}
								softptt = 0;						// set soft PTT inactive
							}
							break;

						case 'L':									// VFO/LOCK (HM151 = SPCH/LOCK)
							if(*(dp + HM_IDX_STAT) & HM_1STKEY){
								pulse(radio_fbtn[swopt][IDX_VFO], 1, pulse_delay);
							}
							break;

						case 'T':									// MR/CALL (HM151 = TUNER/CALL)
							if(swopt == STRAP_ICOM){				// if ICOM mode...long up pulse to start scan
								if(*(dp + HM_IDX_STAT) & HM_1STKEY){
									pulse(radio_fbtn[swopt][IDX_UP], 1, MS800);
								}
							}else{
								if(*(dp + HM_IDX_STAT) & HM_1STKEY){
									pulse(radio_fbtn[swopt][IDX_MR], 1, pulse_delay);
								}
							}
							break;

						case 'X':									// BND/OPT (HM151 = XFC)
							// toggle DMUTE on v001 hardware
							if(*(dp + HM_IDX_STAT) & HM_1STKEY){
								pulse(radio_fbtn[swopt][IDX_BAND], 1, pulse_delay);
							}
							break;

						case 'V':									// F1 (HM151 = V/M)
							if(swopt == STRAP_ICOM){				// if ICOM mode...long dn pulse to start scan
								if(*(dp + HM_IDX_STAT) & HM_1STKEY){
									pulse(radio_fbtn[swopt][IDX_DN], 1, MS800);
								}
							}else{
								if(*(dp + HM_IDX_STAT) & HM_1STKEY){
									pulse(radio_fbtn[swopt][IDX_F1], 1, pulse_delay);
								}
							}
							break;

						case 'M':									// Toggle test tone
							if(*(dp + HM_IDX_STAT) & HM_1STKEY){
								if(delF1){
									MMUTE_N	= UNMUTE_BIT;			// (o) mic mute out (act low)
									delF1 = 0;
									delF2 = 0;
									if(swopt != STRAP_HM151){		// if NOT HM151 FT mode...
										PCA0CPM2 |= 0x40;			// fn led on if no tone (ECOM);
									}else{
										if(DMUTE){
											PCA0CPM2 |= 0x40;		// fn led on if no tone (ECOM);
										}
									}
								}else{
									MMUTE_N	= MUTE_BIT;				// (o) mic mute out (act low)
									delF1 = TONE_1000;
									delF2 = 0;
									PCA0CPM2 &= ~0x40;				// fn led off if tone (ECOM);
								}
							}
							break;

						default:
							i += 1;
							break;
					} // end key processing switch()
				} // end "key processing" branch
			} // end "new events"
		} // end while(run)
	} // end outer while()
} // end main()

// *********************************************
//  *************** SUBROUTINES ***************
// *********************************************

//-----------------------------------------------------------------------------
// flash_swvers() uses ms timer to pulse out Morse data on LED
//	"pattern" is dit(0) or dah(1).  1st element is pointed to by "mask"
//		subsequent elements are toward the LSb of pattern (mask is right shifted
//		to reach next element).  When mask == 0, character is done.  There is
//		a DAH delay on entry and exit.
//-----------------------------------------------------------------------------
	
#define	MORSE_PER	MS100	// dit period

void flash_swvers(U8 pattern, U8 mask){
	U8	j = 0;			// temp vars
volatile	U8	k = mask;		// vers pattern temps

	PCA0CPM2 &= ~0x40;								// led dim (ECOM)
	ud_timer = (3 * MORSE_PER);
	while(ud_timer != 0L);
	do{
		PCA0CPM2 &= ~0x40;							// led dim (ECOM)
		ud_timer = MS100;
		while(ud_timer != 0L);
		PCA0CPM2 |= 0x40;							// led brt (ECOM)
		if(pattern & k){
			ud_timer = (3 * MORSE_PER);
		}else{	
			ud_timer = MORSE_PER;
		}
		k = k >> 1;
		while(ud_timer != 0L);
		j = got_hmd();
		j |= ~PTTM_N;
	}while((j == 0) && k);
	if(j == 0){										// if there was no keypress or ptt, delay
		PCA0CPM2 &= ~0x40;							// led dim (ECOM)
		ud_timer = (3 * MORSE_PER);
		while(ud_timer != 0L);
	}
	PCA0CPM2 |= 0x40;								// led brt (ECOM)
	return;
}

//-----------------------------------------------------------------------------
// pulse() uses ms timer to pulse discretes pcount pulses
//-----------------------------------------------------------------------------

void pulse(U8 discr, U8 pcount, U16 pdly){
	U8	i;		// temp vars
	U8	j;

	if(pcount == 0){
		j = 1;
	}else{
		j = pcount;
		press_timer = 100;							// set some delay
		do{
			i = got_hmd();
			if(i){
				get_hmd();							// clear data word buffer
				press_timer = 80;
			}
		}while(press_timer);
	}
	for(i=0; i<j; i++){
		PCA0CPM2 &= ~0x40;							// led dim (ECOM)
		outbit(discr, 1);
		ud_timer = pdly;
		while(ud_timer);
		PCA0CPM2 |= 0x40;							// led brt (ECOM)
		outbit(discr, 0);
		ud_timer = pdly;
		while(ud_timer);
		if(got_hmd()){
			i = j;									// a keypress detected, abort
		}
	}
	return;
}

//************************************************************************
// outbit() consolodates OC output port updates into a single function
//************************************************************************
void outbit(U8 bitmap, U8 on){
#if	VERS == 1
	// This is the version 1 routine.  Only PTT, UP, DN, & F2 (MW) are supported
	switch(bitmap){
		case PTT:
			if(on){
				PTTb = 1;
				xport |= bitmap;
			}else{
				PTTb = 0;
				xport &= ~bitmap;
			}
			break;
		
		case UP:
			if(on) UPb = 1;
			else UPb = 0;
			break;
		
		case DN:
			if(on) DNb = 1;
			else DNb = 0;
			break;
		
		case DN2:
			if(on) DMUTE = 1;
			else DMUTE = 0;
			break;
		
		default:
			break;
	}
#endif
#if	VERS == 2
	// This is the version 2 routine.  PTT, UP, UP1, UP2, DN, DN1, & DN2 are supported

	if(on){											// set/clear bitmap
		xport |= bitmap;
	}else{
		xport &= ~bitmap;
	}
	send8(xport);									// transfer to port expander
#endif
	return;
}

#if	VERS == 2
//************************************************************************
// send8() does a bit-bang SPI into a CD4094 to produce a port expansion
//	output.  Uses T0 to clock data
//************************************************************************
void send8(U8 sdata){
	
	spmask = 0x80;									// init mask
	spdr = sdata;									// store to bbSPI data reg
	TR0 = 1;
	while(TR0);										// loop until xfr complete
	STB = 1;										// toggle strobe with delay
	TR0 = 1;
	while(TR0);										// loop until xfr complete
	STB = 0;
	// since there will be several ms applied between send8 xfers, no further delay is needed
	//	but beware...this must be enforced by the programmer!!
	return;
}

//************************************************************************
// get_opt() sets strap GPIOs for inputs (assumes they are already configured
//	as OD outputs), reads the strap inputs, calculates the resulting option
//	value, then returns that value to the caller.
//************************************************************************
U8 get_opt(void){
	U8	rtn = 0;

	Ro3 = 1;										// set the option GPIOs for input
	Ro2 = 1;
	Ro1 = 1;
	wait(50);										// let the inputs settle
	rtn = ((U8)Ro3 & 0x01) << 2;					// collect the option inputs
	rtn |= ((U8)Ro2 & 0x01) << 1;
	rtn |= ((U8)Ro1 & 0x01);
	rtn = ~rtn & 0x07;								// convert to pos logic
	return rtn;
}

//************************************************************************
// wait() waits the U8 value then returns.
//************************************************************************
void wait(U8 wvalue){

	waittimer = wvalue;								// set the timer
	while(waittimer);								// wait for it to expire
	return;
}
#endif


// ******************* HM keypress capture ******************

//-----------------------------------------------------------------------------
// got_hmd() returns true if HM-133/151 data buffer not empty
//-----------------------------------------------------------------------------
U8 got_hmd(void){
 	U8	rtn = FALSE;

	if(hm_tptr != hm_hptr){
		rtn = TRUE;
	}
	return rtn;
}

//-----------------------------------------------------------------------------
// get_hmd() returns HM-133/151 data/status or -1 if no data
//-----------------------------------------------------------------------------
S32 get_hmd(void){
	S32	rtn = -1L;

	if(hm_tptr != hm_hptr){
		rtn = (S32)(hm_buf[hm_tptr] & 0x00ffffff) | ((S32)hm_status_buf[hm_tptr] << 24);
		if(++hm_tptr > (HM_BUFMAX-1)) hm_tptr = 0;
	}
	return rtn;
}

//-----------------------------------------------------------------------------
// get_hmcode() returns pointer to keycode array
//	1st index is key-data status
//	2nd index is keycode (KEY_NULL if invalid data)
//	3rd index is DTMF tone offset index
//-----------------------------------------------------------------------------
char* get_hmcode(U32 keym){
	U16		ii;					// temps
	U8		i;
	U8		di = 0;				// dtmf index temp
	static	char	hm_rtn[3];	// holding array for code and status data
								// 0 = stat, 1 = code, 2 = DTMF idx

	hm_rtn[HM_IDX_CODE] = KEY_NULL;
	hm_rtn[HM_IDX_STAT] = (U8)keym & 0x0f;						// get key status
	ii = (U16)(keym >> 4);										// conv hm151/133 keymatrix to keyaddr (low nyb is status)
	if(ii == 0x0002){											// HM-133 PTT
		hm_rtn[HM_IDX_CODE] = PTT_CODE;							// semicolon is PTT
	}else{
		if(ii != 0xffff){
			i = 0;
			do{
				if(ii == key_addr[i]){	
					hm_rtn[HM_IDX_CODE] = key_code[i];			// look for match in fn LUT
					di = i;
				}
			}while((hm_rtn[HM_IDX_CODE] == KEY_NULL) && key_addr[++i]);	// loop exits when match or end of LUT
			if(hm_rtn[HM_IDX_CODE] == KEY_NULL){
				sys_error_flags |= HM_DATA_ERR;
			}
			hm_rtn[HM_IDX_DTMF] = di - DTMF_OFFS;
		}
	}
	return hm_rtn;
}

//-----------------------------------------------------------------------------
// pca_intr
//-----------------------------------------------------------------------------
//
// Called when PCA CEX0 input RE event is detected:
//	This event is rising edge from the HM-151/151 data line.  HM-133/151 data is a
//		serial format that features a 195us low followed by a 230us high
//		(logic "0") or a 415us high (logic "1").  NOTE: The data line is
//		inverted at the input of the MCU (by the comparator).  7 zeros plus a
//		750us low is a sync pulse (start of a 2-word frame)
//
//	HM-151 Keys send bursts of 2 data words at a 43-60 ms rate (measured at
//		about 50 ms on a single device).
//
//	This timer measures the time between rising edges (as seen at MCU pin) and
//		determines if the transitions represent a 1, 0, or sync pulse.  An
//		application timer is used to establish a previous period of no edge
//		activity. (this resets the serial input state machine).
//		The messages are 19 bits plus a stop bit.  Since the timer looks for
//		falling edges, and bursts consist of two words, only the 1st word's
//		stop bit will be captured.  Thus, the first word after a long idle or
//		a sync pulse will count 20 bits until the data is captured.  The 2nd
//		word triggers a capture after only 19 bits.  This saves the expense
//		of configuring another timer ISR to establish the end of the 2nd word's
//		stop bit.
//		Once captured, the data word is stored to the FILO buffers (hm_buf[]
//		for data, hm_status_buf[] for status) for processing by the process loops.
//		NOTE: Data consumers must extract both the data and status before
//		updating the tail pointer.
//
//		Keycode matrix:
//		LOCK (L)	TUNER (T)	XFC (X)
//		0b020		13020		23020
//
//		upARROW (/)		V/M (V)	MW (M)
//		22020			0a020	12020
//
//		dnARROW (\)		F1 (F)	F2 (f)
//		20020			10020	08020
//
//		1		2		3		MODE (A)
//		0b820	13820	23820	43820
//
//		4		5		6		FIL  (B)
//		09820	11820	21820	41820
//
//		7		8		9		GENE (C)
//		0a820	12820	22820	42820
//
//		. (*)	0		CE (#)	ENT  (D)
//		08820	10820	20820	40820
//
//		HM-133:
//		(1) 00002 (1) = PTT pressed
//		(3) 00002 (0) = PTT release
//-----------------------------------------------------------------------------

void pca_intr(void) interrupt 9 using 2{
	// HM-133/151 data decode vars
	U8		temp;				// temp ptr reg
	U16		captim;				// capture time holding reg
	U16		captemp;			// capture time temp reg
	
    if(CCF0 == 1){
			CCF0 = 0;                       				// clr intr flag

			captim = (U16)PCA0L;							// grab time of edge
			captim |= ((U16)PCA0H) << 8;
			if(captim < last_edge){							// calc pulse duration (in SYSCLK tics)
				captemp = 65535 - last_edge + captim;		// if rollover
			}else{
				captemp = captim - last_edge;				// normal timer progression
			}
			if(captemp < HMT_MIN) return;					// ignore if pulse too short
			last_edge = captim;								// save new edge
			if(hmd_timer == 0){								// look for LOS timeout
				// reset data & mask
				dmask = 0x01;								// reset data/status regs
				bit_count = 0;
				hm_data = 0;
				hm_status = 0;
				hmd_timer = HMD_BURST_TO;					// reset burst timer
			}else{
				hmd_timer = HMD_BURST_TO;					// reset 5ms timer
				if(captemp < HMT_0){						// check if pulse is a "0"
					dmask = dmask << 1;						// shift dmask...0 in data is implicit
					bit_count++;							// update bit count
				}else{
					if(captemp < HMT_1){					// check if pulse is a "1"
						hm_data |= dmask;					// place a "1" into the data reg
						dmask = dmask << 1;					// shift dmask
						bit_count++;						// update bit count
					}else{
						if(captemp < HMT_SYNC){ // if sync pulse (opt, && sync preamble),
							dmask = 0x01;					// reset data input registers (discard old data)
							hm_data = 0;
							bit_count = 0;
							hm_status |= HM_START;			// set start of frame
		/*					if((!((hm_data >> (bit_count-7)) & 0x7F)) && (bit_count > 6)){
								if((!((hm_data >> (bit_count-7)) & 0x7F)) && (bit_count > 6)){
								hm_status |= HM_START;		// set start if proper sync
							}else{
								hm_status |= HM_ERROR;		// else set error
							}*/
						}else{
							hm_status |= HM_ERROR;			// else set error
						}
					}
				}
				if(bit_count == 20){						// if 20 bits, capture data & status
					temp = hm_hptr;							// save ptr in case we overflow
					hm_buf[hm_hptr] = hm_data;
					hm_status_buf[hm_hptr++] = hm_status;
					if(hm_hptr > (HM_BUFMAX-1)) hm_hptr = 0; // roll-over head pointer
					if(hm_hptr == hm_tptr) hm_hptr = temp;	// buffer overflow, don't advance head
					dmask = 0x01;							// reset data regs
					hm_data = 0;
					bit_count = 1;							// only look for 19 bits on the 2nd try
					hm_status = 0;
				}
			}
		}
		return;
}

//-----------------------------------------------------------------------------
// Timer0_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 0 overflows (NORM mode):
//      drives bbSPI by shifting 8 bits out MOSI at the timer rate/2 (the timer
//			sets the half clock period).
//		If T0 enabled with spmask = 0, nothing happens except T0 is disabled when
//			the interrupt happens.  This is used to time the strobe pulse (external
//			code handles the strobe set/clear).
//
//-----------------------------------------------------------------------------

void Timer0_ISR(void) interrupt 1
{
#if	VERS == 1
	TR0 = 0;										// shouldn't get here, but turn off intrpt if we do
#endif

#if	VERS == 2
	if(!spmask){									// used to time strobe pulse
		TR0 = 0;									// turn off T0 intrpt
	}else{
		if(SCLK){									// toggle clock & set MOSI or shift data, depending on edge
			SCLK = 0;
			if(spdr & spmask) MOSI = 1;				// FE: transfer sdata bit to output
			else MOSI = 0;
		}else{
			SCLK = 1;
			spmask >>= 1;							// RE: shift data
			if(!spmask) TR0 = 0;					// xfr done, disable intrpt
		}
	}
    TF0 = 0;                           				// Clear Timer0 intrpt flag
#endif

	return;
}

//-----------------------------------------------------------------------------
// Timer1_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 1 overflows (NORM mode):
//      drives DDS
//
//-----------------------------------------------------------------------------

void Timer1_ISR(void) interrupt 3 using 2
{
	// DDS vars
	U8		pac;				// temp regs
	U16		pdac;
	static U16	phaccum1;		// tone 1 phacc
	static U16	phaccum2;		// tone 2 phacc

    TF1 = 0;                           				// Clear Timer2 interrupt flag
	
	if(ipldds){
		phaccum1 = 0;
		phaccum2 = 0;
		ipldds = 0;
	}
	// process phase accumulator 1
	phaccum1 += delF1;								// add delta for tone 1
	pac = (U8)(phaccum1 >> 8);
	pdac = (U16)SINE[pac];

	// process phase accumulator 2
	phaccum2 += delF2;								// add delta for tone 2
	pac = (U8)(phaccum2 >> 8);
	pdac += (U16)SINE[pac];							// add tone 2 DAC to holding reg
	pdac >>= 1;										// div by 2 to get 8 bit combined tone DAC value
	// store pdac to pwm
	PCA0CPH1 = (U8)pdac;							// storing here to sync update
}

//-----------------------------------------------------------------------------
// Timer2_ISR
//-----------------------------------------------------------------------------
//
// Called when timer 2 overflows (NORM mode):
//      updates app timers @ 10ms rate
//		rate = (sysclk/12) / (65536 - TH:L)
//
//-----------------------------------------------------------------------------

void Timer2_ISR(void) interrupt 5 using 2
{

    TF2H = 0;                           			// Clear Timer2 interrupt flag
    if(waittimer != 0){                 			// g.p. delay timer
        waittimer--;
    }
	if(hmkey_timer){
		hmkey_timer--;
	}
    if(dbounceHM_tmr != 0){             			// pbsw debounce timer
        dbounceHM_tmr--;
    }
	if(ud_timer){									// key timer
		ud_timer--;
	}
	if(press_timer){								// key press timer and flag
		press_timer--;
		if(!press_timer){
			press_flag = 1;
		}
	}
}

//#undef IS_MAINC
//**************
// End Of File
//**************
