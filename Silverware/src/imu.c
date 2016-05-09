
// library headers
#include <stdbool.h>
#include <inttypes.h>

//#define _USE_MATH_DEFINES
#include <math.h>
#include "drv_time.h"

#include "util.h"
#include "sixaxis.h"
#include "config.h"

#include <stdlib.h>


#ifdef DEBUG
#include "debug.h"
extern debug_type debug;
#endif


#define ACC_1G 2048.0f


// accel magnitude limits for drift correction
#define ACC_MIN 0.7f
#define ACC_MAX 1.3f


float GEstG[3] = { 0, 0, ACC_1G };

float attitude[3];

float estimated_bias[3];
float filteredp2[3];

extern float gyro[3];
extern float accel[3];
extern float accelcal[3];


void imu_init(void)
{
	// init the gravity vector with accel values
	for (int xx = 0; xx < 100; xx++)
	  {
		  sixaxis_read();

		  for (int x = 0; x < 3; x++)
		    {
			    lpf(&GEstG[x], accel[x], 0.85);
		    }
		  delay(1000);


	  }
}

// from http://en.wikipedia.org/wiki/Fast_inverse_square_root
// originally from quake3 code

float Q_rsqrt( float number )
{

	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       
	i  = 0x5f3759df - ( i >> 1 );               
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 3nd iteration, this can be removed
	
	return y;
}




static unsigned long gptimer;

float atan2approx(float y, float x);

float calcmagnitude(float vector[3])
{
	float accmag = 0;
	for (uint8_t axis = 0; axis < 3; axis++)
	  {
		  accmag += vector[axis] * vector[axis];
	  }
	accmag = 1.0f/Q_rsqrt(accmag);
	return accmag;
}



float offset[3];

void imu_calc(void)
{
	float deltatime;	// time in seconds

	unsigned long time = gettime();
	deltatime = time - gptimer;
	gptimer = time;
	if (deltatime < 1)
		deltatime = 1;
	if (deltatime > 20000)
		deltatime = 20000;
	deltatime = deltatime * 1e-6f;	// uS to seconds


// remove bias
  accel[0] = accel[0] - accelcal[0];
	accel[1] = accel[1] - accelcal[1];
	

	float deltaGyroAngle[3];

	for ( int i = 0 ; i < 3 ; i++)
	{
	deltaGyroAngle[i] = (gyro[i]) * deltatime + offset[i];
	}
	

	
	GEstG[2] = GEstG[2] - (deltaGyroAngle[0]) * GEstG[0];
	GEstG[0] = (deltaGyroAngle[0]) * GEstG[2] +  GEstG[0];


	GEstG[1] =  GEstG[1] + (deltaGyroAngle[1]) * GEstG[2];
	GEstG[2] = -(deltaGyroAngle[1]) * GEstG[1] +  GEstG[2];


	GEstG[0] = GEstG[0] - (deltaGyroAngle[2]) * GEstG[1];
	GEstG[1] = (deltaGyroAngle[2]) * GEstG[0] +  GEstG[1];


#ifdef DEBUG
	attitude[2] += RADTODEG * gyro[2] * deltatime;

	limit180(&attitude[2]);
#endif


//static float accel_filt[3]; 

#define FILTERCALC( sampleperiod, filtertime) (1.0f - ((float)sampleperiod) / ((float)filtertime))

	//for (int axis = 0; axis < 3; axis++)
	//{
	//	lpf( &accel_filt[axis] , accel[axis] , FILTERCALC( 0.001 , 0.25));
	//}	


// calc acc mag
	float accmag;

	accmag = calcmagnitude(&accel[0]);
	#ifdef DEBUG
	//debug.accmag= accmag;	
	//debug.gmag = calcmagnitude(&GEstG[0]);
	#endif


	float pterm[3];


#define Kp_ROLLPITCH 1e-6


	attitude[0] = atan2approx(GEstG[0], GEstG[2]) ;
	attitude[1] = atan2approx(GEstG[1], GEstG[2])  ;
		
	if ( accmag  > 0.8f * ACC_1G && accmag  < 1.2f * ACC_1G )
  {
	
	float error[3];
	static float accattitude[3];
		
	
	for (int axis = 0; axis < 3; axis++)
	{
		accel[axis] = accel[axis] /  accmag ;
	}	
	
		
	accattitude[0] = atan2approx(accel[0], accel[2]) ;
	accattitude[1] = atan2approx(accel[1], accel[2]) ;		

	for ( int axis = 0 ; axis < 2 ; axis++)
	{
//	static float attitudefilt[3];	
//	lpf( &attitudefilt[axis] , attitude[axis] , FILTERCALC( 0.001 , 0.25));
		
		error[axis] = accattitude[axis] - attitude[axis];
//	error[axis] = accattitude[axis] - attitudefilt[axis];

	pterm[axis] = error[axis] * (float) Kp_ROLLPITCH;

	 lpf ( &filteredp2[axis],pterm[axis],0.999);

		if ( filteredp2[axis] > 1e-5f )
		{
			estimated_bias[axis] += 0.3e-8f; // 1 lsb over 1 sec 1e-6
			
		}
		if ( filteredp2[axis] < -1e-5f) // -1e-6
		{
			estimated_bias[axis] -= 0.3e-8f;
		}
	
	}
  }
  else
  {	
	 pterm[0] = 0;
   pterm[1] = 0;
   pterm[2] = 0;	 
  }
	
offset[0] = pterm[0] + estimated_bias[0] + 0e-5f;
offset[1] = pterm[1] + estimated_bias[1];
//offset[2] = pterm[2] + estimated_bias[2];

}



#define M_PI  3.14159265358979323846	/* pi */


#define OCTANTIFY(_x, _y, _o)   do {                            \
    float _t;                                                   \
    _o= 0;                                                \
    if(_y<  0)  {            _x= -_x;   _y= -_y; _o += 4; }     \
    if(_x<= 0)  { _t= _x;    _x=  _y;   _y= -_t; _o += 2; }     \
    if(_x<=_y)  { _t= _y-_x; _x= _x+_y; _y=  _t; _o += 1; }     \
} while(0);

// +-0.09 deg error
float atan2approx(float y, float x)
{

	if (x == 0)
		x = 123e-15f;
	float phi = 0;
	float dphi;
	float t;

	OCTANTIFY(x, y, phi);

	t = (y / x);
	// atan function for 0 - 1 interval
	//dphi = M_PI / 4 * t - t * ((t) - 1) * (0.2447f + 0.0663f * (t));

//	dphi = t*(M_PI / 4  -  ((t) - 1) * (0.2447f + 0.0663f * (t)));
	dphi = t*( ( M_PI/4 + 0.2447f ) + t *( ( -0.2447f + 0.0663f ) + t*( - 0.0663f)) );
	phi *= M_PI / 4;
	dphi = phi + dphi;
	if (dphi > (float) M_PI)
		dphi -= 2 * M_PI;
	return RADTODEG * dphi;
}

