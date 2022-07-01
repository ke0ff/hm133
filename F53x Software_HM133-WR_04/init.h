/*************************************************************************
 *********** COPYRIGHT (c) 2020 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: init.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for main.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    08-23-20 jmh:  modified to support bluetooth interface
 *    07-13-13 jmh:  removed typecast from timer defines & updates XTAL freq 
 *    05-10-13 jmh:  creation date
 *
 *******************************************************************/

#include "typedef.h"

#ifndef INIT_H
#define INIT_H
#endif

#define	REV_STR	"#RC900_WRDv0.1%\r"
//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

// timer definitions.  Uses EXTXTAL #def to select between ext crystal and int osc
//  for normal mode.
// SYSCLK value in Hz
#define SYSCLKL 10000L
#ifdef EXTXTAL
#define SYSCLKF 20000000L
#else
#define SYSCLKF (12000000L) // / 8)
#endif
#define	SYSCLK	24500000L
// timer2 register value
#define TMR2RLL_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) & 0xffL)
#define TMR2RLH_VAL (U8)((65536 -(SYSCLK/(12L * 1000L))) >> 8L)
#define	TMRSECHACK	2
#define TMRIPL		1
#define TMRRUN		0
#define	T2_MS1PS	25

// HM Key defines
#define	HM_DATA_ERR		0x00001000		// HM-151 data error (not valid keycode)
#define	KEY_NULL		'@'				// null chr for HMD keycode LUT
#define	KEY_CHNG		0x10
#define	KEYH_CHNG		0x20
#define	KEYHM_CHNG		0x40

#define	HM_PTT			0x01			// HM-133 data bit that indicates PTT 1st press
#define	HM_1STKEY		0x02			// HM-151 data bit that indicates initial keypress
#define	HM_DTMF			0x04			// HM-133 data bit that indicates DTMF key mode
#define	HM_FNKEY		0x08			// HM-133 data bit that indicates fn key mode

#define	HM_IDX_STAT		0x00
#define	HM_IDX_CODE		0x01
#define	HM_IDX_DTMF		0x02
#define	PTT_CODE		';'				// HM-133 PTT keycode
#define HMKEY_HOLD_CNT	40				// hm-151 keypad hold timer value (~~ 2sec)
//#define HM_DEBOUNCE		150
#define HM_DEBOUNCE		75

// PCA timer constants
#define	PCACLK	(SYSCLK / 4L)
#define	FSAMP	(PCACLK / 256L)			// For 8-bit PWM, FSAMP is PCACLK / 2^8
#define	LED_DIM			0xf8
#define	LED_BRT			0x01

// port expansion open drain drive output defines (active hi)
#define	PTT				0x40			// PTT out to rig
#define	UP	    		0x04			// mic UP button out to rig
#define	UP1	    		0x02			// Kenwood CALL button (F1)
#define	UP2	    		0x08			// Kenwood VFO button (VFO/LOCK)
#define	DN	    		0x20			// mic DN button out to rig
#define	DN1	    		0x10			// Kenwood MR button (MR/CALL)
#define	DN2	    		0x01			// Kenwood PF button (BANK/OPT)
#define	MH36_ACC   		0x02			// MH36 ACC button (F1)
#define	MH36_DMR   		0x08			// MH36 D/MR button (MR/CALL)
#define	MH36_P1    		0x10			// MH36 P1 button (VFO/LOCK)
#define	MH36_P2    		0x01			// MH36 P2 button (BANK/OPT)
#define	IDX_UP			0x00			// fn array indicies
#define	IDX_F1			0x01
#define	IDX_VFO			0x02
#define	IDX_DN			0x03
#define	IDX_MR			0x04
#define	IDX_BAND		0x05
#define	IDX_PLSDLY		0x06

// strap option mode defines
#define	STRAP_ICOM		0x00			// ICOM generic strap (works for Kenwood also)
#define	STRAP_YAESU		0x01			// Yaesu MH36
#define	STRAP_KENW		0x02			// Kenwood
#define	STRAP_G100		0x03			// Generic, 100ms u/d
#define	STRAP_G125		0x04			// Generic, 125ms u/d
//#define	STRAP_???		0x05			// reserved-future
//#define	STRAP_???		0x06			// reserved-future
#define	STRAP_H901B		0x05			// ICOM 901 BlueTooth remote button version
#define	STRAP_HM151		0x07			// ICOM HM151/IC-7000


// DDS defines
//#define	PHMASK 0x1fff					// phase mask (= N_SIN)
#define N_SIN 256L						// number of effective slots in sin look-up table
#define RSHIFT 8L						// bits to downshift phase accum to align RADIX
#define RADIX 256L						// = 2^RSHIFT

