

NP Calculator Emulator
======================
Nonpariel Physical (NP) is an standalone calculator microcode emulator

Features
_________________
* selectable rom from HP woodstock and spice series units. 21, 22, 25, 27, 29C, 31E, 32E, 33C, 34C, 37E, 38C
* Setup is done by turning on the unit with [0] pressed, and use [f][g][h] to toggle options
* [+][-] to adjust brightness, [PGM/RUN] exit setup
* setup allows choice of ROM to use [g], set speed (fast / slow) [f] and sleep timeout of either 2 or 30 minutes [h]
* secret message / greetings by turining on the unit with [.] pressed, [R/S] to start edit message, other keys to dismiss
* register contents / program steps can be store in flash (press [PGM/RUN] key twice in quick succession) in their own spaces
* each model has its own flash space for register / program retention (via double keying [PGRM/RUN])
* all models share the same RAM space, switching between models are hard resets with RAM clearing

Description
___________

Compile w/ (example only, substitute w/ your setup path)

/msp430-elf-gcc -D EMBEDDED -I /usr/local/ti/msp430-gcc/include -mmcu=msp430g2955 -Os -g -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,--relax -Wl,--gc-sections,--section-start=.rodata_factory=0x01040,--section-start=.rodata_greetings=0x1080 -L /usr/local/ti/msp430-gcc/include -T msp430g2955.ld  np34.o -o np34.out

june 2025, cc
* initial public release

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


Parts list
__________

* msp430g2955
* 2x CL25011AH LED Module
* 32× SMT Tactile Button 6x3mm
* 1x CR2032 SMD/TH battery holder
* 1x 47k resistor
* 1x 100nF (104) capacitor (optional)

_________________

* [0]+[ON] enter setup
* [G] cycles 21/22/25/27/29C/31E/32E/33C/34C/37E/38C emulation
* [F] toggles slow/fast cpu
* [H] toggles 2 minutes or 30 minutes sleep time
* [+][-] adjust brightness
* [PGM/RUN] exit setup

_________________

* [.]+[ON] enter secret message / greeting
* [R/S] starts editing and advances digit position
* [0]..[9] to enter digits and letters like telephone keypad
* any other key dismiss greeting mode

Schematic
_________


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


Additional Notes
________________





