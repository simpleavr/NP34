/*

Chris Chung August 2021,2025 simpleavr@gmail.com
nonpareil physical (NP) is an standalone calculator microcode emulator;

## NP-34 Calculator Emulator
Nonpareil Physical (NP) is an standalone calculator microcode emulator

### Features
* selectable rom from HP woodstock and spice series units. 21, 22, 25, 27, 29C, 31E, 32E, 33C, 34C, 37E, 38C
* Setup is done by turning on the unit with [0] pressed, and use [f][g][h] to toggle options
* [+][-] to adjust brightness, [PGM/RUN] exit setup
* setup allows choice of ROM to use [g], set speed (fast / slow) [f] and sleep timeout of either 2 or 30 minutes [h]
* secret message / greetings by turining on the unit with [.] pressed, [R/S] to start edit message, other keys to dismiss
* register contents / program steps can be store in flash (press [PGM/RUN] key twice in quick succession) in their own spaces
* each model has its own flash space for register / program retention (via double keying [PGRM/RUN])
* all models share the same RAM space, switching between models are hard resets with RAM clearing

**Setup**
* [0]+[ON] enter **setup**
* [G] cycles 21/22/25/27/29C/31E/32E/33C/34C/37E/38C emulation
* [F] toggles slow/fast cpu
* [H] toggles 2 minutes or 30 minutes sleep time
* [+][-] adjust brightness
* [PGM/RUN] exit setup
* [.]+[ON] enter **secret message / greeting**
* [R/S] starts editing and advances digit position
* [0]..[9] to enter digits and letters like telephone keypad
* any other key dismiss greeting mode
* [R/S]+[ON] to show current programming password
* [R/S] cycles to next non 0xff location + value
* any other key to dismiss

**Compile w/ (example only, substitute w/ your setup path)**

`/usr/local/ti/msp430-gcc/bin/msp430-elf-gcc -D EMBEDDED -Wall -I /usr/local/ti/msp430-gcc/bin/../include -mmcu=msp430g2955 -Os -g -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,--relax -Wl,--gc-sections,--section-start=.rodata_factory=0x0f040,--section-start=.rodata_greetings=0x0f080,--section-start=.rodata_noerase=0x0ffde -L /usr/local/ti/msp430-gcc/bin/../include -T msp430g2955.ld -c -o np34.o np34.c`

### Changes included in firmware 01
**250724** fix 29C startup fix 2 decimal display
**250722** add expose programming password function
**250722** fix initial greetings

### Changes included in firmware 00
**250714** add 2 digit firmware version _ver
**250623** add greetings / secret message
**250622** re-order buttons for power-up setup
**250620** timeout selectable, 2 min or 30 min, allow 1 more cycle for decimal point to increase brightness, postphone flash to ram loading to after 1st key hit upon startup
**250521** align timeout to 60 secs
**250522** 'h' key toggles hint display, unit startup shows rom model
**250528** introduce full_hint() to show setup information
**250601** show_hint() for briefly showing rom model and slide switch change
**250603** timeout now about 120 secs

### September 2021, original notes
* based on TI msp430g2744 mcu, 32kB flash, 1kB ram, replaced w/ g2955 in 2025
* based on work from Eric Smith's nonpareil
* emualtes spice core
* this is a hobby project not suitable for any use.
* please observe same GPL licensing terms as the original Nonpareil package
* notice from orignal Nonpareil package

### Firmware updates
* an IO header block on the top left of the NP-34 unit facilitates firmware updates
* firmware flashing can be done via SBW (Spy-By-Wire) or BSL (BootStrap Loader) mechanisms
* SBW requires more expensive hardware (TI LaunchPads), is quicker and more secure
* BSL requires less expensive headware (USB-TTL dongles), is slower and less secure
* **there may be / are other ways to flash firmware on the MSP430 MCUs, I am only mentioning those that I am familiar with**

```
  +---------------------------+
  |  ...Display.............  |  
  |                           |  the 2x4 header pins on the right size of the unit
  |    o o R  /      \        |  is for firmware flashing and I/O purpose
  |    o o T | CR2032 |       |  left column 4 pins are Rx, Tx (IO) and Rx, Tx (BSL)
  | Rx o o +  \      /        |  the IO receive and transmit pins are for future development
  | Tx o o G                  |  the BSL Rx and Tx pins are needed for BSL firmware flashing
  |                           |  right column 4 pins are (R)eset, (T)est, (+)power and (G)round
  |                           |  these pins are needed for both BSL and SBW programming

```

* each firmware release contains 4 files, example for version 00
  `ex. np34-00.hex (for SBW), np34-00.txt, PASS-00.txt and np34-00np.txt (for BSL)`
* firmware files reside in **firmware** folder in source tree

**Firmware flashing procedure using TI LaunchPad**
* you can use a Texas Instructment LaunchPad development board to flash firmware via the SBW (Spy-By-Wire) protocol
* most LaunchPads should be able to flash the NP-34, except for the low end MSP430G2ET, tested OK LaunchPads include FR2433 and FR5529
* connection is done through 4 dupont wires with optional pin headers on the NP-34
* LaunchPads are development boards, for NP-34 firmware flashing, we are using the programming side of the LaunchPads
* Disconnect the MCU side from the programming side of the LaunchPad by removing the connecting jumper caps
* Locate the 4 connecting points (Ground, Vcc, Test, Reset) and run dupont wires between LaunchPad (programmer side) and NP-34
* LaunchPad RESET and TEST pins are also called SBWTDIO (RESET) and SBWTCLK (TEST) pin

```
                        +---------------------------+
                        |  ...Display.............  |
  LaunchPad  NP-34      |                           |
  RESET  to  RESET (R)  |    o o R  /      \        |
  TEST   to  TEST  (T)  |    o o T | CR2032 |       |
  VCC    to  VCC   (+)  | Rx o o +  \      /        |
  GND    to  GND   (G)  | Tx o o G                  |

```

* google "ti msp flasher" to locate software
* download and install "MSPFLASHER", from TI
* after installation, it should reside in C:\ti\MSPFlasher_1.3.20\MSP430Flasher.exe
* ex. to flash version 01 firmware use the following command
* C:\ti\MSPFlasher_1.3.20\MSP430Flasher.exe -n MSP430G2X55 -w np34-01.hex -j fast -z [VCC]

**Firmware update using USB-TTL dongles**
* use at own risk, there is a chance to brick your unit
* there may be other opensource software that could work w/ the MSP430G2955
* google "TI BSL Scripter" to locate software
* download "BSL Scripter", from TI
* follow instruction to install, need to use the legacy BSLDEMO.EXE
* after installation, it should reside in C:\ti\BSL-Scripter\BSLDEMO.exe
* also requires a USB-TTL dongle, and it must have DTR RTS lines
* tested working are the ftdi, cp210x, ch34x variety
* connect USB-TTL dongle and NP-34 w/ 6 dupont wires, you may also need header pins

```
  Dongle   NP-34        +---------------------------+
  RX    to BSL  (Tx)    |  ...Display.............  |
  TX    to BSL  (Rx)    |                           |
  3.3V  to VCC   (+)    |    o o R  /      \        |
  GND   to GND   (G)    |    o o T | CR2032 |       |
  DTR   to RESET (R)    | Rx o o +  \      /        |
  RTS   to TEST  (T)    | Tx o o G                  |
```

**Normal firmware flashing procedure using USB-TTL dongle**
* for normal upgrade, you need the current version firmware file and password file of the last version
* ex. to flash version 01 firmware use the following command
* C:\ti\BSL-Scripter\BSLDEMO.exe -tUSB serial -cCOM? -pPASS-00.txt -m1 +aepvr np34-01.txt
* COM? port depends on your own system setup and dongle
* if you cannot supply the correct previous version PASS-??.txt as password, flashing will fail
* previous version should be the version of firmware currently reside in your NP-34 unit
* normal firmware flashing w/ password will maintain your NP-34 unit w/ factory calibrated timing

**Force firmware flashing procedure using USB-TTL dongle**
* used to recovered when you bricked your NP-34 unit, or you can't provide corrent password file of current firmware
* ex. to flash version 01 firmware use the following command
* C:\ti\BSL-Scripter\BSLDEMO.exe -tUSB serial -cCOM? -m1 +epvr np34-01np.txt
* force firmware flashing w/o password will replace your NP-34 unit w/ non-optimal generic timing

**Password used for BSL programming**
* the MSP430 MCU used in NP-34 has IP protection and can only be programmed with a 32 byte password
* the password is actually the current programmed interrupt vector table
* to upgrade from one firmware version to another, you can supply the PASS-??.txt from the last (or current) version of firmware
* since firmware 01, the [R/S]+[ON] key wll show the current programming password, and you can create a correct password file PASS.txt to be used for BSL programming
* an address + value pair will be shown like so "FFE4 76E5", [R/S] key will cycle to the next pair
* values between address 0xffe0 to 0xffff (aka interrupt vector table) is need as password, no show address slots are valued as FFFF
* use these information to create a PASS.txt file and used to update firmware via BSL
* example: [R/S]+[ON] functions shows "FFE4 76E5", "FFF2 8EE2" and "FFFE 08C9", your should create your a PASS.txt file like

```
@ffe0
FF FF FF FF 76 E5 FF FF FF FF FF FF FF FF FF FF
FF FF 8E E2 FF FF FF FF FF FF FF FF FF FF 08 C9
q
```

------------------------------------------------------------------


part of NP-34's code were derived from Nonpareil, please observe Nonpareil license

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

--------

### Hardware

**Parts list**
* msp430g2955
* 2x CL25011AH LED Module
* 32× Tactile Button 6x3mm
* 1x CR2032 SMD/TH battery holder
* 1x 47k resistor
* 1x 100nF (104) capacitor (optional)

**Schematic**
```
    * runs off 3V button cell

          MSP430G2955
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

```

tail -n +5 $prg.c | sed -n '/^tail/q;p' > README.md");
*/