// (Fsamp = 7975 Hz)	Fstep = FSAMP/(RADIX * N_SIN) = 7975/(256*256) = 0.122 Hz
// (Fsamp = 23925 Hz)	Fstep = FSAMP/(RADIX * N_SIN) = 23925/(256*256) = 0.365 Hz

#define TONE_400 (U16)(400L * (RADIX * N_SIN) / FSAMP)		// 400hz tone
#define TONE_700 (U16)(700L * (RADIX * N_SIN) / FSAMP)		// 700hz tone
#define TONE_1000 (U16)(1000L * (RADIX * N_SIN) / FSAMP)	// 1000hz tone
#define TONE_1400 (U16)(1400L * (RADIX * N_SIN) / FSAMP)	// 1000hz tone
															// F(tol) = 1.5%
#define ROW1_TONE	(U16)(697L * (RADIX * N_SIN) / FSAMP)	//= +/- 10.4 Hz	actual tolerance is +/- 0.0018% plus SYSCLK tolerance
#define ROW2_TONE	(U16)(770L * (RADIX * N_SIN) / FSAMP)	// = +/- 11.6 Hz
#define ROW3_TONE	(U16)(852L * (RADIX * N_SIN) / FSAMP)	// = +/- 12.8 Hz
#define ROW4_TONE	(U16)(941L * (RADIX * N_SIN) / FSAMP)	// = +/- 14.1 Hz
#define COL1_TONE	(U16)(1209L * (RADIX * N_SIN) / FSAMP)	// = +/- 18.1 Hz
#define COL2_TONE	(U16)(1336L * (RADIX * N_SIN) / FSAMP)	// = +/- 20.0 Hz
#define COL3_TONE	(U16)(1477L * (RADIX * N_SIN) / FSAMP)	// = +/- 22.2 Hz
#define COL4_TONE	(U16)(1633L * (RADIX * N_SIN) / FSAMP)	// = +/- 24.5 Hz

// HM-133/151 data stream bit constants
// Bit time thresholds.  Bit times are defined as us then converted to timer register values
//	regvalue = ((time(us) * PCACLK (regcnt/sec)) / 1000000 (u))
#define	HMT_MIN		((220L * (PCACLK/10L))/100000L)			// minimum bit time
#define	HMT_0		((540L * (PCACLK/10L))/100000L)			// max "0" bit time
#define	HMT_1		((850L * (PCACLK/10L))/100000L)			// max "1" bit time
#define	HMT_SYNC	((1500L * (PCACLK/10L))/100000L)		// max SYNC bit time
#define	HMD_BURST_TO 3										// burst word timeout


#define MS_PER_TIC  1
// General timer constants
#define MS25        	(25/MS_PER_TIC)
#define MS35        	(35/MS_PER_TIC)
#define MS40        	(40/MS_PER_TIC)
#define MS45        	(45/MS_PER_TIC)
#define MS50        	(50/MS_PER_TIC)
#define MS75        	(75/MS_PER_TIC)
#define MS80        	(80/MS_PER_TIC)
#define MS100        	(100/MS_PER_TIC)
#define MS125       	(125/MS_PER_TIC)
#define MS250       	(250/MS_PER_TIC)
#define MS400       	(400/MS_PER_TIC)
#define MS450       	(450/MS_PER_TIC)
#define MS500       	(500/MS_PER_TIC)
#define MS750       	(750/MS_PER_TIC)
#define MS800       	(800/MS_PER_TIC)
#define MS1000      	(1000/MS_PER_TIC)
#define MS1500      	(1500/MS_PER_TIC)
#define MS1650      	(1650/MS_PER_TIC)
#define MS2000      	(2000/MS_PER_TIC)
#define MS2500      	(2500/MS_PER_TIC)
#define MS5000      	(5000/MS_PER_TIC)
#define MS10000     	(10000/MS_PER_TIC)
#define MS20000     	(20000/MS_PER_TIC)
#define MINPERHOUR		60
#define SECPERMIN		60
#define MINPER6HOUR		(6 * MINPERHOUR)
#define MINPER12HOUR	(12 * MINPERHOUR)
#define MINPER18HOUR	(18 * MINPERHOUR)
#define MINPER24HOUR	(24 * MINPERHOUR)

//PBSW mode defines
#define	MUTE_BIT		0
#define	UNMUTE_BIT		1

// CLI commands
#define	CLI_NULL		0x00
#define	CLI_LOGHDR		0x01
#define	CLI_SURV		0x02

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

#ifndef IS_MAINC
extern U8 spi_tmr;
#endif

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

void Init_Device(void);
char wr_timeout(U16 toval);
void wr_clr(void);
char wra_timeout(U16 toval);
void wait(U8 wvalue);

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
