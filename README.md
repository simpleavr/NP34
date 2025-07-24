
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