/*
auto-sleep does not work for 37E and 38C ROM?? verify
todo/requests
. review PSE or R/S on fast mode, should pause more?
. provide setup option to allow for no timeout - rejected
. instead of automatically do from_flash(), could have a key-sequence to re-load


*/

//#define C_SPICE
//#define EMBEDDED

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef EMBEDDED

//#define MEM_INIT
#define G2955
//#define G2744
#define MHZ	8

#include <msp430.h>

#define ___use_cal_clk(x)	\
BCSCTL1 = CALBC1_##x##MHZ;	\
DCOCTL  = CALDCO_##x##MHZ;

#define __use_cal_clk(x)	___use_cal_clk(x)

const char _ver[] = "01";
//

//#define DEBUG_STEP
//#define KEYTEST
//#define REGISTER_DUMP
//#define MEMORY_DUMP

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
#define KEYR_PGM_RUN	    22
#define KEYR_F				14
#define KEYR_G				23
#define KEYR_H				13
#define KEYR_CLR			17
#define KEYR_PLUS			1
#define KEYR_MINUS		    37
#define KEYR_ZERO	        10
#define KEYR_DOT            11
#define KEYR_RS	            42

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

	if (*fmt != '+') return;
    static int threshold = 2000;
    if (!threshold) return;
    threshold--;
	if (!(threshold%10)) fputs("\n", stderr);
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
*/
#ifdef G2955
#endif
static const uint8_t key_map_21[] = {
	0x00, 0xa4, 0xa3, 0xa2, 0x00, 0x74, 0x73, 0x72, 0x00, 0x94, 
	0x93, 0x92, 0x00, 0x40, 0xb1, 0x41, 0x00, 0xd0, 0x61, 0xa1, 
	0x00, 0x00, 0x00, 0xb0, 0x00, 0xb4, 0xb3, 0xb2, 0x00, 0x44, 
	0x43, 0x42, 0x00, 0xd4, 0xd2, 0xd1, 0x00, 0x64, 0x63, 0x62, 
	0x00, 0x71, 0x91, 
};

