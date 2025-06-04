/*

Chris Chung August 2021, simpleavr@gmail.com
nonpariel physical (NP) is an standalone calculator microcode emulator;

Compile w/ (example only, substitute w/ your setup path)

/msp430-gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=np25.map,--cref -Wl,--relax -Wl,--gc-sections -I /cygdrive/c/mspgcc-20120406-p20120502/bin/../msp430/include -mmcu=msp430g2553 -o np25.elf np25.c

Changes
250521 align timeout to 60 secs
250522 'h' key toggles hint display, unit startup shows rom model
250528 introduce full_hint() to show setup information
250601 show_hint() for briefly showing rom model and slide switch change
250603 timeout now about 120 secs

September 2021, cc

. based on TI msp430g2744 mcu, 32kB flash, 1kB ram
. based on work from Eric Smith's nonpariel
. emualtes spice core
. this is a hobby project not suitable for any use.
. please observe same GPL licensing terms as the original Nonpariel package
. notice from orignal Nonpareil package

todo
. brightness control via + - keys
. assign double press keys to save registers to flash

Copyright 1995, 2003, 2004, 2005 Eric L. Smith <eric@brouhaha.com>

Nonpareil is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.  Note that I am not
granting permission to redistribute or modify Nonpareil under the
terms of any later version of the General Public License.

Nonpareil is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (in the file "COPYING"); if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111, USA.
*/

/*
auto-sleep does not work for 37E and 38C ROM
*/

//#define C_SPICE
//#define EMBEDDED

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#ifdef EMBEDDED
#ifdef EMBEDDED

//#define G2955
#define G2744
#define MHZ	8

#include <msp430.h>

#define ___use_cal_clk(x)	\
BCSCTL1 = CALBC1_##x##MHZ;	\
DCOCTL  = CALDCO_##x##MHZ;

#define __use_cal_clk(x)	___use_cal_clk(x)
//

static volatile uint16_t _ticks = 0;
static volatile uint8_t _clicks = 0;
static volatile uint8_t _brightness = 2;

/*

          MSP430G2744
       -----------------    
      |                 |  
      |              RST|---[ 47k ]-----VCC  
      |         AVCC+VCC|---------------VCC
      |                 | 
      |     digit 0 P1.0|--\
      |     digit 1 P1.1|--\       digits bus (11), D10 shows negative sign only
      |     digit 2 P1.2|-----//--+---------+--------------------+
      |     digit 3 P1.3|--/      |         ~  D10, sign led     ~ D0-D9, 2 x 5 digit LED module
      |     digit 4 P1.4|--/      |         |                    |
      |     digit 5 P1.5|--/      |         +      +-----------+ +-----------+
      |     digit 6 P1.6|--/      |        /_\     | % % % % % | | % % % % % |--+
      |     digit 7 P1.7|--/      ~         |      +-----------+ +-----------+  |
      |     digit 8 P2.4|--/      |                                             |
      |     digit 9 P2.5|--/      | digits bus x 3 scan lines                   |
      |    digit 10 P2.6|--/      +-----+-----+---/ ~~ /--+-----+-----+-----+   |
      |                 |     _=_ | _=_ | _=_ | _/ ~~ /=_ | _=_ | _=_ | _=_ |   |
	  |      scan X P4.0|-----o o-+-o o-+-o o-+-/ ~~ /o o-+-o o-+-o o-+-o o-+   |
	  |      scan Y P4.1|-----o o-+-o o-+-o o-+/ ~~ /-o o-+-o o-+-o o-+-o o-+   |
	  |      scan Z P4.2|-----o o-+-o o-+-o o-/ ~~ /+-o o-+-o o-+-o o-+-o o-+   |
      |                 |                                                       ~
	  |                 |      (tactile buttons)                                |
      |                 |                                                       |
      |  segment a  P3.0|-------------------------------------------------------|
      |  segment b  P3.1|-------------------------------------------------------|
      |  segment c  P3.2|-------------------------------------------------------|
      |  segment d  P3.3|-------------------------------------------------------|
      |  segment e  P3.4|-------------------------------------------------------|
      |  segment f  P3.5|-------------------------------------------------------|
      |  segment g  P3.6|-------------------------------------------------------|
      |  segment h  P3.7|-------------------------------------------------------|
      |                 | 
      |         AVSS+VSS|--+
      +-----------------+  |
                          _|_
                          ///




   21          22
   25 26 27 14 23
   29 30 31 15 13
   33    34 35 17
   37 38 39    18
   01 02 03    19
   05 06 07    41
   09 10 11    42


*/
//#define FIXPCB    // temporary as i messed up led module position on test pcb

static const uint8_t digit_map_p1[] = 
#ifdef FIXPCB
	{ 1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7, 0x00, 0x00, 0x00, 0x00, };//fixpcb
#else
	{ 1<<5, 1<<6, 1<<7, 0x00, 0x00, 1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 0x00, 0x00, };
#endif

static const uint8_t digit_map_p2[] = 
#ifdef FIXPCB
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 1<<4, 1<<5, 1<<6, 0x00, };
#else
	{ 0x00, 0x00, 0x00, 1<<4, 1<<5, 0x00, 0x00, 0x00, 0x00, 0x00, 1<<6, 0x00, };
#endif
/*
static const uint8_t digit_map_p3[] = {
	1<<0, 1<<1, 1<<2, 1<<3,
	1<<4, 1<<5, 1<<6, 1<<7,
	0x00, 0x00, 0x00, 0x00,
};

static const uint8_t digit_map_p2[] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	1<<4, 1<<5, 1<<6, 1<<7,
};
*/

//#define CLK_LOW	P2OUT &= ~BIT2
//#define CLK_HI	P2OUT |= BIT2

#define SPIN	P4IN
#define SPOUT	P4OUT
#define SPREN	P4REN
#define SPDIR	P4DIR

#define SCANX	BIT0
#define SCANY	BIT1
#define SCANZ	BIT2
#define ALL_SCANS	(SCANX|SCANY|SCANZ)

#define SEG_A	BIT0
#define SEG_B	BIT1
#define SEG_C	BIT2
#define SEG_D	BIT3
#define SEG_E	BIT4
#define SEG_F	BIT5
#define SEG_G	BIT6
#define SEG_H	BIT7


#define ALL_DIGITS_P1	\
   (digit_map_p1[0]|digit_map_p1[1]|digit_map_p1[2]|digit_map_p1[3]|	\
	digit_map_p1[4]|digit_map_p1[5]|digit_map_p1[6]|digit_map_p1[7]|	\
	digit_map_p1[8]|digit_map_p1[9]|digit_map_p1[10]|digit_map_p1[11])
#define ALL_DIGITS_P2	\
   (digit_map_p2[0]|digit_map_p2[1]|digit_map_p2[2]|digit_map_p2[3]|	\
	digit_map_p2[4]|digit_map_p2[5]|digit_map_p2[6]|digit_map_p2[7]|	\
	digit_map_p2[8]|digit_map_p2[9]|digit_map_p2[10]|digit_map_p2[11])

