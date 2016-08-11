###H8mini Blue board version###

From rcg user __kmtam__
> Hi all, I finally got the h8mini blue board erased and flashed to Silverxxx firmware after zillion times of tried and error.
> The board's labelled wrong with the CLK and DAT got swapped....
> 
> This is how I got it work from the STLINK to the H8mini blue board:
> SWDIO->CLK
> GND->GND
> SWCLK->DAT
> 
> Thanks Silverxxx for this wonderful firmware! Cheers! 

From rcg user __ketsa__
> Thanks!!! I finally managed to flash my blue board.
> 
> It was not straightforward, I tried the suggested connections and was still getting "ST-LINK USB communication error."
> 
> I then found a file in my keil directory named "ST-LinkUpgrade.exe" So I Upgraded the FW of my ST-link to "V2.J24.S4 > > > STM32+STM8 Debugger"
> 
> Then i was getting a different message : "no target" hmm. progress. but still not flashing.
> Then I removed the battery, plugged it in and it erased !
> 
> I tried to flash : "no target."
> 
> Replug battery, and it flashed !

From rcg user __silverxxxa__
> I made a blue board quad to see what's happening with the flashes.
> 
> So far I found out that it does indeed stop working after the st-link gets disconnected from usb.
> This seems some software issue, maybe with keil.
> 
> The st-link starts working after it is used in some other way , such as: flash another quad with keil(not blue board) , use > the "st-link utility" and connect to a board(even blue), use openocd and connect to a board (even blue).
> 
> After the "st-link usb communication error" comes up , you have to disconnect the st-link and reconnect it, or the above > fixes do not work.
> 
> Another find was that the nucleo built in st-link does not have this issue. It also does not get detected by the st-link firmware updater. A nucleo board is about $12
> 
> I'll have to find a more permanent solution, although openocd could be made a shortcut. 
The H8 uses a stm32f030 and a XN297L 3 wire radio

_The last 2 posts refer to windows and keil only_

This port supports both acro only and dual mode. To enable acro only, an option exist in config.h.

The H8 board - blue version - uses a stm32f030 and a XN297L 3 wire radio. The XN297L is differerent from the XN297 in that is supports 250K rate, and the dabug registers are different.  [XN297L datasheet](https://drive.google.com/file/d/0B3AKcbg1PFrnbHRXMzUzUUFmUFk/view?pref=2&pli=1) (use google translate)

### Flashing instructions
http://www.rcgroups.com/forums/showthread.php?t=2634611

###Wiki
http://sirdomsen.diskstation.me/dokuwiki/doku.php?id=start

###Updates:
###11.08.16
* added tx autocenter for stock tx
* added extra devo channels that were MIA

###03.07.16
* added updates from cg023 build
* now using hardware i2c for full speed (1 Mhz)

###30.05.16
* fixed low rates control issue in yaw+roll

###19.05
* added gestures
* changed imu from the experimental one that somehow made it in, to the old one

###11.05
* added linux compilation support

###10.05
* minor update
* additional protocols now work

###Original text from cg023 follows:###

This is an alternate firmware for the Eachine CG023 quadcopter. It is a port of the Eachine H8 firmware, also by me ( silverxxx).

The quadcopter is based around the STM32F031 chip, Cortex M0 processor with 16K flash memory.

###Compiling:
Compile using MDK-ARM toolchain aka Keil uVision. A special version is available for stm32F0xx devices ( full free version ), but it's not necessary since the 32K limit of the free version is above the cpu's 16K. STM32 support may need to be installed using the "pack installer" 

###Radio protocol:
Current options are stock cg023 transmitter ,H8 mini transmitter / devo, CX-10 (later "blue" variant) and H7. Protocol is by default stock CG023 protocol, and can be changed in file config.h

###Accelerometer calibration:
Hold pitch stick down for 3 seconds, with throttle off. Needs to be done on a level surface. Saved so it only needs to be done once. You may need to use high rates in order to reach the treshold.

###Differences from H8 version:
 * rates have been integrated into protocol file, and depend on protocol abilities
 * the quadcopter rate ( in deg/sec) is no longer multiplied by 2, so it's the actual rate with devo.
 * acro only version can be compiled by enabling respective setting in config.h
 * LEVEL_MAX_RATE is now a hardware limit and does not affect pid kp and ki

###Hardware settings
The STM32 port ( this firmware ) is designed to be configurable in order to be adaptable to other boards.Hardware configuration  settings are found in file hardware.h . Right now, only xn297 radio and invensense gyro (sometimes marked M5xx or M6xx) are supported. Pwm pins can be set to PA0, PA1, PA2, PA3 , PA5 , PA8 , PA9 , PA10 , PA11. Radio SPI and gyro I2C can be set to any PAx or PBx pins.

###Linux/gcc version
The gcc version compiles close to 16k, and may need turning off features in order to make it fit.

###Installation and Support
Currently this port is covered by the it's own thread on rcgroups.
http://www.rcgroups.com/forums/showthread.php?t=2634611#post34381034


###History:

####29.04.16
* added motor curves
* exp optimization merged from user stawel

####21.04.16
* added CX-10 (blue) protocol

####19.04.16
* adc input fix
* added invsqrt to imu (from quake) 

####19.04.16
* merged gcc support by Paweł Stawicki
* merged H7 protocol support by Paweł Stawicki

####13.04.16
* fixed bug setting motor to PA11

####03.04.16
* added automatic flips
* flash saving optimization 

####01.04.16
* flash saving optimizations
* headless fixed in both modes
* pwm frequency can be set now

####29.03.16
* moved pin setting to hardware.h
* level pid limit change ( LEVEL_MAX_RATE now acts as hardware limit)

####26.03.16
* added pwm defines
* i2c speed improvement

####23.03.16
* some optimizations, etc

####20.03.16:
* dual mode added
* added alternate led "battery low 2" ( 3.3V )

####20.03.16:
* CG023 stock tx protocol added



