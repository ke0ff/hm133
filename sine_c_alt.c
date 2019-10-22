/****************************************************************************************
 ****************** COPYRIGHT (c) 2019 by Joseph Haas (DBA FF Systems)  *****************
 *
 *  File name: sine_c_alt.c
 *
 *  Module:    Control
 *
 *  Summary:   Stored sine array
 *	License and other legal stuff:
 *			   This software, comprised of all files contained in the original distribution archive,
 *				are protected by US Copyright laws.  The files may be used and modified by the person
 *				receiving them under the following terms and conditions:
 *				1) The software, or any protion thereof may not be distributed to any 3rd party by
 *					the recipient or any agent or assign of the recipient.
 *				2) The recipient assumes all risks for the use (or mis-use) of this software
 *
 *
 *  Project scope revision history:
 *    07-24-19 jmh:  Project origin, copied from Orion
 *
 ***************************************************************************************/

/****************************************************************************************
 *  File scope revision history:
 *  07-24-19 jmh:	creation date
 *
 ***************************************************************************************/

#include "typedef.h"
// sine lookup table
// size =   256, min =     1, max =   254 
// MID = 127.500000 SWING = 126.500000
code const U8	SINE[256] = {
	  128,   131,   134,   137,   140,   143,   146,   149,   152,   155,   158,   161,   164,   167,   170,   173,
	  176,   179,   182,   184,   187,   190,   193,   195,   198,   200,   203,   205,   208,   210,   212,   215,
	  217,   219,   221,   223,   225,   227,   229,   231,   233,   234,   236,   238,   239,   240,   242,   243,
	  244,   246,   247,   248,   249,   249,   250,   251,   252,   252,   253,   253,   253,   254,   254,   254,
	  254,   254,   254,   254,   253,   253,   253,   252,   252,   251,   250,   249,   249,   248,   247,   246,
	  244,   243,   242,   240,   239,   238,   236,   234,   233,   231,   229,   227,   225,   223,   221,   219,
	  217,   215,   212,   210,   208,   205,   203,   200,   198,   195,   193,   190,   187,   184,   182,   179,
	  176,   173,   170,   167,   164,   161,   158,   155,   152,   149,   146,   143,   140,   137,   134,   131,
	  127,   124,   121,   118,   115,   112,   109,   106,   103,   100,    97,    94,    91,    88,    85,    82,
	   79,    76,    73,    71,    68,    65,    62,    60,    57,    55,    52,    50,    47,    45,    43,    40,
	   38,    36,    34,    32,    30,    28,    26,    24,    22,    21,    19,    17,    16,    15,    13,    12,
	   11,     9,     8,     7,     6,     6,     5,     4,     3,     3,     2,     2,     2,     1,     1,     1,
	    1,     1,     1,     1,     2,     2,     2,     3,     3,     4,     5,     6,     6,     7,     8,     9,
	   11,    12,    13,    15,    16,    17,    19,    21,    22,    24,    26,    28,    30,    32,    34,    36,
	   38,    40,    43,    45,    47,    50,    52,    55,    57,    60,    62,    65,    68,    71,    73,    76,
	   79,    82,    85,    88,    91,    94,    97,   100,   103,   106,   109,   112,   115,   118,   121,   124
	};