static const 
//__attribute__ ((section (".data_fe"))) 
uint8_t seg_map[] = {
   SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F,
   SEG_B|SEG_C,
   SEG_A|SEG_B|SEG_D|SEG_E|SEG_G,
   SEG_A|SEG_B|SEG_C|SEG_D|SEG_G,
   SEG_B|SEG_C|SEG_F|SEG_G,
   SEG_A|SEG_C|SEG_D|SEG_F|SEG_G,
   SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
   SEG_A|SEG_B|SEG_C,
   SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,
   SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G,
   SEG_E|SEG_G,	// r, H/F, o, p, e, blank
//#ifdef C_SPICE
//#else
   SEG_A|SEG_E|SEG_F|SEG_G, 			
//#endif
   SEG_C|SEG_D|SEG_E|SEG_G,
   SEG_A|SEG_B|SEG_E|SEG_F|SEG_G,
   SEG_A|SEG_D|SEG_E|SEG_F|SEG_G,
   0x00,
   SEG_B|SEG_C|SEG_E|SEG_F|SEG_G, 		// for spice as 0xb
   //________________ a to z
   SEG_A|SEG_B|SEG_C|SEG_E|SEG_F|SEG_G,	// A
   SEG_C|SEG_D|SEG_E|SEG_F|SEG_G,		// b
   SEG_D|SEG_E|SEG_G,					// c
   SEG_B|SEG_C|SEG_D|SEG_E|SEG_G,		// d
   SEG_A|SEG_D|SEG_E|SEG_F|SEG_G,		// E
   SEG_A|SEG_E|SEG_F|SEG_G,				// F
   SEG_A|SEG_C|SEG_D|SEG_E|SEG_F,		// G
   SEG_C|SEG_E|SEG_F|SEG_G,				// h
   SEG_E|SEG_F,							// i
   SEG_B|SEG_C|SEG_D,					// j
   SEG_D|SEG_F|SEG_G,					// k
   SEG_D|SEG_E|SEG_F,					// L
   SEG_A|SEG_C|SEG_E,					// m
   SEG_C|SEG_E|SEG_G,					// n
   SEG_C|SEG_D|SEG_E|SEG_G,				// o
   SEG_A|SEG_B|SEG_E|SEG_F|SEG_G,		// P
   SEG_A|SEG_B|SEG_C|SEG_F|SEG_G,		// q
   SEG_E|SEG_G,							// r
   SEG_A|SEG_C|SEG_D|SEG_F|SEG_G,		// S
   SEG_D|SEG_E|SEG_F|SEG_G,				// t
   SEG_C|SEG_D|SEG_E,					// u
   SEG_C|SEG_D|SEG_E|SEG_F,				// v
   SEG_B|SEG_D|SEG_F,					// w
   SEG_B|SEG_C|SEG_E|SEG_F|SEG_G,		// X
   SEG_B|SEG_C|SEG_D|SEG_F|SEG_G,		// y
   SEG_A|SEG_B|SEG_D|SEG_E|SEG_G,		// Z
   SEG_A|SEG_D|SEG_E|SEG_F|SEG_G,		// [
   SEG_A|SEG_B|SEG_F,					// ^
   SEG_A|SEG_B|SEG_C|SEG_D|SEG_G,		// ]
   SEG_C|SEG_F|SEG_G,					// blackslash
   SEG_D,								// _
   SEG_B,								// '
};

/*
raw scancode on pcb version, code is digit * 4 + [123] for [XYZ]

                       [8Z]	            ,35,
   [1X] [3X] [4X] [8X] [8Y]	, 5,13,17,33,34,
   [1Y] [3Y] [4Y] [7Y] [aY] , 6,14,18,30,42,
   [1Z] [3Z] [4Z] [7X] [aX] , 7,15,19,29,41,
   [5Y]  [5Z]   [7Z]   [aZ] ,22,   23,31,43,
   [0Z]  [2Z]   [bZ]   [9Z] , 3,   11,47,39,
   [0X]  [2X]   [bX]   [9X] , 1,    9,45,37,
   [0Y]  [2Y]   [bY]   [9Y] , 2,   10,46,38,
   scan lines X, Y, Z

               22,
   25,26,27,14,23,
	 29,30,31,15,13,
	  33,  34,35,17,
	    37,38,39,18,
	     1, 2, 3,19,
	     5, 6, 7,41,
	     9,10,11,42,

	0x00,0x43,0x42,0x41,0x00, 0xd3,0xd2,0xd1,0x00,0x63,
	0x62,0x61,0x00,0x90,0x31, 0x91,0x00,0x70,0xa0,0x40,
	0x00,0x00,0x00,0x30,0x00, 0x34,0x33,0x32,0x00,0x94,
	0x93,0x92,0x00,0x73,0x72, 0x71,0x00,0xa3,0xa2,0xa1,
	0x00,0xd0,0x60,

keys layout
	// 34c
	"  A    B   GSB   F    G ",	// 0x34, 0x33, 0x32, 0x31, 0x30
	" X.Y  GTO  STO  RCL   h ",	// 0x94, 0x93, 0x92, 0x91, 0x90
	" ETR  ETR  CHS  EEX  CLX",	// 0x73,       0x72, 0x71, 0x70
	"  -     7     8      9  ",	// 0xa3, 0xa2, 0xa1, 0xa0
	"  +     4     5      6  ",	// 0x43, 0x42, 0x41, 0x40
	"  x     1     2      3  ",	// 0xd3, 0xd2, 0xd1, 0xd0
	"  /     0     .     R/S ", // 0x63, 0x62, 0x61, 0x60

#define KEYR_PGM_RUN	35
#define KEYR_F	33
#define KEYR_G	34
#define KEYR_H	42
#define KEYR_CLR	41

*/
#define KEYR_ON				21
#define KEYR_PGM_RUN	22
#define KEYR_F				14
#define KEYR_G				23
#define KEYR_H				13
#define KEYR_CLR			17
#define KEYR_PLUS			1
#define KEYR_MINUS		37

/*
static const uint8_t key_map_34c[] = {
	0x00, 0xd3, 0x43, 0x63, 
	0x00, 0x34, 0x94, 0x73,
	0x00, 0xd2, 0x42, 0x62, 
	0x00, 0x33, 0x93, 0x73,	//0x72,
	0x00, 0x32, 0x92, 0x72, 
	0x00, 0x00, 0xa3, 0xa2,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x71, 0x91, 0xa1,
	0x00, 0x31, 0x30, 0x00, 
	0x00, 0xd0, 0x40, 0x60,
	0x00, 0x70, 0x90, 0xa0,
	0x00, 0xd1, 0x41, 0x61,
};
*/

/*
const uint8_t _34_25map[] = {
	0, 3,11,47, 0, 1, 9,45,
	0, 2,10,46, 0,42,33,30,
	0,41,43,39, 0, 0,35,34,
	0, 5,13,17, 0, 6,14,18,
	0,15,19,29, 0,22,23,31,
	0,37,38, 0,
};
*/

