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


//#define RECTANGULAR_RULE_INTEGRAL
#define MIDPOINT_RULE_INTEGRAL
//#define SIMPSON_RULE_INTEGRAL


//#define NORMAL_DTERM
//#define SECOND_ORDER_DTERM
#define NEW_DTERM

#include <stdbool.h>
#include <stdlib.h>
#include "pid.h"
#include "util.h"
#include "config.h"
#include "led.h"
#include "defines.h"



// Kp											ROLL       PITCH     YAW
float pidkp[PIDNUMBER] = { 17.0e-2 , 17.0e-2  , 10e-1 }; 

// Ki											ROLL       PITCH     YAW
float pidki[PIDNUMBER] = { 15e-1  , 15e-1 , 5e-1 };	

// Kd											ROLL       PITCH     YAW
float pidkd[PIDNUMBER] = { 6.8e-1 , 6.8e-1  , 0.0e-1 };	
// PID_GESTURES modifications
int number_of_increments[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
int current_pid_axis = 0;
int current_pid_term = 1;
float * current_pid_term_pointer = pidkp;
// PID_GESTURES modifications - End

// "setpoint weighting" 0.0 - 1.0 where 0.0 = normal pid
float b[3] = { 0.0 , 0.0 , 0.0};





// output limit			
const float outlimit[PIDNUMBER] = { 0.8 , 0.8 , 0.4 };

// limit of integral term (abs)
const float integrallimit[PIDNUMBER] = { 0.8 , 0.8 , 0.4 };




// non changable things below
float ierror[PIDNUMBER] = { 0 , 0 , 0};	

float pidoutput[PIDNUMBER];

extern float error[PIDNUMBER];
static float lasterror[PIDNUMBER];

extern float looptime;
extern float gyro[3];
extern int onground;

extern float looptime;


#ifdef NORMAL_DTERM
static float lastrate[PIDNUMBER];
#endif

#ifdef NEW_DTERM
static float lastratexx[PIDNUMBER][2];
#endif

#ifdef SECOND_ORDER_DTERM
static float lastratexx[PIDNUMBER][4];
#endif

#ifdef SIMPSON_RULE_INTEGRAL
static float lasterror2[PIDNUMBER];
#endif

float timefactor;

extern int ledcommand;

void pid_precalc()
{
	timefactor = 0.0032f / looptime;
}

// PID_GESTURES modifications
int get_current_pid_term() { return current_pid_term; } // These may be replaced by extern variables.
int get_current_pid_axis() { return current_pid_axis; } // These may be replaced by extern variables.

// Cycle through P / I / D - The initial value is P
// The return value is the currently selected TERM (after setting the next one)
// 1: P
// 2: I
// 3: D
// The return value is used to blink the leds in main.c
int next_pid_term()
{
	current_pid_axis = 0;
	
	switch (current_pid_term)
	{
		case 1:
			current_pid_term_pointer = pidki;
			current_pid_term = 2;
			break;
		case 2:
			current_pid_term_pointer = pidkd;
			current_pid_term = 3;
			break;
		case 3:
			current_pid_term_pointer = pidkp;
			current_pid_term = 1;
			break;
	}
	
	return current_pid_term;
}

// Cycle through the axis - Initial is Roll
// Return value is the selected axis, after setting the next one.
// 1: Roll
// 2: Pitch
// 3: Yaw
// The return value is used to blink the leds in main.c
int next_pid_axis()
{
	int size = 3;
	if (current_pid_axis == size - 1) {
		current_pid_axis = 0;
	}
	else {
		current_pid_axis++;
	}
	
	return current_pid_axis + 1;
}

int change_pid_value(int increase)
{
	float multiplier = 0.9f;
	if (increase) {
		multiplier = 1.1f;
		number_of_increments[current_pid_term][current_pid_axis]++;
	}
	else {
		number_of_increments[current_pid_term][current_pid_axis]--;
	}
	current_pid_term_pointer[current_pid_axis] = current_pid_term_pointer[current_pid_axis] * multiplier;
	
	return abs(number_of_increments[current_pid_term][current_pid_axis]);
}

// Increase currently selected term, for the currently selected axis, (by functions above) by 10%
// The return value, is absolute number of times the specific term/axis was increased or decreased.  For example, if P for Roll was increased by 10% twice,
// And then reduced by 10% 3 times, the return value would be 1  -  The user has to rememeber he has eventually reduced the by 10% and not increased by 10%
// I guess this can be improved by using the red leds for increments and blue leds for decrements or something, or just rely on SilverVISE
int increase_pid()
{
	return change_pid_value(1);
}

// Same as increase_pid but... you guessed it... decrease!
int decrease_pid()
{
	return change_pid_value(0);
}
// PID_GESTURES modifications - End

float pid(int x )
{ 

        if (onground) 
				{
           ierror[x] *= 0.8f;
				}
	
				int iwindup = 0;
				if (( pidoutput[x] == outlimit[x] )&& ( error[x] > 0) )
				{
					iwindup = 1;		
				}
				if (( pidoutput[x] == -outlimit[x])&& ( error[x] < 0) )
				{
					iwindup = 1;				
				}
        if ( !iwindup)
				{
				#ifdef MIDPOINT_RULE_INTEGRAL
				 // trapezoidal rule instead of rectangular
         ierror[x] = ierror[x] + (error[x] + lasterror[x]) * 0.5f *  pidki[x] * looptime;
				 lasterror[x] = error[x];
				#endif
					
				#ifdef RECTANGULAR_RULE_INTEGRAL
				 ierror[x] = ierror[x] + error[x] *  pidki[x] * looptime;
				 lasterror[x] = error[x];					
				#endif
					
				#ifdef SIMPSON_RULE_INTEGRAL
					// assuming similar time intervals
				 ierror[x] = ierror[x] + 0.166666f* (lasterror2[x] + 4*lasterror[x] + error[x]) *  pidki[x] * looptime;	
					lasterror2[x] = lasterror[x];
					lasterror[x] = error[x];
					#endif
					
				}
				
				limitf( &ierror[x] , integrallimit[x] );
				
				// P term
          pidoutput[x] = error[x] * ( 1 - b[x])* pidkp[x] ;
				
				// b
          pidoutput[x] +=  - ( b[x])* pidkp[x] * gyro[x]  ;
				
				// I term	
					pidoutput[x] += ierror[x];
			
				// D term		  

				#ifdef NORMAL_DTERM
					pidoutput[x] = pidoutput[x] - (gyro[x] - lastrate[x]) * pidkd[x] * timefactor  ;
					lastrate[x] = gyro[x];
				#endif

				#ifdef SECOND_ORDER_DTERM 
					pidoutput[x] = pidoutput[x] - ( -( 0.083333333f) *gyro[x] + (0.666666f) * lastratexx[x][0]
								- (0.666666f) * lastratexx[x][2] + ( 0.083333333f) * lastratexx[x][3]) * pidkd[x] * timefactor 						;
				
					lastratexx[x][3] = lastratexx[x][2];
					lastratexx[x][2] = lastratexx[x][1];
					lastratexx[x][1] = lastratexx[x][0];
					lastratexx[x][0] = gyro[x];
				#endif
			  #ifdef NEW_DTERM
					pidoutput[x] = pidoutput[x] - ( ( 0.5f) *gyro[x] 
								- (0.5f) * lastratexx[x][1] ) * pidkd[x] * timefactor  ;
									
					lastratexx[x][1] = lastratexx[x][0];
					lastratexx[x][0] = gyro[x];
			  #endif
				

				  limitf(  &pidoutput[x] , outlimit[x]);



return pidoutput[x];		 		
}


