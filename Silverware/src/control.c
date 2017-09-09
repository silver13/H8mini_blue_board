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
#include "led.h"


extern float rx[];
extern float gyro[3];
extern int failsafe;
extern float pidoutput[PIDNUMBER];

extern float angleerror[];
extern float attitude[];

int onground = 1;
int onground_long = 1;
int pid_gestures_used = 0;

float thrsum;

float error[PIDNUMBER];
float motormap( float input);

float yawangle;

extern float looptime;

extern char auxchange[AUXNUMBER];
extern char aux[AUXNUMBER];

extern int ledcommand;
extern int ledblink;

extern float apid(int x);

#ifdef NOMOTORS
// to maintain timing or it will be optimized away
float tempx[4];
#endif

#ifdef STOCK_TX_AUTOCENTER
float autocenter[3];
float lastrx[3];
unsigned int consecutive[3];
#endif

unsigned long timecommand = 0;

extern int controls_override;
extern float rx_override[];
extern int acro_override;

float overthrottlefilt = 0;
float underthrottlefilt = 0;

float rxcopy[4];

extern int pwmdir;

void control( void)
{	

// rates / expert mode
float rate_multiplier = 1.0;
	
	if ( aux[RATES]  )
	{		
		
	}
	else
	{
		rate_multiplier = LOW_RATES_MULTI;
	}
	// make local copy
	

#ifdef INVERTED_ENABLE	
	if ( aux[FN_INVERTED]  )		
        pwmdir = REVERSE;
    else
        pwmdir = FORWARD;    
#endif	
	
	for ( int i = 0 ; i < 3 ; i++)
	{
		#ifdef STOCK_TX_AUTOCENTER
		rxcopy[i] = (rx[i] - autocenter[i])* rate_multiplier;
		#else
		rxcopy[i] = rx[i] * rate_multiplier;
		#endif
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
		#ifndef DISABLE_GESTURES2
		int command = gestures2();

		if (command!=GESTURE_NONE)
        {
            if (command == GESTURE_DDD)
		    {
			                  
                //skip accel calibration if pid gestures used
                if ( !pid_gestures_used )
                {
                    gyro_cal();	// for flashing lights
                    acc_cal();                   
                }
                else
                {
                    ledcommand = 1;
                    pid_gestures_used = 0;
                }
                //#ifdef FLASH_SAVE2
                //extern float accelcal[3];
               // flash2_fmc_write( accelcal[0] + 127 , accelcal[1] + 127);
                //#endif
                
                //#ifdef FLASH_SAVE1
			    extern void flash_save( void);
                extern void flash_load( void);
                flash_save( );
                flash_load( );
                //#endif
			    // reset loop time 
			    extern unsigned long lastlooptime;
			    lastlooptime = gettime();
		    }		

            if (command == GESTURE_RRD)
              {
                  aux[CH_AUX1] = 1;
                  ledcommand = 1;
              }
            if (command == GESTURE_LLD)
              {
                  ledcommand = 1;
                  aux[CH_AUX1] = 0;
              }
            #ifdef PID_GESTURE_TUNING              
            if ( command >= GESTURE_UDR ) pid_gestures_used = 1;   
              
           // int blink = 0;
            if (command == GESTURE_UDU)
              {
                        // Cycle to next pid term (P I D)
                        ledblink = next_pid_term();
              }
            if (command == GESTURE_UDD)
              {
                        // Cycle to next axis (Roll Pitch Yaw)
                        ledblink = next_pid_axis();
              }
            if (command == GESTURE_UDR)
              {
                  // Increase by 10%
                        ledblink = increase_pid();
              }
            if (command == GESTURE_UDL)
              {
                        // Descrease by 10%
                  ledblink = decrease_pid();
              }
                // U D U - Next PID term
                // U D D - Next PID Axis
                // U D R - Increase value
                // U D L - Descrease value
               // ledblink = blink; //Will cause led logic to blink the number of times ledblink has stored in it.
                #endif

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
		extern	void stick_vector( float);
		extern float errorvect[];	
		float yawerror[3];
			
		stick_vector( 0 );
			
			extern float GEstG[3];
		
			float yawrate = rxcopy[2] * (float) MAX_RATEYAW * DEGTORAD;
			
			yawerror[0] = GEstG[1] * yawrate;
			yawerror[1] = - GEstG[0] * yawrate;
			yawerror[2] = GEstG[2] * yawrate;

	  angleerror[0] = errorvect[0] * RADTODEG;// - attitude[0] + (float) TRIM_ROLL;
		angleerror[1] = errorvect[1] * RADTODEG;// - attitude[1] + (float) TRIM_PITCH;
			
		for ( int i = 0 ; i <2; i++)
			{
			error[i] = apid(i) + yawerror[i] - gyro[i];
			}
			
		error[2] = yawerror[2]  - gyro[2];
	  }
	else
	  {	// rate mode

		  error[0] = rxcopy[0] * (float) MAX_RATE * DEGTORAD  - gyro[0];
		  error[1] = rxcopy[1] * (float) MAX_RATE * DEGTORAD  - gyro[1];

			error[2] = rxcopy[2] * (float) MAX_RATEYAW * DEGTORAD  - gyro[2];
			
		  // reduce angle Iterm towards zero
		  extern float aierror[3];
		  for (int i = 0; i <= 1; i++)
			  aierror[i] *= 0.8f;
	  }


	pid(0);
	pid(1);
	pid(2);
#else
// rate only build
	error[ROLL] = rxcopy[ROLL] * (float) MAX_RATE * DEGTORAD  - gyro[ROLL];
	error[PITCH] = rxcopy[PITCH] * (float) MAX_RATE * DEGTORAD  - gyro[PITCH];
	error[YAW] = rxcopy[YAW] * (float) MAX_RATEYAW * DEGTORAD  - gyro[YAW];
	

	pid(ROLL);
	pid(PITCH);
	pid(YAW);
#endif

float	throttle;

// map throttle so under 10% it is zero	
if ( rx[3] < 0.1f ) throttle = 0;
else throttle = (rx[3] - 0.1f)*1.11111111f;

#ifdef AIRMODE_HOLD_SWITCH
	if (failsafe || aux[AIRMODE_HOLD_SWITCH] || throttle < 0.001f && !onground_long)
	{
		onground_long = 0;
#else

// turn motors off if throttle is off and pitch / roll sticks are centered
	if ( failsafe || (throttle < 0.001f && (!ENABLESTIX || !onground_long || aux[LEVELMODE] || (fabsf(rx[ROLL]) < (float) ENABLESTIX_TRESHOLD && fabsf(rx[PITCH]) < (float) ENABLESTIX_TRESHOLD && fabsf(rx[YAW]) < (float) ENABLESTIX_TRESHOLD ) ) ) ) 
	{	// motors off
#endif
		if ( onground_long )
		{
			if ( gettime() - onground_long > ENABLESTIX_TIMEOUT)
			{
				onground_long = 0;
			}
		}	
		
		for ( int i = 0 ; i <= 3 ; i++)
		{
			pwm_set( i , 0 );	
			#ifdef MOTOR_FILTER	
			// reset the motor filter
			motorfilter( 0 , i);
			#endif
		}	
		
		#ifdef MOTOR_BEEPS
		extern void motorbeep( void);
		motorbeep();
		#endif

		#ifdef MIX_LOWER_THROTTLE
		// reset the overthrottle filter
		lpf(&overthrottlefilt, 0.0f, 0.72f);	// 50hz 1khz sample rate
		lpf(&underthrottlefilt, 0.0f, 0.72f);	// 50hz 1khz sample rate
		#endif				
		
		#ifdef STOCK_TX_AUTOCENTER
		for( int i = 0 ; i <3;i++)
			{
				if ( rx[i] == lastrx[i] )
					{
						consecutive[i]++;
						
					}
				else consecutive[i] = 0;
				lastrx[i] = rx[i];
				if ( consecutive[i] > 1000 && fabsf( rx[i]) < 0.1f )
					{
						autocenter[i] = rx[i];
					}
			}
		#endif				
		
		onground = 1;
		thrsum = 0;
		
	}
	else
	{// motors on - normal flight
		
		onground = 0;
		onground_long = gettime();
		
		float mix[4];	
		
	if ( controls_override)
	{// change throttle in flip mode
		throttle = rx_override[3];
	}
		
	
		  // throttle angle compensation
#ifdef AUTO_THROTTLE
		  if (aux[LEVELMODE])
		    {
			    //float autothrottle = fastcos(attitude[0] * DEGTORAD) * fastcos(attitude[1] * DEGTORAD);
			    extern float GEstG[];
				float autothrottle = GEstG[2];
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

#ifdef INVERTED_ENABLE

if (pwmdir == REVERSE)
		{
			// inverted flight
		
		mix[MOTOR_FR] = throttle + pidoutput[ROLL] + pidoutput[PITCH] - pidoutput[YAW];		// FR
		mix[MOTOR_FL] = throttle - pidoutput[ROLL] + pidoutput[PITCH] + pidoutput[YAW];		// FL	
		mix[MOTOR_BR] = throttle + pidoutput[ROLL] - pidoutput[PITCH] + pidoutput[YAW];		// BR
		mix[MOTOR_BL] = throttle - pidoutput[ROLL] - pidoutput[PITCH] - pidoutput[YAW];		// BL	
		

		}	
else
#endif    
{
// normal
		
		mix[MOTOR_FR] = throttle - pidoutput[ROLL] - pidoutput[PITCH] + pidoutput[YAW];		// FR
		mix[MOTOR_FL] = throttle + pidoutput[ROLL] - pidoutput[PITCH] - pidoutput[YAW];		// FL	
		mix[MOTOR_BR] = throttle - pidoutput[ROLL] + pidoutput[PITCH] - pidoutput[YAW];		// BR
		mix[MOTOR_BL] = throttle + pidoutput[ROLL] + pidoutput[PITCH] + pidoutput[YAW];		// BL		
    
}
	
#ifdef INVERT_YAW_PID
// we invert again cause it's used by the pid internally (for limit)
pidoutput[2] = -pidoutput[2];			
#endif

		for ( int i = 0 ; i <= 3 ; i++)
		{			
		#ifdef MOTOR_FILTER		
		mix[i] = motorfilter(  mix[i] , i);
		#endif	
		
        #ifdef MOTOR_FILTER2_ALPHA	
        float motorlpf( float in , int x) ;           
		mix[i] = motorlpf(  mix[i] , i);
		#endif	
        }


#ifdef MIX_LOWER_THROTTLE

//#define MIX_INCREASE_THROTTLE

// options for mix throttle lowering if enabled
// 0 - 100 range ( 100 = full reduction / 0 = no reduction )
#ifndef MIX_THROTTLE_REDUCTION_PERCENT
#define MIX_THROTTLE_REDUCTION_PERCENT 100
#endif
// lpf (exponential) shape if on, othewise linear
//#define MIX_THROTTLE_FILTER_LPF

// limit reduction and increase to this amount ( 0.0 - 1.0)
// 0.0 = no action 
// 0.5 = reduce up to 1/2 throttle      
//1.0 = reduce all the way to zero 
#ifndef MIX_THROTTLE_REDUCTION_MAX
#define MIX_THROTTLE_REDUCTION_MAX 0.5
#endif

#ifndef MIX_MOTOR_MAX
#define MIX_MOTOR_MAX 1.0f
#endif


		  float overthrottle = 0;
			float underthrottle = 0.001f;
		
		  for (int i = 0; i < 4; i++)
		    {
			    if (mix[i] > overthrottle)
				    overthrottle = mix[i];
					if (mix[i] < underthrottle)
						underthrottle = mix[i];
		    }

		  overthrottle -= MIX_MOTOR_MAX ;

		  if (overthrottle > (float)MIX_THROTTLE_REDUCTION_MAX)
			  overthrottle = (float)MIX_THROTTLE_REDUCTION_MAX;

#ifdef MIX_THROTTLE_FILTER_LPF
		  if (overthrottle > overthrottlefilt)
			  lpf(&overthrottlefilt, overthrottle, 0.82);	// 20hz 1khz sample rate
		  else
			  lpf(&overthrottlefilt, overthrottle, 0.72);	// 50hz 1khz sample rate
#else
		  if (overthrottle > overthrottlefilt)
			  overthrottlefilt += 0.005f;
		  else
			  overthrottlefilt -= 0.01f;
#endif
			
#ifdef MIX_INCREASE_THROTTLE
// under			
			
		  if (underthrottle < -(float)MIX_THROTTLE_REDUCTION_MAX)
			  underthrottle = -(float)MIX_THROTTLE_REDUCTION_MAX;
			
#ifdef MIX_THROTTLE_FILTER_LPF
		  if (underthrottle < underthrottlefilt)
			  lpf(&underthrottlefilt, underthrottle, 0.82);	// 20hz 1khz sample rate
		  else
			  lpf(&underthrottlefilt, underthrottle, 0.72);	// 50hz 1khz sample rate
#else
		  if (underthrottle < underthrottlefilt)
			  underthrottlefilt -= 0.005f;
		  else
			  underthrottlefilt += 0.01f;
#endif
// under
			if (underthrottlefilt < - (float)MIX_THROTTLE_REDUCTION_MAX)
			  underthrottlefilt = - (float)MIX_THROTTLE_REDUCTION_MAX;
		  if (underthrottlefilt > 0.1f)
			  underthrottlefilt = 0.1;

			underthrottle = underthrottlefilt;
					
			if (underthrottle > 0.0f)
			  underthrottle = 0.0001f;

			underthrottle *= ((float)MIX_THROTTLE_REDUCTION_PERCENT / 100.0f);
#else
  underthrottle = 0.001f;			
#endif			
// over			
		  if (overthrottlefilt > (float)MIX_THROTTLE_REDUCTION_MAX)
			  overthrottlefilt = (float)MIX_THROTTLE_REDUCTION_MAX;
		  if (overthrottlefilt < -0.1f)
			  overthrottlefilt = -0.1;


		  overthrottle = overthrottlefilt;

			
		  if (overthrottle < 0.0f)
			  overthrottle = -0.0001f;

			
			// reduce by a percentage only, so we get an inbetween performance
			overthrottle *= ((float)MIX_THROTTLE_REDUCTION_PERCENT / 100.0f);

			
			
		  if (overthrottle > 0 || underthrottle < 0 )
		    {		// exceeding max motor thrust
					float temp = overthrottle + underthrottle;
			    for (int i = 0; i < 4; i++)
			      {
				      mix[i] -= temp;
			      }
		    }
#endif				


#ifdef MIX_LOWER_THROTTLE_3
{
#ifndef MIX_THROTTLE_REDUCTION_MAX
#define MIX_THROTTLE_REDUCTION_MAX 0.5f
#endif

float overthrottle = 0;

for (int i = 0; i < 4; i++)
		    {
			    if (mix[i] > overthrottle)
				    overthrottle = mix[i];
            }


overthrottle -=1.0f;
// limit to half throttle max reduction
if ( overthrottle > (float) MIX_THROTTLE_REDUCTION_MAX)  overthrottle = (float) MIX_THROTTLE_REDUCTION_MAX;

if ( overthrottle > 0.0f)
{
    for ( int i = 0 ; i < 4 ; i++)
        mix[i] -= overthrottle;
}
#ifdef MIX_THROTTLE_FLASHLED
if ( overthrottle > 0.1f) ledcommand = 1;
#endif
}
#endif


#ifdef MIX_INCREASE_THROTTLE_3
{
#ifndef MIX_THROTTLE_INCREASE_MAX
#define MIX_THROTTLE_INCREASE_MAX 0.2f
#endif

float underthrottle = 0;

for (int i = 0; i < 4; i++)
    {
        if (mix[i] < underthrottle)
            underthrottle = mix[i];
    }


// limit to half throttle max reduction
if ( underthrottle < -(float) MIX_THROTTLE_INCREASE_MAX)  underthrottle = -(float) MIX_THROTTLE_INCREASE_MAX;

if ( underthrottle < 0.0f)
    {
        for ( int i = 0 ; i < 4 ; i++)
            mix[i] -= underthrottle;
    }
#ifdef MIX_THROTTLE_FLASHLED
if ( underthrottle < -0.01f) ledcommand = 1;
#endif
}
#endif

            
            
            
thrsum = 0;		
				
		for ( int i = 0 ; i <= 3 ; i++)
		{			
		           
		#ifdef CLIP_FF
		mix[i] = clip_ff(mix[i], i);
		#endif

		#ifdef MOTORS_TO_THROTTLE
		// flash leds in valid throttle range
		ledcommand = 1;
        float test = throttle;
        // Spin all motors if the roll/pitch stick is centered.
        // Otherwise select the motors to test by deflecting the roll/pitch stick.
        if ( i == 1 && ( rx[ROLL] > 0.5f || rx[PITCH] < -0.5f ) ) { test = 0; }
        if ( i == 0 && ( rx[ROLL] > 0.5f || rx[PITCH] > 0.5f ) ) { test = 0; }
        if ( i == 3 && ( rx[ROLL] < -0.5f || rx[PITCH] < -0.5f ) ) { test = 0; }
		if ( i == 2 && ( rx[ROLL] < -0.5f || rx[PITCH] > 0.5f ) ) { test = 0; }
        mix[i] = test;   
		#warning "MOTORS TEST MODE"
		#endif

		#ifdef MOTOR_MIN_ENABLE
		if (mix[i] < (float) MOTOR_MIN_VALUE)
		{
			mix[i] = (float) MOTOR_MIN_VALUE;
		}
		#endif
		
		#ifdef MOTOR_MAX_ENABLE
		if (mix[i] > (float) MOTOR_MAX_VALUE)
		{
			mix[i] = (float) MOTOR_MAX_VALUE;
		}
		#endif
			
		#ifndef NOMOTORS
		#ifndef MOTORS_TO_THROTTLE
		//normal mode
		pwm_set( i ,motormap( mix[i] ) );
		#else
		// throttle test mode
		ledcommand = 1;
		pwm_set( i , test );
		#endif
		#else
		// no motors mode ( anti-optimization)
		#warning "NO MOTORS"
		tempx[i] = motormap( mix[i] );
		#endif
		
		if ( mix[i] < 0 ) mix[i] = 0;
		if ( mix[i] > 1 ) mix[i] = 1;
		thrsum+= mix[i];
		}	
		thrsum = thrsum / 4;
		
	}// end motors on
	
}


#ifndef MOTOR_FILTER2_ALPHA
#define MOTOR_FILTER2_ALPHA 0.3
#endif


float motor_filt[4];

float motorlpf( float in , int x)
{
    
    lpf(&motor_filt[x] , in , 1 - MOTOR_FILTER2_ALPHA);
       
    return motor_filt[x];
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