void vlog(char *fmt, ...) { return; }

#else

#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <stdarg.h>

void putstr(const char *p) {
	while (*p) putchar(*p++);
}

#define RB "\e(0\x6a\e(B" // 188 Right Bottom corner
#define RT "\e(0\x6b\e(B" // 187 Right Top corner
#define LT "\e(0\x6c\e(B" // 201 Left Top cornet
#define LB "\e(0\x6d\e(B" // 200 Left Bottom corner
#define MC "\e(0\x6e\e(B" // 206 Midle Cross
#define HL "\e(0\x71\e(B" // 205 Horizontal Line
#define LC "\e(0\x74\e(B" // 204 Left Cross
#define RC "\e(0\x75\e(B" // 185 Right Cross
#define BC "\e(0\x76\e(B" // 202 Bottom Cross
#define TC "\e(0\x77\e(B" // 203 Top Cross
#define VL "\e(0\x78\e(B" // 186 Vertical Line
#define SP " " 		  // space string

int log_on=1;

//----------------------------------------------------------------------
void vlog(char *fmt, ...) {
	va_list arg;

	if (*fmt == '+') log_on = 1;
	if (*fmt == '-') log_on = 0;

	if (!log_on) return;
	if (*fmt == '+') fputs("\n", stderr);
	char buf[32];

	va_start(arg, fmt);
	vsnprintf(buf, 31, fmt, arg);
	fputs(buf, stderr); 

	va_end(arg);
}

void vlog_flag(int bits, uint16_t v) {
	uint16_t c = 1<<(bits-1);
	while (c) {
		fputc(v & c ? '1' : '0', stderr); 
		c >>= 1;
	}//while
}

#endif

/*
   r, H/F, o, p, e, blank
   (25,34) (26,33) (27,32) (14,31) (23,30)
   (29,94) (30,93) (31,92) (15,91) (13,90)
   (33,73)         (34,72) (35,71) (17,70)
   (37,a3)    (38,a2)   (19,a1)    (18,a0)
   ( 1,63)    ( 2,62)   ( 3,61)    (19,60)
   ( 5,d3)    ( 6,d2)   ( 7,d1)    (41,d0)
   ( 9,43)    (10,42)   (11,41)    (42,40)

	// 25
	" SST  BST  GTO   F    G ",	// 0xb3, 0xb2, 0xb1, 0xb0, 0xb4
	" X.Y  RV   STO  RCL  E+ ",	// 0x43, 0x42, 0x41, 0x40, 0x44
	" ETR  ETR  CHS  EEX  CLX",	// 0xd3,       0xd1, 0xd0, 0xd4
	"  -     7     8      9  ",	// 0x63, 0x62, 0x61, 0x60
	"  +     4     5      6  ",	// 0x93, 0x92, 0x91, 0x90
	"  x     1     2      3  ",	// 0x73, 0x72, 0x71, 0x70
	"  /     0     .     R/S ", // 0xa3, 0xa2, 0xa1, 0xa0

   (25,34) (26,33) (27,32) (14,31) (23,30)
   (29,44) (30,43) (31,42) (15,41) (13,40)
   (33,d3)         (34,d2) (35,d1) (17,d0)
   (37,63)    (38,62)   (19,61)    (18,60)
   ( 1,a3)    ( 2,a2)   ( 3,a1)    (19,a0)
   ( 5,73)    ( 6,72)   ( 7,71)    (41,70)
   ( 9,93)    (10,92)   (11,91)    (42,90)

static const uint8_t key_map_25[] = {
	0x00,0xa3,0xa2,0xa1,0x00, 0x73,0x72,0x71,0x00,0x93,
	0x92,0x91,0x00,0x44,0xb0, 0x40,0x00,0xd4,0x60,0xa0,
	0x00,0x00,0x00,0xb4,0x00, 0xb3,0xb2,0xb1,0x00,0x43,
	0x42,0x41,0x00,0xd3,0xd1, 0xd0,0x00,0x63,0x62,0x61,
	0x00,0x70,0x90,
};
*/
static const uint8_t key_map_34c[] = {
	0x00,0x63,0x62,0x61,0x00, 0xd3,0xd2,0xd1,0x00,0x43,
	0x42,0x41,0x00,0x90,0x31, 0x91,0x00,0x70,0xa0,0x60,
	0x00,0x00,0x00,0x30,0x00, 0x34,0x33,0x32,0x00,0x94,
	0x93,0x92,0x00,0x73,0x72, 0x71,0x00,0xa3,0xa2,0xa1,
	0x00,0xd0,0x40,
};

//#define __USE_RAM	32		// our highest model 33c has 32 units
#define __USE_RAM	64			// 34c now uses 64 units

#include "rom_34c.h"
#include "rom_37e.h"
#include "rom_38c.h"

struct ROM {
    const uint8_t *rom;
    const uint8_t *rom_ex;
    const uint8_t *key_map;
    char model[4];      // model number
    char slide[7];      // slide switch labels
    uint8_t msg_pos;    // seems like number of decimal places on startup
    uint8_t is_spice;
};

struct ROM _rom[] = {
    // possible future inclusion of other models
    //{ rom_25c, rom_25c_ex, key_map_25, "25C", 4, 0, },
    //{ rom_33c, rom_33c_ex, key_map_34c, "33C", 4, 1, },
    { rom_34c, rom_34c_ex, key_map_34c, "34C", "PGMRUN", 4, 1, },
    { rom_37e, rom_37e_ex, key_map_34c, "37E", "BGNEND", 2, 1, },
    { rom_38c, rom_38c_ex, key_map_34c, "38C", "DMYMDY", 2, 1, },
};
//const uint8_t *_rom[] = { rom_33c, rom_34c, rom_37e, rom_38c, };
//const uint8_t *_rom_ex[] = { rom_33c_ex, rom_34c_ex, rom_37e_ex, rom_38c_ex, };
static volatile uint8_t _pgm_run=0;

#include "np34.h"

char __attribute__ ((section(".rodata_prg38c"))) prg38c[] = {		// use 0xfb00
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (00)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (01)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (02)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (03)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (04)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (05)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (06)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (07)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (08)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (09)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (10)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (11)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (12)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (13)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (14)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (15)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (16)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (17)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (18)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (19)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (20)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (21)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (22)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (23)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (24)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (25)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (26)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (27)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (28)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (29)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (30)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (31)
 /*
                                           // */
};

