/*
The MIT License (MIT)

Copyright (c) 2016 silverx

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <inttypes.h>
#include <math.h>

#include "pid.h"
#include "config.h"
#include "util.h"
#include "drv_pwm.h"
#include "control.h"
#include "defines.h"
#include "drv_time.h"
#include "sixaxis.h"
#include "drv_fmc.h"
#include "flip_sequencer.h"
#include "gestures.h"
#include "defines.h"


extern float rx[7];
extern float gyro[3];
extern int failsafe;
extern float pidoutput[PIDNUMBER];

extern float angleerror[3];
extern float attitude[3];

int onground = 1;
float thrsum;

float error[PIDNUMBER];
float motormap( float input);

float yawangle;

extern float looptime;

extern char auxchange[AUXNUMBER];
extern char aux[AUXNUMBER];

extern int ledcommand;

extern float apid(int x);

#ifdef NOMOTORS
// to maintain timing or it will be optimized away
float tempx[4];
#endif

unsigned long timecommand = 0;

extern int controls_override;
extern float rx_override[];
extern int acro_override;


void control( void)
{	

	// make local copy
	float rxcopy[4];	
	for ( int i = 0 ; i < 4 ; i++)
	{
		rxcopy[i] = rx[i];
	}


#ifndef DISABLE_FLIP_SEQUENCER	
  flip_sequencer();
	
	if ( controls_override)
	{
		for ( int i = 0 ; i < 3 ; i++)
		{
			rxcopy[i] = rx_override[i];
		}
	}

	if ( auxchange[STARTFLIP]&&!aux[STARTFLIP] )
	{// only on high -> low transition
		start_flip();		
	}
#endif	
	
	// check for accelerometer calibration command
	if ( onground )
	{
		#ifdef GESTURES1_ENABLE
		if ( rx[1] < -0.8  )
		{
			if ( !timecommand) timecommand = gettime();
			if ( gettime() - timecommand > 3e6 )
			{
				// do command
					
			    gyro_cal();	// for flashing lights		
					#ifndef ACRO_ONLY				
			    acc_cal();
				  extern float accelcal[3];			  
				  fmc_write( accelcal[0] + 127 , accelcal[1] + 127);
				  #endif
			    // reset loop time so max loop time is not exceeding
			    extern unsigned lastlooptime;
			    lastlooptime = gettime();
		      timecommand = 0;
			}		
		}
		else timecommand = 0;	
		#endif		
		#ifdef GESTURES2_ENABLE
		int command = gestures2();

		if (command)
	  {
		  if (command == 3)
		    {
			    gyro_cal();	// for flashing lights
			    #ifndef ACRO_ONLY
			    acc_cal();
				  extern float accelcal[3];
				  
				  fmc_write( accelcal[0] + 127 , accelcal[1] + 127);
				  #endif
			    // reset loop time 
			    extern unsigned lastlooptime;
			    lastlooptime = gettime();
		    }
		  else
		    {
			    ledcommand = 1;
			    if (command == 2)
			      {
				      aux[CH_AUX1] = 1;

			      }
			    if (command == 1)
			      {
				      aux[CH_AUX1] = 0;
			      }
		    }
	  }
		#endif		
	}
#ifndef DISABLE_HEADLESS 
// yaw angle for headless mode	
	yawangle = yawangle + gyro[YAW]*looptime;
	if ( auxchange[HEADLESSMODE] )
	{
		yawangle = 0;
	}
	
	if ( aux[HEADLESSMODE] ) 
	{
		while (yawangle < -3.14159265f)
    yawangle += 6.28318531f;

    while (yawangle >  3.14159265f)
    yawangle -= 6.28318531f;
		
		float temp = rxcopy[ROLL];
		rxcopy[ROLL] = rxcopy[ROLL] * fastcos( yawangle) - rxcopy[PITCH] * fastsin(yawangle );
		rxcopy[PITCH] = rxcopy[PITCH] * fastcos( yawangle) + temp * fastsin(yawangle ) ;
	}
#endif	
pid_precalc();	

#ifndef ACRO_ONLY
	// dual mode build
	if (aux[LEVELMODE]&&!acro_override)
	  {			// level mode

		  angleerror[0] = rxcopy[0] * MAX_ANGLE_HI - attitude[0] + (float) TRIM_ROLL;
		  angleerror[1] = rxcopy[1] * MAX_ANGLE_HI - attitude[1] + (float) TRIM_PITCH;

		  error[0] = apid(0)  - gyro[0];
		  error[1] = apid(1)  - gyro[1];

	  }
	else
	  {	// rate mode

		  error[0] = rxcopy[0] * (float) MAX_RATE * DEGTORAD  - gyro[0];
		  error[1] = rxcopy[1] * (float) MAX_RATE * DEGTORAD  - gyro[1];

		  // reduce angle Iterm towards zero
		  extern float aierror[3];
		  for (int i = 0; i <= 2; i++)
			  aierror[i] *= 0.8f;
	  }


	error[2] = rxcopy[2] * (float) MAX_RATEYAW * DEGTORAD  - gyro[2];

	pid(0);
	pid(1);
	pid(2);
#else
// rate only build
	error[ROLL] = rxcopy[ROLL] * (float) MAX_RATE * DEGTORAD  - gyro[ROLL];
	error[PITCH] = rxcopy[PITCH] * (float) MAX_RATE * DEGTORAD  - gyro[PITCH];
	error[YAW] = rxcopy[YAW] * (float) MAX_RATEYAW * DEGTORAD  - gyro[YAW];
	
pid_precalc();

	pid(ROLL);
	pid(PITCH);
	pid(YAW);
#endif

float	throttle;

// map throttle so under 10% it is zero	
if ( rx[3] < 0.1f ) throttle = 0;
else throttle = (rx[3] - 0.1f)*1.11111111f;


// turn motors off if throttle is off and pitch / roll sticks are centered
	if ( failsafe || (throttle < 0.001f && (!ENABLESTIX||  (fabsf(rx[ROLL]) < 0.5f && fabsf(rx[PITCH]) < 0.5f ) ) ) ) 

	{ // motors off
		for ( int i = 0 ; i <= 3 ; i++)
		{
			pwm_set( i , 0 );	
			#ifdef MOTOR_FILTER	
			// reset the motor filter
			motorfilter( 0 , i);
			#endif
		}	
		
		#ifdef USE_PWM_DRIVER
		#ifdef MOTOR_BEEPS
		extern void motorbeep( void);
		motorbeep();
		#endif
		#endif
		
		onground = 1;
		thrsum = 0;
		
	}
	else
	{
		onground = 0;
		float mix[4];	
		
	if ( controls_override)
	{// change throttle in flip mode
		throttle = rx_override[3];
	}
		
		
		  // throttle angle compensation
#ifdef AUTO_THROTTLE
		  if (aux[LEVELMODE])
		    {
			    float autothrottle = fastcos(attitude[0] * DEGTORAD) * fastcos(attitude[1] * DEGTORAD);
			    float old_throttle = throttle;
			    if (autothrottle <= 0.5f)
				    autothrottle = 0.5f;
			    throttle = throttle / autothrottle;
			    // limit to 90%
			    if (old_throttle < 0.9f)
				    if (throttle > 0.9f)
					    throttle = 0.9f;

			    if (throttle > 1.0f)
				    throttle = 1.0f;

		    }
#endif
	
#ifdef LVC_PREVENT_RESET
extern float vbatt;
if (vbatt < (float) LVC_PREVENT_RESET_VOLTAGE) 
{
	throttle = 0;
}
#endif

				
#ifdef INVERT_YAW_PID
pidoutput[2] = -pidoutput[2];			
#endif
		
		mix[MOTOR_FR] = throttle - pidoutput[ROLL] - pidoutput[PITCH] + pidoutput[YAW];		// FR
		mix[MOTOR_FL] = throttle + pidoutput[ROLL] - pidoutput[PITCH] - pidoutput[YAW];		// FL	
		mix[MOTOR_BR] = throttle - pidoutput[ROLL] + pidoutput[PITCH] - pidoutput[YAW];		// BR
		mix[MOTOR_BL] = throttle + pidoutput[ROLL] + pidoutput[PITCH] + pidoutput[YAW];		// BL	
		
#ifdef INVERT_YAW_PID
// we invert again cause it's used by the pid internally (for limit)
pidoutput[2] = -pidoutput[2];			
#endif
	
				

thrsum = 0;		
				
		for ( int i = 0 ; i <= 3 ; i++)
		{			
		#ifdef MOTOR_FILTER		
		mix[i] = motorfilter(  mix[i] , i);
		#endif		
		#ifdef CLIP_FF
		mix[i] = clip_ff(mix[i], i);
		#endif	
		#ifndef NOMOTORS
		#ifndef MOTORS_TO_THROTTLE
		//normal mode
		pwm_set( i ,motormap( mix[i] ) );
		#else
		// test mode
		ledcommand = 1;
		pwm_set( i , throttle );
		#endif
		#else
		// no motors mode ( anti-optimization)
		tempx[i] = motormap( mix[i] );
		#endif
		if ( mix[i] < 0 ) mix[i] = 0;
		if ( mix[i] > 1 ) mix[i] = 1;
		thrsum+= mix[i];
		}	
		thrsum = thrsum / 4;
		
	}// end motors on
	
}



float hann_lastsample[4];
float hann_lastsample2[4];

// hanning 3 sample filter
float motorfilter( float motorin ,int number)
{
 	float ans = motorin*0.25f + hann_lastsample[number] * 0.5f +   hann_lastsample2[number] * 0.25f ;
	
	hann_lastsample2[number] = hann_lastsample[number];
	hann_lastsample[number] = motorin;
	
	return ans;
}


float clip_feedforward[4];
// clip feedforward adds the amount of thrust exceeding 1.0 ( max) 
// to the next iteration(s) of the loop
// so samples 0.5 , 1.5 , 0.4 would transform into 0.5 , 1.0 , 0.9;

float clip_ff(float motorin, int number)
{

	if (motorin > 1.0f)
	  {
		  clip_feedforward[number] += (motorin - 1.0f);
		  //cap feedforward to prevent windup 
		  if (clip_feedforward[number] > .5f)
			  clip_feedforward[number] = .5f;
	  }
	else if (clip_feedforward[number] > 0)
	  {
		  float difference = 1.0f - motorin;
		  motorin = motorin + clip_feedforward[number];
		  if (motorin > 1.0f)
		    {
			    clip_feedforward[number] -= difference;
			    if (clip_feedforward[number] < 0)
				    clip_feedforward[number] = 0;
		    }
		  else
			  clip_feedforward[number] = 0;

	  }
	return motorin;
}


