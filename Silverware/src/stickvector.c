#include "config.h"
#include "util.h"

#include <math.h>
#include <string.h>



extern float rxcopy[];
extern float GEstG[3];
extern float Q_rsqrt( float number );
extern char aux[];

float errorvect[3];

void stick_vector( float maxangle)
{
	
float stickvector[3];	
	

float pitch, roll;

	// rotate down vector to match stick position
pitch = rxcopy[1] * MAX_ANGLE_HI * DEGTORAD + (float) TRIM_PITCH  * DEGTORAD;
roll = rxcopy[0] * MAX_ANGLE_HI * DEGTORAD + (float) TRIM_ROLL  * DEGTORAD;

stickvector[0] = fastsin( roll );
stickvector[1] = fastsin( pitch );
stickvector[2] = fastcos( roll ) * fastcos( pitch );

		
float	mag2 = (stickvector[0] * stickvector[0] + stickvector[1] * stickvector[1]);

if ( mag2 > 0.001f ) 
{
mag2 = Q_rsqrt( mag2 / (1 - stickvector[2] * stickvector[2]) );
}
else mag2 = 0.707f;

stickvector[0] *=mag2;
stickvector[1] *=mag2;	

#ifdef INVERTED_ENABLE
extern int pwmdir;

if ( pwmdir==REVERSE )
{
	stickvector[0] = - stickvector[0];
	stickvector[1] = - stickvector[1];
	stickvector[2] = - stickvector[2];
}
#endif

// find error between stick vector and quad orientation
// vector cross product 
  errorvect[0]= (GEstG[1]*stickvector[2]) - (GEstG[2]*stickvector[1]);
  errorvect[1]= (GEstG[2]*stickvector[0]) - (GEstG[0]*stickvector[2]);

// some limits just in case
limitf( & errorvect[0] , 1.0);
limitf( & errorvect[1] , 1.0);

float temp = errorvect[0];
errorvect[0] = (errorvect[1]);
errorvect[1] = (- temp);

// fix to recover if triggered inverted
// the vector cross product results in zero for opposite vectors, so it's bad at 180 error
// without this the quad will not invert if angle difference = 180 

#ifdef INVERTED_ENABLE

static int flip_active_once = 0;
static int flipaxis = 0;
static int flipdir = 0;
int flip_active = 0;

#define rollrate 2.0f
#define g_treshold 0.125f
#define roll_bias 0.25f

if ( aux[FN_INVERTED]  && (GEstG[2] > g_treshold) )
{
	flip_active = 1;
	// rotate around axis with larger leaning angle

		if ( flipdir ) 
		{
			errorvect[flipaxis] = rollrate;
		}
		else 
		{
			errorvect[flipaxis] = -rollrate;			
		}
		
}
else if ( !aux[FN_INVERTED]  && (GEstG[2] < -g_treshold) )
{
	flip_active = 1;

		if ( flipdir ) 
		{
			errorvect[flipaxis] = -rollrate;
		}
		else 
		{
			errorvect[flipaxis] = rollrate;			
		}

}
else
	flip_active_once = 0;

// set common things here to avoid duplication
if ( flip_active )
{
	if ( !flip_active_once )
	{
		// check which axis is further from center, with a bias towards roll
		// because a roll flip does not leave the quad facing the wrong way
		if( fabsf(GEstG[0])+ roll_bias > fabsf(GEstG[1]) )
		{
			// flip in roll axis
			flipaxis = 0;
		}
		else
			flipaxis = 1;
	
	if (  GEstG[flipaxis] > 0 )
		flipdir = 1;
	else
		flipdir = 0;
	
	flip_active_once = 1;
	}
	
	// set the error in other axis to return to zero
	errorvect[!flipaxis] = GEstG[!flipaxis]; 
	
}
#endif

}