char __attribute__ ((section(".rodata_prg34c"))) prg34c[] = {		// use 0xfc00
//const char prg[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (00)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (01)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (02)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (03)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (04)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (05)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (06)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (07)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (08)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (09)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (10)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (11)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (12)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (13)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (14)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (15)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (16)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (17)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (18)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (19)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (20)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (21)
 0xff, 0xea, 0x00, 0x62, 0xcf, 0xff, 0xeb, // (22)
 0x6d, 0xdf, 0x2c, 0x38, 0xe0, 0x60, 0xcf, // (23)
 0x01, 0x6d, 0xef, 0x6d, 0xef, 0xaf, 0x61, // (24)
 0x6b, 0xbc, 0x62, 0xdf, 0x6e, 0xff, 0xe6, // (25)
 0x8b, 0x0f, 0xff, 0x07, 0x0e, 0xe7, 0x0b, // (26)
 0x54, 0x82, 0x06, 0x4b, 0x9f, 0x9f, 0x9f, // (27)
 0xff, 0x08, 0x7a, 0xe8, 0xe9, 0x0b, 0x18, // (28)
 0x81, 0x6d, 0x60, 0xcf, 0xbc, 0x8b, 0x61, // (29)
 0x0a, 0x28, 0x54, 0x80, 0x09, 0x0f, 0xff, // (30)
 0x16, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00, // (31)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (32)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (33)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (34)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (35)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (36)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (37)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (38)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (39)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (40)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (41)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (42)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (43)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (44)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (45)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (46)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (47)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (48)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (49)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (50)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (51)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (52)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (53)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (54)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (55)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (56)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (57)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (58)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (59)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (60)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (61)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (62)
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // (63)
};

#ifdef EMBEDDED
static volatile uint8_t _state=0;
static volatile uint8_t _opt=0;
static volatile uint8_t _key=0;

#define ST_RAM_LOADED 	BIT7
#define ST_KEY_PRESSED	BIT6
#define ST_KEY_RELEASED	BIT5
#define ST_ALPHA_MSG		BIT4
#define ST_HW_SLOW			BIT3
#define ST_HW_TEST			BIT2
#define ST_ROM_BIT1			BIT1
#define ST_ROM_BIT0  		BIT0
#define ST_ROM				(ST_ROM_BIT1|ST_ROM_BIT0)

#define OPT_HINT        BIT0


/*
  base=142
  64+64+64
  4 blocks 112+112+112+112=448
  142 + [224] = 366 used + 32 ram_reserve = 392
  > offset 224
  get block 2+3 (224) write 192 to info BCD, 32 into ram_reserve
*/

// code memory 0xffff-0x8000, 1 segment is 512 bytes
// interrupt v 0xffff-0xffc0
// info memory 0x10ff-0x1000, 1 segment is 64 bytes
// 0xfc00-0xfdff we will use
// G2744 RAM at 0x05ff-0x0200
//________________________________________________________________________________
void from_flash() {
	uint8_t rom = _state&ST_ROM;
	if (rom & 0x01) {
		char *dest = (char*) act_reg->ram;
		char *flash = (char*) (rom==1 ? prg34c : prg38c);
		uint16_t i;
		for (i=0;i<(__USE_RAM*7);i++) *dest++ = *flash++;
	}//if
}
//________________________________________________________________________________
void to_flash() {
	uint8_t rom = _state&ST_ROM;
	if (rom & 0x01) {
		char *src = (char*) act_reg->ram;
		char *flash = (char*) (rom==1 ? prg34c : prg38c);
		FCTL2 = FWKEY+FSSEL0+FN2;
		FCTL1 = FWKEY+ERASE;
		FCTL3 = FWKEY;
		*flash = 0;

		FCTL1 = FWKEY+WRT; 

		uint16_t i;
		for (i=0;i<(__USE_RAM*7);i++) *flash++ = *src++;

		FCTL1 = FWKEY;
		FCTL3 = FWKEY+LOCK; 
	}//if
}

#define _MSG_POS	act_reg->f
#define _MSG_LEN	act_reg->p
#define _EDIT_POS act_reg->key_buf
#define _MSG_PTR	act_reg->rom
#define _LAST_KEY	act_reg->flags

//volatile uint8_t _MSG_POS=0, _MSG_LEN=0;
//volatile const char *_MSG_PTR=0;

/*
hp-25 5
pos 11 digit 0(x).......     11(h)
pos 10 digit 0(x).......10(h)11(p)
pos 2 digit 0(x)...9(h)10(p)11(-)
*/


//static const uint8_t *_msg_ptr = NULL;
static char _msg_buf[11];
//________________________________________________________________________________
uint8_t char_at_digit(uint8_t digit) {			// uppercase not accepted
	return seg_map[_msg_buf[digit] - '0'];
}
//________________________________________________________________________________
void sleep() {
	//_____ shutdown and allow P2.3-4 'C' button to boot
	TA0CCTL0 &= ~CCIE;
	//_____ set all io pins
	P1OUT = 0x00; P1REN = 0x00; P1DIR = 0xff;
	P2OUT = 0x00; P2REN = 0x00; P2DIR = 0xff;
	P3OUT = 0x00; P3REN = 0x00; P3DIR = 0xff;
	P4OUT = 0x00; P4REN = 0x00; P4DIR = 0xff;

	//______ ON button is wired between P4.0 and P1.0
	P4REN&=~BIT0;
	P4OUT|= BIT0;
	P4DIR|= BIT0;

	P1REN|= BIT0;
	P1OUT&=~BIT0;
	P1DIR&=~BIT0;

	__bic_SR_register(GIE);
	P1IE |= BIT0;
	P1IES|= BIT0;
	P1IFG&=~BIT0;

	__bis_SR_register(LPM4_bits+GIE);
	P2IE &= ~BIT5;		// woke up, disable interrupt

	//_____ re-configure io
	P1OUT = ALL_DIGITS_P1;
	P2OUT = ALL_DIGITS_P2;
	P1REN = 0x00;
	P3DIR = P2DIR = 0xff;
	P1DIR = 0xff;

	SPOUT |= ALL_SCANS;
	SPREN |= ALL_SCANS;
	SPDIR &= ~ALL_SCANS;

	TA0CCTL0 |= CCIE;

}


void full_hint() {
    // ..........
    // 34c SLOW H
    strcpy(_msg_buf, _rom[_state&0x03].model);
    _msg_buf[3] = '?';
    if (_state&ST_HW_SLOW) {
        strcpy(_msg_buf+4, "SLOW");
    }//if
    else {
        strcpy(_msg_buf+4, "FAST");
    }//else
    _msg_buf[8] = '?';
    _msg_buf[9] = _opt&OPT_HINT ? 'H' : '?';
    _msg_buf[10] = '?';
    /*
    _msg_buf[5] = '?';
    _msg_buf[6] = '0' + (_key/10);
    _msg_buf[7] = '0' + (_key%10);
    _msg_buf[8] = '\0';
    */
}

void show_hint(const char *p) {
    strcpy(_msg_buf, "???????");
    _msg_buf[7] = *p++;
    _msg_buf[8] = *p++;
    _msg_buf[9] = *p++;
    _state |= ST_ALPHA_MSG;
    _clicks = 20;
}

#define TIMEOUT	(120*60/48)      // 250521 align timeout to 60 secs, now 120 secs

