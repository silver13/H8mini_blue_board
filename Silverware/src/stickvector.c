#include "config.h"
#include "util.h"

#include <math.h>
#include <string.h>


void vector_cross(float vout[3], float v1[3],float v2[3])
{
  vout[0]= (v1[1]*v2[2]) - (v1[2]*v2[1]);
  vout[1]= (v1[2]*v2[0]) - (v1[0]*v2[2]);
  vout[2]= (v1[0]*v2[1]) - (v1[1]*v2[0]);
}

extern float rx[];
extern float GEstG[3];
extern float Q_rsqrt( float number );


float errorvect[3];

void stick_vector( float maxangle)
{
	
float stickvector[3];	
	
	// start with a down vector
	stickvector[0] = 0;
	stickvector[1] = 0;
	stickvector[2] = 1;

float pitch, roll;

	// rotate down vector to match stick position
pitch = rx[1] * MAX_ANGLE_HI * DEGTORAD + (float) TRIM_PITCH;
roll = rx[0] * MAX_ANGLE_HI * DEGTORAD + (float) TRIM_ROLL;

stickvector[0] = fastsin( roll );
stickvector[1] = fastsin( pitch );
stickvector[2] = fastcos( roll ) * fastcos( pitch );

float mag2 = Q_rsqrt( (stickvector[0] * stickvector[0] + stickvector[1] * stickvector[1]) / (1 - stickvector[2] * stickvector[2]));
stickvector[0] *=mag2;
stickvector[1] *=mag2;	


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

}



