
#include "config.h"

float lpffilter( float in,int num )
{
	#ifdef SOFT_LPF_NONE
	return in;
	#else
	return filter[num].step(in );
	#endif
	
}


// 16Hz hpf filter for throttle compensation
//High pass bessel filter order=1 alpha1=0.016 


float throttlehpf1_step(float x) //class II 
{
        static float v[2];
	v[0] = v[1];
	v[1] = (9.521017968695103528e-1f * x)
		 + (0.90420359373902081668f * v[0]);
	return 
		 (v[1] - v[0]);
}


float throttlehpf( float in )
{
	return throttlehpf1_step(in );
}