#else
static volatile uint8_t _state=2;
#endif
//________________________________________________________________________________
int main() {

#ifdef EMBEDDED
	WDTCTL  = WDTPW + WDTHOLD; 	// Stop WDT
	__use_cal_clk(MHZ);
	//FCTL2 = FWKEY + FSSEL0 + FN1;	// MCLK/3 for Flash Timing Generator
	FCTL2 = FWKEY + FSSEL0 + FN2;	// for 12Mhz

	TA0CCR0  = MHZ * 1000;		// start w/ 1ms
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_2 + MC_2;	// SMCLK, cont.

	P1SEL = P2SEL = 0;

	P1OUT = ALL_DIGITS_P1;
	P2OUT = ALL_DIGITS_P2;
	P3DIR = P2DIR = 0xff;
	P1DIR = 0xff;

	SPOUT |= ALL_SCANS;
	SPREN |= ALL_SCANS;
	SPDIR &= ~ALL_SCANS;

	__bis_SR_register(GIE);



//#define RAM_OFFSET	(7*9)
//#define RAM_SIZE	(7*7)		// 49 program steps
	woodstock_clear_memory();
	from_flash();
	_state = ST_HW_TEST + ST_ALPHA_MSG;		// enter setup, default to rom 0 (HP34C)
	_key = KEYR_G;
	//return 0;		//deb
    strcpy(_msg_buf, "R?25_05_30");
    _msg_buf[10] = '?';
    //_state |= ST_ALPHA_MSG;
    _clicks = 30;
    //LPM0;
    //uint8_t init = 1;
	while (_state&ST_HW_TEST) {
		//if (init) init = 0;
        //else LPM0;
        LPM0;
		switch (_key) {
            case KEYR_ON:			// auto-off
                //sleep();
                //WDTCTL = 0;		// s/w reset when wakeup
                _key = KEYR_PGM_RUN;
			case KEYR_PGM_RUN: 	// pgm/run key
				_state &= ~ST_HW_TEST; 
				//to_flash();
				break;
			case KEYR_F:	// F key to rotate rom
			    // 00..01, 01..10, 10..11, 11..01
                _state++;
				if ((_state&ST_ROM)==0x03) _state &= ~ST_ROM;
				break;
			case KEYR_G:	// G key to toggle fast and slow cpu
                _state ^= ST_HW_SLOW;
				break;
			case KEYR_H:	// H key to toggle hint
                _opt ^= OPT_HINT;
				break;
            default:
				break;
			case KEYR_MINUS:
				_brightness++;
				_brightness &= 0x07;
				break;
			case KEYR_PLUS:
				_brightness--;
				_brightness &= 0x07;
				break;
		}//switch
        full_hint();
        _state |= ST_ALPHA_MSG;
	}//while
    _state &= ~ST_ALPHA_MSG;		// clear possible sticky message

	woodstock_set_rom(_state&ST_ROM);
	woodstock_new_processor();
	from_flash();
	//_state &= ~(ST_RAM_LOADED|ST_KEY_PRESSED|ST_KEY_RELEASED);
	woodstock_set_ext_flag (3, (_pgm_run=0));		// set run mode
	act_reg->f = _rom[_state&ST_ROM].msg_pos;
#else

	const char overlay[] = "\x1b[13;0H\x1b[32m\
					   |A    | |B    | |gsb  | |F    | |G    |\n\r\n\r\n\r\
					   |x y  | |gto  | |sto  | |rcl  | |H    |\n\r\n\r\n\r\
					   |Enter        | |chs  | |eex  | |clx  |\n\r\n\r\n\r\
					   |-     |  |7     |  |8     |   |9     |\n\r\n\r\n\r\
					   |+     |  |4     |  |5     |   |6     |\n\r\n\r\n\r\
					   |*     |  |1     |  |2     |   |3     |\n\r\n\r\n\r\
					   |/     |  |0     |  |      |   |R/S   |\n\r\n\r\n\r";

	const char overlay_f[] = "\x1b[13;0H\x1b[32m\
					   |fix  | |sci  | |eng  | |F    | |G    |\n\r\n\r\n\r\
					   |x.y  | |R^   | |v    | |(i)  | |H    |\n\r\n\r\n\r\
					   |PREFIX |-clear-|pgm  | |reg  | |E    |\n\r\n\r\n\r\
					   |x<y   |  |sin   |  |cos   |   |tan   |\n\r\n\r\n\r\
					   |x>y   |  | >R   |  | >G   |   | >H.ms|\n\r\n\r\n\r\
					   |x!y   |  |ln    |  |log   |   |sqrt  |\n\r\n\r\n\r\
					   |x=y   |  |integr|  |SOLVE |   |E+    |\n\r\n\r\n\r";

	const char overlay_g[] = "\x1b[13;0H\x1b[32m\
					   |deg  | |rad  | |grd  | |     | |     |\n\r\n\r\n\r\
					   |     | |Rv   | |dse  | |isg  | |H    |\n\r\n\r\n\r\
					   |MEM          | |     | |     | |     |\n\r\n\r\n\r\
					   |x<0   |  |sin-1 |  |cos-1 |   |tan-1 |\n\r\n\r\n\r\
					   |x>0   |  | >P   |  | >R   |   | >H   |\n\r\n\r\n\r\
					   |x|0   |  |eX    |  |10^X  |   |X^2   |\n\r\n\r\n\r\
					   |x=0   |  |      |  |      |   |E-    |\n\r\n\r\n\r";

	const char overlay_h[] = "\x1b[13;0H\x1b[32m\
					   |dsp  | |rtn  | |lbl  | |     | |     |\n\r\n\r\n\r\
					   |xy(i)| |del  | |bst  | |sst  | |H    |\n\r\n\r\n\r\
					   |MANT         | |int  | |frac | |abs  |\n\r\n\r\n\r\
					   |%     |  |^%    |  |-x    |   |s     |\n\r\n\r\n\r\
					   |SF    |  |^y    |  |r     |   |L.R   |\n\r\n\r\n\r\
					   |CF    |  |x!    |  |1/x   |   |y^x   |\n\r\n\r\n\r\
					   |F?    |  |lstX  |  |pi    |   |PSE   |\n\r\n\r\n\r";

//________ color keypad, remove end of line escape code to remove color
const char keypad[] = "\x1b[2J\x1b[3;0H\
\n\r\x1b[37m\
					   \n\r\
					   .-------------------------------------.\n\r\
					   |                                     |\n\r\
					   |_____________________________________|\n\r\
					   \n\r\
					    ------                       -------- \n\r\x1b[32m\
					   |ON    |                     |PRGM-RUN|\n\r\x1b[33m\
					                               (=)       |\n\r\x1b[37m\
					    -----   -----   -----   -----   ----- \n\r\x1b[32m\
					   |A    | |B    | |gsb  | |F    | |G    |\n\r\x1b[33m\
					  (a)     (s)     (d)     (f)     (g)     \n\r\x1b[37m\
					    -----   -----   -----   -----   ----- \n\r\x1b[32m\
					   |x y  | |gto  | |sto  | |rcl  | |H    |\n\r\x1b[33m\
					  (z)     (x)     (c)     (v)     (h)     \n\r\x1b[37m\
					    -------------   -----   -----   ----- \n\r\x1b[32m\
					   |Enter        | |chs  | |eex  | |clx  |\n\r\x1b[33m\
					  ( )             (n)     (m)     (,)    |\n\r\x1b[37m\
					    ------    ------    ------     ------ \n\r\x1b[32m\
					   |-     |  |7     |  |8     |   |9     |\n\r\x1b[33m\
					  (-)       (7)       (s)        (t)      \n\r\x1b[37m\
					    ------    ------    ------     ------ \n\r\x1b[32m\
					   |+     |  |4     |  |5     |   |6     |\n\r\x1b[33m\
					  (+)       (4)       (w)        (x)      \n\r\x1b[37m\
					    ------    ------    ------     ------ \n\r\x1b[32m\
					   |*     |  |1     |  |2     |   |3     |\n\r\x1b[33m\
					  (*)       (1)       (=)        (?)      \n\r\x1b[37m\
					    ------    ------    ------     ------ \n\r\x1b[32m\
					   |/     |  |0     |  |      |   |R/S   |\n\r\x1b[33m\
					  (/)       (0)       ( )        (E)      \n\r\x1b[37m\
					    ------    ------    ------     ------ \n\r\x1b[32m\
					   ";

	set_conio_terminal_mode(); 
	woodstock_clear_memory();
	woodstock_set_rom(_state&0x03);
	woodstock_new_processor();
	uint8_t c=0;
	uint8_t key_map[128];
	for (uint8_t i=0;i<128;i++) key_map[i] = 0;

	key_map['p'] = 22;
	key_map['a'] = 25; key_map['s'] = 26; key_map['d'] = 27; key_map['f'] = 14; key_map['g'] = 23;
	key_map['z'] = 29; key_map['x'] = 30; key_map['c'] = 31; key_map['v'] = 15; key_map['h'] = 13;
	key_map[' '] = 33; key_map['n'] = 34; key_map['m'] = 35; key_map[','] = 17;
	key_map['-'] = 37; key_map['7'] = 38; key_map['8'] = 39; key_map['9'] = 18;
	key_map['+'] = 1; key_map['4'] = 2; key_map['5'] = 3; key_map['6'] = 19;
	key_map['*'] = 5; key_map['1'] = 6; key_map['2'] = 7; key_map['3'] = 41;
	key_map['/'] = 9; key_map['0'] = 10; key_map['.'] = 11; key_map[0x0d] = 42;
	key_map['E'] = 42;
	uint8_t release_in=50;

	putstr(keypad);
	putstr("\x1b[?25l");

	int shift=0;

	const char play[] = 
"\0fnhdah*0fa4c0hd90Ec1g6v0*hncfcv1Ehd8gcx8x9hdsh+0fa4c2hd6.999\
cfc0Ehd7gvx7hdsvfchnv2/f6Ex6hd1g6zg6z-v1g6/h2h/0x0v0*E\
xahd0v2*Exs=op";
/*
"\0ofnhda500c650nc760c8hd0v6fa4m4/v7h,+v7g+d4znhEhE\
fa0v8hE3hE2hE1hE0hEhd9v8zf+x6c-82*5-c92/v6+v7+v9\
c+7gxc6hng+x0v7hd7hEx7hd6v82.5-c+6\
2*c+7v610*v7g3+f3nx7hd4znzhv=op";
*/

	int play_pos=0;

	if (!*play) {
		char *dest = (char*) act_reg->ram;
		char *flash = (char*) prg34c;
		uint16_t i;
		for (i=0;i<(__USE_RAM*7);i++) *dest++ = *flash++;
		act_reg->f = _rom[_state&0x03].msg_pos;
		woodstock_set_ext_flag (3, _pgm_run ^= 1);		// pgm-run toggle
	}//if
#endif
	uint8_t done=0, initialized=0;

	while (!done) {
#ifdef EMBEDDED
		if (_state) {
			if (_state & ST_KEY_PRESSED) {
				/*
				if (!(_state&ST_RAM_LOADED)) {
					//______ lets load ram from flash (pretend continous memory) at 1st key
					//       need to do it here after cpu initialized
					if (_pgm_run && ((_state&ST_ROM) != 0x03)) {
						_state |= ST_RAM_LOADED;
						char *src = (char*) 0x1040;
						uint8_t c = RAM_SIZE;
						if (!(_state&ST_ROM_BIT0)) src += 0x40;		// next infomem block for hp25
						while (c--) ((char*) act_reg->ram)[c+RAM_OFFSET] = *(src + c);
					}//if
				}//if
				*/
				//if (_key == (100+KEYR_CLR)) {		// double pressed clear, sleep
				if (_key == KEYR_ON) {					// on key pressed, sleep
					while (_key) __asm("  nop");
					_state &= ~ST_KEY_PRESSED;
					sleep();
                    //uint8_t i;
                    //for (i=0;i<10;i++) _msg_buf[i] = '?';
                    //_state |= ST_ALPHA_MSG;
					_clicks = 5;
					while (_clicks) {
						if (_key == 10) {   // '0'
                            while (_key) __asm("  nop");
                            WDTCTL = 0;		// s/w reset when '0' pressed to wakeup
                        }//if
						if (_key == 11) {	// '.' debug, show ram values
							uint8_t i=0, j, s=0;
							//uint8_t msg[] = "01__001122__33445566__";
							_clicks = 0;
							for (;;) {
								_state &= ~ST_KEY_PRESSED;
                                if (s) {
                                    _msg_buf[0] = '_';
                                    _msg_buf[1] = '_';
                                }//if
                                else {
                                    _msg_buf[0] = i/10 + '0';
                                    _msg_buf[1] = i%10 + '0';
                                    _msg_buf[2] = '_';
                                    _msg_buf[3] = '_';
                                }//else
								uint8_t v=0, p=4;
								for (j=0;j<WSIZE/2;j++) {
									uint8_t r = act_reg->ram[i][j];
                                    uint8_t r1 = (r/0x10) > 0x09 ? r/0x10 + 7 + '0' : r/0x10 + '0';
                                    uint8_t r2 = (r%0x10) > 0x09 ? r%0x10 + 7 + '0' : r%0x10 + '0';
                                    if (!s && p<10) {
                                        _msg_buf[p++] = r1;
                                        _msg_buf[p++] = r2;
                                    }//if
                                    else {
                                        if (s && p>11) {
                                            _msg_buf[(p++)-10] = r1;
                                            _msg_buf[(p++)-10] = r2;
                                        }//if
                                        else {
                                            p += 2;
                                        }//else
                                    }//else
									if (j==2) p += 2;
									if (j>2) v += r;
								}//for
								if (!s && v) _msg_buf[2] = 12 + '0';
								//show_msg((const uint8_t *) (s ? msg+10 : msg), 0);
                                //strcpy(_msg_buf, (char *) (s ? msg+10 : msg));
                                _state |= ST_ALPHA_MSG;
								// wait for a key
								while (!(_state & ST_KEY_PRESSED)) __asm(" nop");
								switch (_key) {
									case 11: s ^= 1; break;		// .
									case 41: if (i) i--; break;	// 3
									case 42: i++; break;		// R/S
									default: break;
								}//switch
								if (_key == 10) break;			// 0
							}//for
							while (_key) __asm(" nop");
                            _state &= ~ST_ALPHA_MSG;
						}//if
						if (_key == 42) {	// 'R/S'
							_clicks = 0;
                            while (1) {
								_state &= ~ST_KEY_PRESSED;
                                _msg_buf[0] = '0' + (_key/10);
                                _msg_buf[1] = '0' + (_key%10);
                                _msg_buf[2] = '_';
                                uint8_t k34 = key_map_34c[_key];
                                _msg_buf[3] = '0' + (k34/16);
                                _msg_buf[4] = '0' + (k34%16);
                                _msg_buf[5] = '?';
                                _state |= ST_ALPHA_MSG;
								while (!(_state & ST_KEY_PRESSED)) __asm(" nop");
                                if (_key == KEYR_PGM_RUN) break;
                            }//while
							while (_key) __asm(" nop");
                            _state &= ~ST_ALPHA_MSG;
                            continue;
                        }//if
					}//while
                    //show_msg((const uint8_t*) "FLASH", 100);
                    if (_opt&OPT_HINT) {
                        show_hint(_rom[_state&0x03].model);
                        _state |= ST_ALPHA_MSG;
                        _clicks = 30;
                    }//if
                    while (_clicks) __asm(" nop");
					continue;
				}//if
				if (_key == (100+KEYR_PGM_RUN)) {		// double press pgm-run, save registers / program to flash
				//if (_key == (100+KEYR_CLR)) {		// double pressed clear, sleep
					to_flash();
					_key -= 100;
                    // show_msg((const uint8_t*) "?MEM?SAVED", 40); 
                    strcpy(_msg_buf, "?MEM?SAVED");
                    _state |= ST_ALPHA_MSG;
                    _clicks = 40;
				}//if
				if (_key == KEYR_PGM_RUN) {
					woodstock_set_ext_flag (3, _pgm_run ^= 1);		// pgm-run toggle
                    if (initialized) {
                        strcpy(_msg_buf, "???????");
                        show_hint(_rom[_state&0x03].slide + _pgm_run*3);
                        /*
                        if (_pgm_run) {
                            _msg_buf[7] = 'R';
                            _msg_buf[8] = 'U';
                            _msg_buf[9] = 'N';
                        }//if
                        else {
                            _msg_buf[7] = 'P';
                            _msg_buf[8] = 'G';
                            _msg_buf[9] = 'M';
                        }//else
                        _state |= ST_ALPHA_MSG;
                        _clicks = 30;
                        */
                    }//if
					//___ to run mode, write flash, only save to flash when instructed
					//if (_pgm_run) to_flash();
				}//if
				else {
					//if (session_key_map[_key]) woodstock_press_key(session_key_map[_key]);
					sim_check_key(_key);
				}//else
				_state &= ~ST_KEY_PRESSED;
			}//if
			else if (_state & ST_KEY_RELEASED) {
				woodstock_release_key();
				_state &= ~ST_KEY_RELEASED;
			}//if
		}//if
        initialized = 1;
		if (!woodstock_execute_instruction()) break;
#else

		//if (kbhit()) {
		if ((!log_on && !release_in && play[play_pos]) || kbhit()) {
			if (shift) {
				shift = 0;
				putstr(overlay);
			}//if
			if (play[play_pos]) {
				c = play[play_pos++];
			}//if
			else {
				c = getch();
			}//else
			//c = getch();
			switch (c) {
				case 27: 
					done = 1; 
					break;
				case '\\': 
					woodstock_new_processor();
					_pgm_run = 0;
					break;
				case '=': 
					_pgm_run ^= 1;
					woodstock_set_ext_flag (3, _pgm_run);		// pgm-run toggle
					//if (_pgm_run) flash_write(0xfc00, (char*)act_reg->ram, 7*__USE_RAM);
					break;
				case 'o':	// debug dump registers
					{
						int i;
						fputs("\n A=", stderr);
						for (i=0;i<14;i++) { fputc("0123456789abcdef"[act_reg->a[i]&0x0f], stderr); }
						fputs("\n B=", stderr);
						for (i=0;i<14;i++) { fputc("0123456789abcdef"[act_reg->b[i]&0x0f], stderr); }
						fputs("\n C=", stderr);
						for (i=0;i<14;i++) { fputc("0123456789abcdef"[act_reg->c[i]&0x0f], stderr); }
						fputs("\n Y=", stderr);
						for (i=0;i<7;i++) { 
							fputc("0123456789abcdef"[act_reg->y[i]>>4], stderr); 
							fputc("0123456789abcdef"[act_reg->y[i]&0x0f], stderr); 
						}
						fputs("\n Z=", stderr);
						for (i=0;i<7;i++) { 
							fputc("0123456789abcdef"[act_reg->z[i]>>4], stderr); 
							fputc("0123456789abcdef"[act_reg->z[i]&0x0f], stderr); 
						}
						fputs("\n T=", stderr);
						for (i=0;i<7;i++) { 
							fputc("0123456789abcdef"[act_reg->t[i]>>4], stderr); 
							fputc("0123456789abcdef"[act_reg->t[i]&0x0f], stderr); 
						}
						fputs("\nM1=", stderr);
						for (i=0;i<7;i++) { 
							fputc("0123456789abcdef"[act_reg->m1[i]>>4], stderr); 
							fputc("0123456789abcdef"[act_reg->m1[i]&0x0f], stderr); 
						}
						fputs("\nM2=", stderr);
						for (i=0;i<7;i++) { 
							fputc("0123456789abcdef"[act_reg->m2[i]>>4], stderr); 
							fputc("0123456789abcdef"[act_reg->m2[i]&0x0f], stderr); 
						}
						fputs("\n f=", stderr);
						vlog_flag(8, act_reg->f);
						fputs("\n p=", stderr);
						vlog_flag(8, act_reg->p);
						fputs("\nflags=", stderr);
						vlog_flag(8, act_reg->flags);
						fputs("\next_flags=", stderr);
						vlog_flag(16, act_reg->ext_flag);
						fputs("\n s=", stderr);
						vlog_flag(16, act_reg->s);
						fprintf(stderr, "\npc=%04x", act_reg->pc);
						fprintf(stderr, "\nsp=%04x", act_reg->sp);
						fprintf(stderr, "\ndel_rom=%04x", act_reg->del_rom);
						fputc('\n', stderr);
					}
					break;
				case 'p':	// debug dump ram
					{
						int i,j;
						fputc('\n', stderr);
						for (i=0;i<__USE_RAM;i++) {
							for (j=0;j<WSIZE/2;j++) fprintf(stderr, " 0x%02x,", act_reg->ram[i][j]);
							fprintf(stderr, " // (%02d)\n", i);
						}//for
					}
					break;
				case 'f':
				case 'g':
				case 'h':
					if (c=='f') putstr(overlay_f);
					if (c=='g') putstr(overlay_g);
					if (c=='h') putstr(overlay_h);
					shift = 1;
				default:
#ifdef C_SPICE
					//c += 128;
#endif
					if (key_map[c]) {
						//woodstock_press_key(key_map[c]);
						vlog("+[%c] %2d %02x", c, key_map[c], key_map_34c[key_map[c]]);
						sim_check_key(key_map[c]);
						release_in = 10;
					}//if
					break;
			}//switch
			c = 0;
		}//if
		if (!woodstock_execute_instruction()) break;
		if (release_in) {
			if (release_in == 1) woodstock_release_key();
			release_in--;
		}//if
#endif

#ifdef EMBEDDED
		if (_state&ST_HW_SLOW) __delay_cycles(MHZ*125);
#else
		//usleep(1000);
#endif
	}//while

#ifndef EMBEDDED
	putstr("\x1b[?25h");
#endif

	return 0;

}