static const uint8_t key_map_2x[] = {
	0x00, 0xa3, 0xa2, 0xa1, 0x00, 0x73, 0x72, 0x71, 0x00, 0x93, 
	0x92, 0x91, 0x00, 0x44, 0xb0, 0x40, 0x00, 0xd4, 0x60, 0xa0, 
	0x00, 0x00, 0x00, 0xb4, 0x00, 0xb3, 0xb2, 0xb1, 0x00, 0x43, 
	0x42, 0x41, 0x00, 0xd3, 0xd1, 0xd0, 0x00, 0x63, 0x62, 0x61, 
	0x00, 0x70, 0x90, 
};

static const uint8_t key_map_3x[] = {
	0x00, 0x63, 0x62, 0x61, 0x00, 0xd3, 0xd2, 0xd1, 0x00, 0x43, 
	0x42, 0x41, 0x00, 0x90, 0x31, 0x91, 0x00, 0x70, 0xa0, 0x60, 
	0x00, 0x00, 0x00, 0x30, 0x00, 0x34, 0x33, 0x32, 0x00, 0x94, 
	0x93, 0x92, 0x00, 0x73, 0x72, 0x71, 0x00, 0xa3, 0xa2, 0xa1, 
	0x00, 0xd0, 0x40, 
};

// virgin state signature, put it in 0xfc00, power on will check and initialize RAMS
char __attribute__ ((section(".rodata_factory"))) factory[] = "FACTORY";
char __attribute__ ((section(".rodata_greetings"))) greetings[] = "THANK?YOU?";
uint16_t __attribute__ ((section(".rodata_noerase"))) noerase = 0x0000;

//#define __USE_RAM	32		// our highest model 33c has 32 units
#define __USE_RAM	64			// 34c now uses 64 units

#include "rom.h"

struct ROM {
    const uint8_t *rom;
    const uint8_t *rom_ex;
    const uint8_t *part;
    const uint8_t *part_ex;
    const uint8_t *key_map;
    uint8_t flash;          // registers / program steps save to flash
    char model[4];          // model number
    char slide[7];          // slide switch labels
    uint8_t use_ram;        // number of register memory
    uint8_t msg_pos;        // seems like number of decimal places on startup
    uint8_t is_spice;
};

