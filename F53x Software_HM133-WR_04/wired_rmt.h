/*************************************************************************
 *********** COPYRIGHT (c) 2022 by Joseph Haas (DBA FF Systems)  *********
 *
 *  File name: wired_rmt.h
 *
 *  Module:    Control
 *
 *  Summary:   This is the header file for bluetooth I/O.
 *
 *******************************************************************/


/********************************************************************
 *  File scope declarations revision history:
 *    08-16-20 jmh:  creation date
 *
 *******************************************************************/

//------------------------------------------------------------------------------
// extern defines
//------------------------------------------------------------------------------

/*				//       0         1           2            3        4       5           6            7        8
	enum resp_id{zero_rspn,conn_rspn,secure_rspn,secure2_rspn,rmt_rspn,ok_rspn,stream_rspn,stream2_rspn,err_rspn,

				//        9        a         b          c            d         e       f
				reboot_rspn,cmd_rspn,err2_rspn,retry_rspn,reboot2_rspn,disc_rspn,no_rspn};*/

// bluetooth state machine defines
#define	STATE_INIT		0xff
#define	STATE_QUERY		0xfe

#define	STATE_0			0
#define	STATE_IDLE		10
#define	STATE_KPRESS	11
#define	STATE_KHOLD		12
#define	STATE_KREL		13
#define	WR_HOLD_PER		200
#define	WR_ABRT_PER		1000

// wired remote state machine defines
//#define	STATE_INIT		0xff
//#define	STATE_QUERY		0xfe
#define	K_HOLD			1							// is hold command
#define	K_REL			2							// is release cmd
#define	K_NULL			3							// is release cmd
//#define	STATE_IDLE		10

#define	PREAMB	'~'									// wired-remote preamble

// IC901A Control Head Key Address defines
#define	V_M				0x10
#define	CALL			0x11
#define	BAND			0x12
#define	MODE			0x13
#define	MHZ				0x14
#define	H_L				0x15
#define	SQUP			0x16
#define	SQDN			0x17

#define	SUB				0x1e
#define	M_S				0x1d
#define	MW				0x1c
#define	SET				0x1b
#define	TS				0x1a
#define	TSQ				0x19
#define	VOLUP			0x18
#define	VOLDN			0x1f

#define	SMUTE			0x20
#define	LOCK			0x22
#define	CHECK			0x23

//------------------------------------------------------------------------------
// public Function Prototypes
//------------------------------------------------------------------------------
U8 process_WR(U8 cmd);

//------------------------------------------------------------------------------
// global defines
//------------------------------------------------------------------------------