#ifdef EMBEDDED
volatile uint8_t _data=0;
volatile uint8_t _digit=0;
volatile uint8_t _keyscan=0;

//________________________________________________________________________________
void __interrupt_vec(TIMER0_A0_VECTOR) Timer_A (void) {

	P1OUT |= ALL_DIGITS_P1;	 // 74hc164 friedly, all digit off
	P2OUT |= ALL_DIGITS_P2;
	P3OUT = _data;

	static uint8_t key_since=0;
	static uint8_t sav_i=0, sav_key=0;
	if (_digit == 12) {		// key scanning cycle
		uint8_t i;
		if (_key) {
			//_____ check for key release
			P2OUT &= ~digit_map_p2[sav_i];
			P1OUT &= ~digit_map_p1[sav_i];
			if ((SPIN&ALL_SCANS) == ALL_SCANS) {
				if (_key != KEYR_PGM_RUN) _state |= ST_KEY_RELEASED;
				sav_key = _key;
				_key = 0;
			}//if
			__delay_cycles(MHZ*100);
			_ticks = 0;
		}//if
		else {
			for (i=0;i<11;i++) {
				P2OUT &= ~digit_map_p2[i];
				P1OUT &= ~digit_map_p1[i];

				if ((SPIN&ALL_SCANS) != ALL_SCANS) {
					uint8_t key = 0;
#ifdef FIXPCB
					uint8_t j=i;
					if (i<5) {
						j += 5;
					}//if
					else {
						if (i<10) j -= 5;
					}//else
					if (!(SPIN&SCANY)) key = 2+(j<<2);
					else if (!(SPIN&SCANZ)) key = 3+(j<<2);
					else if (!(SPIN&SCANX)) key = 1+(j<<2);
#else
					if (!(SPIN&SCANY)) key = 2+(i<<2);
					else if (!(SPIN&SCANZ)) key = 3+(i<<2);
					else if (!(SPIN&SCANX)) key = 1+(i<<2);
#endif
					// SCAN XYZ BIT 5,4,3
					if (key) {
						//if (_ticks < 500 && sav_key == KEYR_CLR && sav_key == _34_25map[key])
						if (_ticks < 500 && sav_key == key && (sav_key == KEYR_CLR || sav_key == KEYR_PGM_RUN))
							_key = 100 + sav_key;		// quick double pressed
						else
							_key = key;
							//_key = _34_25map[key];
						_state |= ST_KEY_PRESSED;
						_state &= ~ST_ALPHA_MSG;		// clear possible sticky message
						LPM0_EXIT;
						sav_i = i;
						key_since = 0;
						//while ((SPIN&ALL_SCANS) != ALL_SCANS) __asm(" nop");
						break;
					}//if
				}//if
				P2OUT |= digit_map_p2[i];
				P1OUT |= digit_map_p1[i];
			}//for
		}//else
		if (key_since > TIMEOUT) {
			//_key = 100+KEYR_CLR;			// auto-off
			// check if cpu not active, waiting for key press and we can sleep (we only check 34C
			if (_state & ST_HW_TEST || (_state & ST_ROM) != 1 || (act_reg->pc >= 0x430 && act_reg->pc <= 0x43c)) {
				_key = KEYR_ON;			// auto-off
				_state |= ST_KEY_PRESSED;
                if (_state & ST_HW_TEST)
                    LPM0_EXIT;
                else
                    key_since = 0;
			}//if
		}//if
	}//if
	else {
		if (act_reg->flags&F_DISPLAY_ON || _state&(ST_ALPHA_MSG|ST_HW_TEST)) {
			P1OUT &= ~digit_map_p1[_digit];
			P2OUT &= ~digit_map_p2[_digit];
		}//if
		//
		//
		//TA0CCR0 += MHZ * 1000;	// time till next, 1ms per digit
		// at 8Mhz 8000000c = 1s, 80c = 0.000005s = 5ns
		// load balance led brightness
		//segs += 6;	// 7..15
		uint8_t segs=0;
		while (_data) {
			if (_data&1) segs++;
			_data >>= 1;
		}//while
		segs += _brightness;
		TA0CCR0 += MHZ * segs * 50;
	}//else

	//______ prepare for next digit
	if (_digit>=12) {
		_digit = 0;
		TA0CCR0 += MHZ * 50;	// for quick turn-around
	}//if
	else {
		_digit++;
		// less frequent key scanning for led brightness
		if (_digit==12 && _keyscan++&0x03) _digit = 0;
	}//else


	//if (_key && !digit) _data = seg_map[_key/3];
	//if (_key) _data = seg_map[0xe];

	if (_digit == 12) {
		_data = 0;		// key scan cycle, blank all segments
	}//if
	else {
		if (_state&ST_ALPHA_MSG) {
			//_data = _alpha_msg[_digit];
			_data = char_at_digit(_digit);
		}//if
		else {
            /*
			if (_state&ST_HW_TEST)
				_data = hwtest_load_segments(_digit);
			else
            */
				_data = sim_load_segments(_digit);
		}//else
	}//else
		
	_ticks++;
	if (!(_ticks&0x07ff)) key_since++;      // 0.8sec
	if (!(_ticks&0x003f) && _clicks) {      // 0.025sec
		_clicks--;
		//if (!_clicks) LPM0_EXIT;
		if (!_clicks) {
			_state &= ~ST_ALPHA_MSG;
			LPM0_EXIT;
		}//if
	}//if
	//
}

//________________________________________________________________________________
void __interrupt_vec(PORT1_VECTOR) PORT1_ISR(void) {
	P1IFG &= ~BIT0;
	_BIC_SR_IRQ(LPM4_bits); 
} 
#endif