/*    
   rom, ram sizes
   21   1   0
   22   2   16
   25c  2   16
   27   3   16
   29c  4   48
   31e  2   -
   32e  3.5 32
   33c  4   32
   34c  7   64
   37e  2.5 48
   38c  5   -
*/
const struct ROM _rom[] = {
#ifdef G2955
    // also --section-start=.rodata_factory=0x0f040,--section-start=.rodata_greetings=0x0f080
  { rom_21, rom_21_ex, 0, 0, key_map_21, 0, "21?", "DEGRAD", 0, 2, 0, },
  { rom_22, rom_22_ex, 0, 0, key_map_2x, 0xf2, "22?", "BGNEND", 16, 2, 0, },
  { rom_25, rom_25_ex, 0, 0, key_map_2x, 0xf3, "25?", "PGMRUN", 16, 2, 0, },
  { rom_27, rom_27_ex, 0, 0, key_map_2x, 0xf6, "27?", "", 16, 2, 0, },
  { rom_29c, rom_29c_ex, 0, 0, key_map_2x, 0xf4, "29C", "PGMRUN", 48, 2, 0, },
  { rom_31e, rom_31e_ex, rom_1820_2105, rom_1820_2105_ex, key_map_3x, 0xf7, "31E", "", 32, 4, 1, },
  { rom_32e, rom_32e_ex, rom_1820_2162, rom_1820_2162_ex, key_map_3x, 0xf8, "32E", "", 32, 4, 1, },
  { rom_33c, rom_33c_ex, rom_1820_2105, rom_1820_2105_ex, key_map_3x, 0xf9, "33C", "PGMRUN", 32, 2, 1, },
  { rom_34c, rom_34c_ex, rom_1820_2162, rom_1820_2162_ex, key_map_3x, 0xfa, "34C", "PGMRUN", 64, 4, 1, },
  { rom_37e, rom_37e_ex, rom_1820_2122, rom_1820_2122_ex, key_map_3x, 0, "37E", "BGNEND", 0, 2, 1, },
  { rom_38c, rom_38c_ex, rom_1820_2162, rom_1820_2162_ex, key_map_3x, 0xfc, "38C", "DMYMDY", 48, 2, 1, },
  /*
  */
#else
  // include some of the above roms as G2774 won't fit all
  { rom_33c, rom_33c_ex, rom_1820_2105, rom_1820_2105_ex, key_map_3x, 0xf9, "33C", "PGMRUN", 32, 2, 1, },
  { rom_34c, rom_34c_ex, rom_1820_2162, rom_1820_2162_ex, key_map_3x, 0xfa, "34C", "PGMRUN", 64, 4, 1, },
  { rom_37e, rom_37e_ex, rom_1820_2122, rom_1820_2122_ex, key_map_3x, 0, "37E", "BGNEND", 0, 2, 1, },
  { rom_38c, rom_38c_ex, rom_1820_2162, rom_1820_2162_ex, key_map_3x, 0xfc, "38C", "DMYMDY", 48, 2, 1, },
#endif
};


#define NUM_ROMS  (sizeof(_rom) / sizeof(_rom[0]))
//const uint8_t *_rom[] = { rom_33c, rom_34c, rom_37e, rom_38c, };
//const uint8_t *_rom_ex[] = { rom_33c_ex, rom_34c_ex, rom_37e_ex, rom_38c_ex, };
static volatile uint8_t _pgm_run=0;

#include "np34.h"

#ifdef EMBEDDED
static volatile uint8_t _state=0;
static volatile uint8_t _opt=0;
static volatile uint8_t _key=0;
static volatile uint8_t _blink=0;

#define ST_RAM_LOADED 	    BIT7
#define ST_KEY_PRESSED	    BIT6
#define ST_KEY_RELEASED	    BIT5
#define ST_NOT_USED 		BIT4

#define ST_ROM_BIT3			BIT3
#define ST_ROM_BIT2			BIT2
#define ST_ROM_BIT1			BIT1
#define ST_ROM_BIT0  		BIT0
#define ST_ROM				(BIT3|BIT2|BIT1|BIT0)

