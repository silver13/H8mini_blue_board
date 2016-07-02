
#include "defines.h"
#include "hardware.h"

// pids in pid.c

// rate in deg/sec
// for acro mode
#define MAX_RATE 360.0
#define MAX_RATEYAW 360.0

// max angle for level mode
#define MAX_ANGLE_HI 55.0f

// max rate used by level pid ( limit )
#define LEVEL_MAX_RATE 2000

// disable inbuilt expo functions
#define DISABLE_EXPO

// use if your tx has no expo function
// also comment out DISABLE_EXPO to use
// -1 to 1 , 0 = no exp
// positive = less sensitive near center 
#define EXPO_XY 0.3
#define EXPO_YAW 0.0



// failsafe time in uS
#define FAILSAFETIME 1000000  // one second


// battery saver
// do not start software if battery is too low
// flashes 2 times repeatedly at startup
#define STOP_LOWBATTERY

// under this voltage the software will not start 
// if STOP_LOWBATTERY is defined above
#define STOP_LOWBATTERY_TRESH 3.3

// voltage to start warning
// volts
#define VBATTLOW 3.5

// compensation for battery voltage vs throttle drop
// increase if battery low comes on at max throttle
// decrease if battery low warning goes away at high throttle
// in volts
#define VDROP_FACTOR 0.7

// voltage hysteresys
// in volts
#define HYST 0.10




// Gyro LPF filter frequency
// gyro filter 0 = 250hz delay 0.97mS
// gyro filter 1 = 184hz delay 2.9mS
// gyro filter 2 = 92hz delay 3.9mS
// gyro filter 3 = 41hz delay 5.9mS (Default)
// gyro filter 4 = 20hz
// gyro filter 5 = 10hz
// gyro filter 6 = 5hz
// gyro filter 7 = 3600hz delay 0.17mS
#define GYRO_LOW_PASS_FILTER 3


// software gyro lpf ( iir )
// set only one below
//#define SOFT_LPF_1ST_023HZ
//#define SOFT_LPF_1ST_043HZ
//#define SOFT_LPF_1ST_100HZ
//#define SOFT_LPF_2ND_043HZ
//#define SOFT_LPF_2ND_088HZ
//#define SOFT_LPF_4TH_088HZ
//#define SOFT_LPF_4TH_160HZ
//#define SOFT_LPF_4TH_250HZ
#define SOFT_LPF_NONE


// this works only on newer boards (non mpu-6050)
// on older boards the hw gyro setting controls the acc as well
#define ACC_LOW_PASS_FILTER 5


// channel assignments (switches)

// H8 protocol channels
// CH_FLIP - flip,  CH_HEADFREE - headfree, CH_RTH - headingreturn

// cg023 protocol chanels
// CH_CG023_FLIP , CH_CG023_VIDEO , CH_CG023_STILL , CH_CG023_LED

// H7 channels
// CH_H7_FLIP , CH_H7_VIDEO , CH_H7_FS

// CX10
// CH_CX10_CH0  (unknown) , CH_CX10_CH2 ( rates mid)

// CH_ON - on always ( all protocols)
// CH_OFF - off always ( all protocols)
// CH_AUX1 - gestures
// CH_ON , CH_OFF , CH_FLIP , CH_EXPERT
// CH_HEADFREE , CH_RTH , CH_AUX1 , CH_AUX2 , CH_AUX3 , CH_AUX4
// CH_PIT_TRIM, CH_RLL_TRIM, CH_THR_TRIM, CH_YAW_TRIM
#define HEADLESSMODE CH_OFF

#define LEVELMODE CH_AUX1

#define STARTFLIP CH_OFF

#define LEDS_ON CH_ON

// Gestures enable ( gestures 1 = acc only)
//#define GESTURES1_ENABLE
#define GESTURES2_ENABLE

// aux1 channel starts on if this is defined, otherwise off.
#define AUX1_START_ON

// enable motor filter
// hanning 3 sample fir filter
#define MOTOR_FILTER

// clip feedforward attempts to resolve issues that occur near full throttle
//#define CLIP_FF

// pwm frequency for motor control
// a higher frequency makes the motors more linear
// in Hz
#define PWMFREQ 8000

// motor curve to use
// the pwm frequency has to be set independently
// 720motors - use 8khz and curve none.
#define MOTOR_CURVE_NONE
//#define MOTOR_CURVE_6MM_490HZ
//#define MOTOR_CURVE_85MM_8KHZ
//#define MOTOR_CURVE_85MM_32KHZ


// throttle angle compensation in level mode
// comment out to disable
#define AUTO_THROTTLE

// build acro only firmware
// removes the distraction of level mode for learning
//#define ACRO_ONLY

// Radio protocol selection
// select only one 
//
//#define RX_CG023_PROTOCOL
//#define RX_H7_PROTOCOL
#define RX_BAYANG_PROTOCOL
//#define RX_CX10BLUE_PROTOCOL

// mode 1 to mode 3 conversion
// cg023 protocol
//#define RX_CG023_SWAP_YAWROLL

// Flash saving features
#define DISABLE_HEADLESS
#define DISABLE_FLIP_SEQUENCER

// turn leds off during flight (to save power)
// battery low will still work
//#define LEDS_OFF













//##################################
// debug / other things
// this should not be usually changed



// enable serial driver ( pin SWCLK after calibration) 
// WILL DISABLE PROGRAMMING AFTER GYRO CALIBRATION - 2 - 3 seconds after powerup)
//#define SERIAL_ENABLE
// enable some serial info output
//#define SERIAL_INFO
// enable osd out
//#define OSD_LTM_PROTOCOL

// level mode "manual" trims ( in degrees)
// pitch positive forward
// roll positive right
#define TRIM_PITCH 0.0
#define TRIM_ROLL 0.0

// disable motors for testing
// #define NOMOTORS

// throttle direct to motors for thrust measure
//#define MOTORS_TO_THROTTLE

// loop time in uS
// this affects soft gyro lpf frequency if used
#define LOOPTIME 1000

// not available
// enable serial out
// 57600 default
// #define SERIAL

// invert yaw pid for hubsan motors
//#define INVERT_YAW_PID

// debug things ( debug struct and other)
//#define DEBUG

// rxdebug structure
//#define RXDEBUG

// enable motors if pitch / roll controls off center (at zero throttle)
// possible values: 0 / 1
#define ENABLESTIX 0

// overclock to 64Mhz
//#define ENABLE_OVERCLOCK


#pragma diag_warning 1035 , 177 , 4017
#pragma diag_error 260

//--fpmode=fast





// define logic
///////////////


#ifdef ENABLE_OVERCLOCK
#define SYS_CLOCK_FREQ_HZ 64000000
#else
#define SYS_CLOCK_FREQ_HZ 48000000
#endif

#ifdef ACRO_ONLY
	#define DISABLE_FLIP_SEQUENCER
	#undef AUTO_THROTTLE
#endif


#ifdef OSD_LTM_PROTOCOL
#define RXDEBUG
#endif















