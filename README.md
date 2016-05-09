# CG023
##**Firmware for CG023**

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