#define OPT_WRITE_GREETINGS BIT4
#define OPT_ALPHA_MSG     BIT3
#define OPT_HW_SLOW       BIT2
#define OPT_HW_TEST       BIT1
#define OPT_LONG_TIMEOUT  BIT0


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
    //if (rom != 4 && _rom[rom].flash) {
    if (_rom[rom].flash) {
        char *ram = (char*) act_reg->ram;
        char *flash = (char *) ((uint16_t) _rom[rom].flash << 8);
        uint16_t i;
        for (i=0;i<(_rom[rom].use_ram*7);i++) *ram++ = *flash++;
    }//if
}
//________________________________________________________________________________
void to_flash() {
    /*
       f2 32, << f3->(ram+(32*7)), >> f2, >> f3 (from*)
       f3 32, << f2->(..........), 
       f4 64
    */
	uint8_t rom = _state&ST_ROM;
    if (_rom[rom].flash) {
        char *ram = (char*) act_reg->ram;
        char *flash = (char *) ((uint16_t) _rom[rom].flash << 8);
        uint8_t use_ram = _rom[rom].use_ram;
        uint16_t i;
        if (use_ram <= 32) {
            // half-segment used, read the other half before erasing whole segment,
            // write back later
            char *s = flash;
            char *d = ram + (32*7);
            if (_rom[rom].flash & 0x01) s -= 0x100;
            else s += 0x100;
            for (i=0;i<(32*7);i++) *d++ = *s++;
        }//if

        FCTL2 = FWKEY+FSSEL0+FN2;
        FCTL1 = FWKEY+ERASE;
        FCTL3 = FWKEY;
        *flash = 0;

        FCTL1 = FWKEY+WRT; 

        for (i=0;i<(use_ram*7);i++) *flash++ = *ram++;

        if (use_ram <= 32) {    // flash saved half segment
            ram = (char*) act_reg->ram + (32*7);
            flash = (char *) ((uint16_t) _rom[rom].flash << 8);
            if (_rom[rom].flash & 0x01) flash -= 0x100;
            else flash += 0x100;
            for (i=0;i<(use_ram*7);i++) *flash++ = *ram++;
        }//if

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

uint8_t hex(uint8_t v) {
    return (v+ (v>=10 ? ('A'-10) : '0'));
}

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


void xfull_hint() {
    // ..........
    // 34c SLOW H
    // FAST 2 34c
    if (_opt&OPT_HW_SLOW) {
        strcpy(_msg_buf, "SLOW");
    }//if
    else {
        strcpy(_msg_buf, "FAST");
    }//else
    _msg_buf[4] = '?';
    _msg_buf[5] = _opt&OPT_LONG_TIMEOUT ? 'L' : '2';
    _msg_buf[6] = '?';
    strcpy(_msg_buf+7, _rom[_state&ST_ROM].model);
    /*
    _msg_buf[5] = '?';
    _msg_buf[6] = '0' + (_key/10);
    _msg_buf[7] = '0' + (_key%10);
    _msg_buf[8] = '\0';
    */
}

void show_hint(const char *p) {
    if (*p) {
        strcpy(_msg_buf, "???????");
        _msg_buf[7] = *p++;
        _msg_buf[8] = *p++;
        _msg_buf[9] = *p++;
        _opt |= OPT_ALPHA_MSG;
        _clicks = 20;
    }//if
}

void full_hint() {
    // ..........
    // 00 F 2 34c
    // 00 S L 34c
    show_hint(_rom[_state&ST_ROM].model);
    _msg_buf[0] = _ver[0];
    _msg_buf[1] = _ver[1];
    _msg_buf[3] = _opt&OPT_HW_SLOW ? 'S' : 'F';
    _msg_buf[5] = _opt&OPT_LONG_TIMEOUT ? 'L' : '2';
}

#define TIMEOUT	        (120*60/48)      // 250521 align timeout to 60 secs, now 120 secs
#define TIMEOUT_LONG	(30l*60l*60l/48l)    // 30 minutes

#else

#define ST_ROM				0x0f
static volatile uint8_t _state=0;

void dump_regs() {
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

    // check factory reset signature and store clean memory state for roms
    char *p = factory;
    if (*p == 'F' && *(p+1) == 'A') {
        int i;
        for (i=0;i<NUM_ROMS;i++) {
            if (_rom[i].flash) {
                _state &= ~ST_ROM;
                _state |= i;
                woodstock_set_rom(i);
                woodstock_new_processor();
                woodstock_clear_memory();
                to_flash();
            }//if
            __delay_cycles(MHZ*125);
        }//for
        //___________ erase signature
        //char *flash = (char*) 0x1040;
        char *flash = (char*) factory;
        FCTL1 = FWKEY + ERASE;
        FCTL3 = FWKEY;
        *flash = 0x00;

        FCTL1 = FWKEY+WRT; 
        flash = (char*) greetings;
        strcpy(greetings, "THANK?YOU?");
        FCTL1 = FWKEY;
        FCTL3 = FWKEY+LOCK; 
        _state &= ~ST_ROM;
    }//if

	_opt = OPT_HW_TEST + OPT_ALPHA_MSG;		// enter setup, default to rom 0 (HP34C)
	_key = KEYR_G;
    _state = 8;
	while (_opt&OPT_HW_TEST) {
        full_hint();
        _opt |= OPT_ALPHA_MSG;
        LPM0;
		switch (_key) {
            case KEYR_ON:			// auto-off
                //sleep();
                //WDTCTL = 0;		// s/w reset when wakeup
                _key = KEYR_PGM_RUN;
			case KEYR_PGM_RUN: 	// pgm/run key
				_opt &= ~OPT_HW_TEST; 
				break;
			case KEYR_G:	// G key to rotate rom
                _state++;
				if ((_state&ST_ROM)>=NUM_ROMS) _state &= ~ST_ROM;
				break;
			case KEYR_F:	// F key to toggle fast and slow cpu
                _opt ^= OPT_HW_SLOW;
				break;
			case KEYR_H:	// H key to toggle hint
                _opt ^= OPT_LONG_TIMEOUT;
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
	}//while
    _opt &= ~OPT_ALPHA_MSG;		// clear possible sticky message

	woodstock_set_rom(_state&ST_ROM);
	woodstock_new_processor();
	//from_flash();

	//_state &= ~(ST_RAM_LOADED|ST_KEY_PRESSED|ST_KEY_RELEASED);
	woodstock_set_ext_flag (3, (_pgm_run=0));		// set run mode
    if (_rom[_state&ST_ROM].msg_pos)
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
	woodstock_set_rom(_state&0x07);
	woodstock_new_processor();
	woodstock_set_ext_flag (3, (_pgm_run=0));		// set run mode
    if (_rom[_state&ST_ROM].msg_pos)
        act_reg->f = _rom[_state&ST_ROM].msg_pos;
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
	uint8_t release_in=50;      // clear key press state in n cycles, impacts ROM initialization code

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

	if (0 && !*play) {
		char *dest = (char*) act_reg->ram;
		char *flash = (char*) 0xfc00;
		uint16_t i;
		for (i=0;i<(__USE_RAM*7);i++) *dest++ = *flash++;
		act_reg->f = _rom[_state&0x0f].msg_pos;
		woodstock_set_ext_flag (3, _pgm_run ^= 1);		// pgm-run toggle
	}//if
#endif
	uint8_t done=0, initialized=0, warm_load=0;

	while (!done) {
#ifdef EMBEDDED
		if (_state) {
			if (_state & ST_KEY_PRESSED) {
				//if (_key == (100+KEYR_CLR)) {		// double pressed clear, sleep
				if (_key == KEYR_ON) {					// on key pressed, sleep
                    show_hint("???");
                    _opt |= OPT_ALPHA_MSG;
					while (_key) __asm("  nop");
                    show_hint(_rom[_state&ST_ROM].model);
					_state &= ~ST_KEY_PRESSED;
					sleep();
                    //uint8_t i;
                    //for (i=0;i<10;i++) _msg_buf[i] = '?';
                    //_opt |= OPT_ALPHA_MSG;
					//_clicks = 5;
					while (_clicks) {
						if (_key == KEYR_ZERO) {   // '0'
                            while (_key) __asm("  nop");
                            WDTCTL = 0;		// s/w reset when '0' pressed to wakeup
                        }//if
#ifdef MEMORY_DUMP
						if (_key == 11) {	// '.' debug, show ram values
							_clicks = 0;
                            char *p = (char *) 0xfce0;
							for (;;) {
								_state &= ~ST_KEY_PRESSED;
                                _msg_buf[0] = hex((((uint16_t) p)&0xff)>>4);
                                _msg_buf[1] = hex(((uint16_t) p)&0x0f);
                                _msg_buf[2] = hex(*p>>4);
                                _msg_buf[3] = hex(*p&0x0f);
                                p++;
                                _msg_buf[4] = hex(*p>>4);
                                _msg_buf[5] = hex(*p&0x0f);
                                p++;
                                _msg_buf[6] = hex(*p>>4);
                                _msg_buf[7] = hex(*p&0x0f);
                                p++;
                                _msg_buf[8] = hex(*p>>4);
                                _msg_buf[9] = hex(*p&0x0f);
                                p++;
                                _opt |= OPT_ALPHA_MSG;
								// wait for a key
								while (!(_state & ST_KEY_PRESSED)) __asm(" nop");
								switch (_key) {
									case 11: p -= 8; break;	// .
									case 42: break;		    // R/S
									default: break;
								}//switch
                                if (_key == KEYR_PGM_RUN) break;
							}//for
							while (_key) __asm(" nop");
                            _opt &= ~OPT_ALPHA_MSG;
                        }//if
#endif
#ifdef REGISTER_DUMP
						if (_key == 11) {	// '.' debug, show ram values
							uint8_t i=0, j, s=0;
                            // 00-10-89-67-45-23-01
                            //  bcd digits        a 89    674523 1   1-10(a) digits
                            // 22.22222222 (....012022)(..22222202)  01.0 means exp 1
                            // .2222222222 (....990922)(..22222202)  99.9 means exp -1
                            //                  AA B            C is sign
                            // AA exponent, B sign for exponent, exponent is 100-exp when negative
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
                                _opt |= OPT_ALPHA_MSG;
								// wait for a key
								while (!(_state & ST_KEY_PRESSED)) __asm(" nop");
								switch (_key) {
									case 11: s ^= 1; break;		// .
									case 41: if (i) i--; break;	// 3
									case 42: i++; break;		// R/S
									default: break;
								}//switch
								//if (_key == 10) break;			// 0
                                if (_key == KEYR_PGM_RUN) break;
							}//for
							while (_key) __asm(" nop");
                            _opt &= ~OPT_ALPHA_MSG;
						}//if
#endif
/*
                 22,
     25,26,27,14,23,
	 29,30,31,15,13,
	  33,  34,35,17,
	    37,38,39,18,
	     1, 2, 3,19,
	     5, 6, 7,41,
	     9,10,11,42,
*/
						if (_key == KEYR_DOT) {	// '.' show greetings
                            const char allow[] = "0?12ABC3DEF4GHI5JKL6MNO7PQRS8TUV9WXYZ:";
							_clicks = (uint8_t) noerase;
                            _state &= ~ST_KEY_PRESSED;
                            _opt |= OPT_HW_TEST;
                            char *p = greetings;
                            uint8_t i=0;
							for (i=0;i<10;i++) _msg_buf[i] = *p++;
                            _blink = 0;
                            while (1) {
                                while (_key) __asm("  nop");
                                _opt |= OPT_ALPHA_MSG;
                                while (!_key) __asm("  nop");
                                if (_key == KEYR_RS) {   // R/S
                                    _blink++;
                                }//if
                                else {
                                    if (_blink) {
                                        switch (_key) {
                                            case 10: i='0'; break;
                                            case  6: i='1'; break;
                                            case  7: i='2'; break;
                                            case 41: i='3'; break;
                                            case  2: i='4'; break;
                                            case  3: i='5'; break;
                                            case 19: i='6'; break;
                                            case 38: i='7'; break;
                                            case 39: i='8'; break;
                                            case 18: i='9'; break;
                                            default: _blink = 11; break;
                                        }//switch
                                        if (_blink < 11) {
                                            const char *s0 = allow, *s1, *ss = allow;
                                            while (*s0 != i) s0++;
                                            s1 = s0;
                                            i++;
                                            while (*s1 != i) s1++;
                                            while (*ss != _msg_buf[_blink-1]) ss++;
                                            ss++;
                                            if (ss < s0 || ss >= s1)
                                                _msg_buf[_blink-1] = *s0;
                                            else
                                                _msg_buf[_blink-1] = *ss;
                                        }//if
                                    }//if
                                    else break;
                                }//else
                                if (_blink >= 11) break;
                            }//while
                            if (_opt |= OPT_WRITE_GREETINGS) {
                                FCTL2 = FWKEY+FSSEL0+FN2;
                                FCTL1 = FWKEY+ERASE;
                                FCTL3 = FWKEY;
                                *greetings = 0;
                                FCTL1 = FWKEY+WRT; 
                                for (i=0;i<11;i++) greetings[i] = _msg_buf[i];
                                FCTL1 = FWKEY;
                                FCTL3 = FWKEY+LOCK; 
                            }//if
                            _opt &= ~(OPT_ALPHA_MSG|OPT_HW_TEST|OPT_WRITE_GREETINGS);
                            _blink = 0;
                            _key = KEYR_CLR;
                            break;
                        }//if

						if (_key == KEYR_RS) {	// 'R/S' show password
                            uint16_t *pass = (uint16_t*) 0xffe0;
                            uint8_t idx = 0;
                            _clicks = 0;
                            while (1) {
                                while (_key) __asm("  nop");
                                _opt |= OPT_ALPHA_MSG;
                                if (*(pass+idx) != 0xffff) {
                                    uint16_t val = *(pass+idx);
                                    _msg_buf[4] = '?';
                                    _msg_buf[3] = hex((idx&0x07)<<1);
                                    _msg_buf[2] = 'E' + (idx>>3);
                                    _msg_buf[1] = 'F';
                                    _msg_buf[0] = 'F';

                                    _msg_buf[9] = '?';
                                    _msg_buf[6] = hex(val&0x0f); val >>= 4;
                                    _msg_buf[5] = hex(val&0x0f); val >>= 4;
                                    _msg_buf[8] = hex(val&0x0f); val >>= 4;
                                    _msg_buf[7] = hex(val&0x0f);
                                    //LPM0;
                                    while (!_key) __asm("  nop");
                                    if (_key != KEYR_RS) break;
                                }//if
                                idx++;
                                idx &= 0x0f;
                            }//while
                            _key = KEYR_CLR;
                            break;
                        }//if
#ifdef KEYTEST
						if (_key == 42) {	// 'R/S'
							_clicks = 0;
                            while (1) {
								_state &= ~ST_KEY_PRESSED;
                                _msg_buf[0] = '0' + (_key/10);
                                _msg_buf[1] = '0' + (_key%10);
                                _msg_buf[2] = '_';
                                uint8_t k34 = key_map_3x[_key];
                                _msg_buf[3] = '0' + (k34/16);
                                _msg_buf[4] = '0' + (k34%16);
                                _msg_buf[5] = '?';
                                _opt |= OPT_ALPHA_MSG;
								while (!(_state & ST_KEY_PRESSED)) __asm(" nop");
                                if (_key == KEYR_PGM_RUN) break;
                            }//while
							while (_key) __asm(" nop");
                            _opt &= ~OPT_ALPHA_MSG;
                            continue;
                        }//if
#endif
					}//while
                    while (_clicks) __asm(" nop");
					continue;
				}//if
				if (_key == (100+KEYR_PGM_RUN)) {		// double press pgm-run, save registers / program to flash
					to_flash();
					//_key -= 100;
                    //_state &= ~ST_KEY_PRESSED;
                    strcpy(_msg_buf, "?MEM?SAVED");
                    _opt |= OPT_ALPHA_MSG;
                    _clicks = 40;
                    //continue;
				}//if
				if (_key == KEYR_PGM_RUN) {
                    if (*_rom[_state&ST_ROM].slide)
                        woodstock_set_ext_flag (3, _pgm_run ^= 1);		// pgm-run toggle
                    if (initialized) show_hint(_rom[_state&ST_ROM].slide + _pgm_run*3);
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
                if (initialized == 1) warm_load = 100;
			}//if
		}//if
        if (!initialized) initialized++;
		if (!woodstock_execute_instruction()) break;
        if (warm_load) {
            if (warm_load == 1) {
                from_flash();
                initialized++;
            }//if
            warm_load--;
        }//if
#ifdef DEBUG_STEP
        _msg_buf[0] = hex((act_reg->s>>12)&0x0f);
        _msg_buf[1] = hex((act_reg->s>>8)&0x0f);
        _msg_buf[2] = hex((act_reg->s>>4)&0x0f);
        _msg_buf[3] = hex(act_reg->s&0x0f);
        _msg_buf[4] = hex((act_reg->flags>>4)&0x0f);
        _msg_buf[5] = hex(act_reg->flags&0x0f);
        _msg_buf[6] = '?';
        _msg_buf[7] = hex((act_reg->pc>>8)&0x0f);
        _msg_buf[8] = hex((act_reg->pc>>4)&0x0f);
        _msg_buf[9] = hex(act_reg->pc&0x0f);
        _opt |= OPT_ALPHA_MSG;
        while (!(_state & ST_KEY_PRESSED)) __asm("  nop");
        woodstock_release_key();
        _opt &= ~OPT_ALPHA_MSG;
#endif
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
                    dump_regs();
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
						vlog("+[%c] %2d %02x", c, key_map[c], key_map_3x[key_map[c]]);
						sim_check_key(key_map[c]);
						release_in = 10;
					}//if
					break;
			}//switch
			c = 0;
		}//if
		if (!woodstock_execute_instruction()) break;
        if (0&&!(act_reg->pc >= 0x1e2 && act_reg->pc <= 0x1f0)) {
            vlog("+%03lx,", act_reg->pc);
            dump_regs();
        }//if
		if (release_in) {
			if (release_in == 1) {
                woodstock_release_key();
                vlog("+[RELEASE_KEY]");
            }//if
			release_in--;
		}//if
#endif

#ifdef EMBEDDED
		if (_opt&OPT_HW_SLOW) __delay_cycles(MHZ*125);
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
volatile uint8_t _decimal_stay=0;

//________________________________________________________________________________
void __interrupt_vec(TIMER0_A0_VECTOR) Timer_A (void) {

	P1OUT |= ALL_DIGITS_P1;	 // 74hc164 friedly, all digit off
	P2OUT |= ALL_DIGITS_P2;
	P3OUT = _data;

	//static uint8_t key_since=0;
	static uint16_t key_since=0;
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
						_opt &= ~OPT_ALPHA_MSG;		// clear possible sticky message
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
		//if (key_since > TIMEOUT) {
		if (key_since > (_opt&OPT_LONG_TIMEOUT ? TIMEOUT_LONG : TIMEOUT)) {
			//_key = 100+KEYR_CLR;			// auto-off
			// check if cpu not active, waiting for key press and we can sleep (we only check 34C
            // fixme
			if (_opt & OPT_HW_TEST || (_state & ST_ROM) != 1 || (act_reg->pc >= 0x430 && act_reg->pc <= 0x43c)) {
				_key = KEYR_ON;			// auto-off
				_state |= ST_KEY_PRESSED;
                if (_opt & OPT_HW_TEST)
                    LPM0_EXIT;
                else
                    key_since = 0;
			}//if
		}//if
	}//if
	else {
		if (act_reg->flags&F_DISPLAY_ON || _opt&(OPT_ALPHA_MSG|OPT_HW_TEST)) {
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
        // average lighted segment of 6, stay on 8 * 6 * 50 = 2400 cycles
        // with MCU running at 8Mhz
        // for 5 digits on refresh rate is 8000000 / (5 * 2400) = 8000/12 = 600Hz
        // for 10 digits it's about 300Hz, it is actually lower as we have key scanning and extra decimal brightening cycles
		TA0CCR0 += MHZ * segs * 50;     // to increase refresh rate, reduce value from 50 to lower
	}//else

    if (_decimal_stay) {
        _decimal_stay--;
        _data = SEG_H;
        return;
    }//if
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

	if (_digit == 12) {
		_data = 0;		// key scan cycle, blank all segments
	}//if
	else {
		//if (_opt&OPT_ALPHA_MSG) {
		if (_opt&(OPT_ALPHA_MSG|OPT_HW_TEST)) {
            if (_blink && _digit == (_blink-1) && (_ticks>>9)&0x01)
                _data = 0;
            else
                _data = char_at_digit(_digit);
		}//if
		else {
            _data = sim_load_segments(_digit);
            if (_data&SEG_H) _decimal_stay += 1;
		}//else
	}//else
		
	_ticks++;
	if (!(_ticks&0x07ff)) key_since++;      // 0.8sec
	if (!(_ticks&0x003f) && _clicks) {      // 0.025sec
		_clicks--;
		//if (!_clicks) LPM0_EXIT;
		if (!_clicks) {
			_opt &= ~OPT_ALPHA_MSG;
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

