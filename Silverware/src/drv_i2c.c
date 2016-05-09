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



#include "project.h"
#include <stdint.h>
#include <stdio.h>

#include "drv_i2c.h"

#include "drv_time.h"

#define I2CADDRESS 0x68  


#define I2C_x I2C1

void i2c_init( void)
{
	// hardware i2c not functional yet
	/*
GPIO_InitTypeDef  GPIO_InitStructure;

	
GPIO_PinAFConfig( GPIOB , EXTI_PinSource8 , GPIO_AF_1 );	
	
	
GPIO_PinAFConfig( GPIOB , EXTI_PinSource9 , GPIO_AF_1 );
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; // i2c1 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // i2c1 
 GPIO_Init(GPIOB, &GPIO_InitStructure);
	*/
	
I2C_InitTypeDef  I2C_InitStructure;
	
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE);
	
	RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
 
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
  I2C_InitStructure.I2C_DigitalFilter = 0x00;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_Timing = 0x00210507;
 
  I2C_Init( I2C_x , &I2C_InitStructure);
   
  I2C_Cmd( I2C_x , ENABLE);
  
	
}

unsigned int liberror;

#define I2CTIMEOUTCONDITION (i2ccount < 8092)
//static unsigned int i2ccount = 0; 


void i2c_sendheader()
{

}

void i2c_writereg( int reg ,int data)
{

	
	
}



int i2c_readreg( int reg )
{
  int result ;

	return result;
}


int i2c_readdata( int reg, int *data, int size )
{

	return 1;
}























